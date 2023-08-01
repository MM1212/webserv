#pragma once

#include <shared.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

#define SYS_SEND ::send
#define SYS_RECV ::recv
#define SYS_LISTEN ::listen
#define SYS_ACCEPT ::accept
#define SYS_CLOSE ::close
#define SYS_FNCTL ::fcntl
#define SYS_POLL ::poll
#define SYS_BIND ::bind

#include <string>
#include <set>

namespace Socket {

  template <typename T>
  class FileManager {
  public:
    class File {
      int fd;
      int events;
    public:
      File() : fd(-1), events(0) {}
      File(int fd) : fd(fd), events(0) {}
      ~File() {}
      File(const File& other) : fd(other.fd), events(other.events) {}
      File& operator=(const File& other) {
        if (this == &other) return *this;
        this->fd = other.fd;
        this->events = other.events;
      }

      inline int getFd() const { return this->fd; }
      inline bool isReadable() const { return this->events & EPOLLIN; }
      inline bool isWritable() const { return this->events & EPOLLOUT; }
      inline bool isClosed() const { return this->events & (EPOLLRDHUP | EPOLLHUP); }
      inline bool isErrored() const { return this->events & EPOLLERR; }

      operator int() const { return this->fd; }
    private:
      void setEvents(int events) { this->events = events; }

      friend class FileManager;
    };
  private:
    typedef struct epoll_event epoll_event_t;

    std::set<File> fds;
    int epollFd;
    epoll_event_t* events;
    int maxEvents;
    bool running;

    T* instance;
    void (T::* onTick)(const FileManager<T>&);
  public:
    FileManager() :
      fds(), epollFd(-1),
      events(new epoll_event_t[0]), maxEvents(0), running(false),
      instance(NULL), onTick(NULL) {
      this->init();
    }
    FileManager(T* instance, void (T::* onTick)(const FileManager<T>&)) :
      fds(), epollFd(-1),
      events(new epoll_event_t[0]), maxEvents(0), running(false),
      instance(instance), onTick(onTick) {
      this->init();
    }
    FileManager(const FileManager& other) :
      fds(other.fds), epollFd(other.epollFd),
      events(new epoll_event_t[other.maxEvents]), maxEvents(other.maxEvents), running(false),
      instance(other.instance), onTick(other.onTick) {}
    FileManager& operator=(const FileManager& other) {
      if (this == &other) return *this;
      this->fds = other.fds;
      this->epollFd = other.epollFd;
      if (this->events) delete[] this->events;
      this->events = new epoll_event_t[other.maxEvents];
      this->maxEvents = other.maxEvents;
      this->instance = other.instance;
      this->onTick = other.onTick;
      this->running = false;
      return *this;
    }

    ~FileManager() {
      if (this->events) delete[] this->events;
      if (this->epollFd != -1) SYS_CLOSE(this->epollFd);
    }
  private:
    void init() {
      if (this->epollFd != -1) return;
      this->epollFd = ::epoll_create1(0);
      if (this->epollFd == -1) {
        throw std::runtime_error("Failed to create epoll instance");
      }
      SYS_FNCTL(this->epollFd, F_SETFD, FD_CLOEXEC);
    }
  public:
    bool add(int fd, int flags) {
      if (this->has(fd))
        this->remove(fd);
      epoll_event_t event;
      event.events = flags;
      event.data.fd = fd;
      if (::epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
        return false;
      }
      SYS_FNCTL(fd, F_SETFD, FD_CLOEXEC | O_NONBLOCK);
      this->fds.insert(File(fd));
      this->maxEvents++;
      if (this->events) delete[] this->events;
      this->events = new epoll_event_t[this->maxEvents];
      return true;
    }

    inline bool has(int fd) const {
      return this->fds.count(fd) > 0;
    }

    inline bool remove(int fd, bool close) {
      if (!this->has(fd)) return false;
      if (::epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        return false;
      }
      this->fds.erase(fd);
      if (close)
        SYS_CLOSE(fd);
      this->maxEvents--;
      if (this->events) delete[] this->events;
      this->events = new epoll_event_t[this->maxEvents];
      return true;
    }

    inline bool update(int fd, int flags) {
      if (!this->has(fd)) return false;
      epoll_event_t event;
      event.events = flags;
      event.data.fd = fd;
      if (::epoll_ctl(this->epollFd, EPOLL_CTL_MOD, fd, &event) == -1) {
        return false;
      }
      return true;
    }

    inline bool update(File& file, int flags) {
      return this->update(file.getFd(), flags);
    }

    void start() {
      if (this->running) return;
      this->running = true;
      while (this->running) {
        int n = ::epoll_wait(this->epollFd, this->events, this->maxEvents, -1);
        if (n == -1) {
          if (errno == EINTR) continue;
          throw std::runtime_error("epoll_wait failed");
        }
        for (int i = 0; i < n; i++) {
          epoll_event_t& event = this->events[i];
          std::set<File>::iterator it = this->fds.find(event.data.fd);
          if (it == this->fds.end()) continue;
          File& file = *it;
          file.setEvents(event.events);
        }
        if (this->instance && this->onTick) {
          (this->instance->*this->onTick)(*this);
        }
      }
    }

  };

}