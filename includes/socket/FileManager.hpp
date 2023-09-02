/**
 * The FileManager class provides functionality to add, remove and get file descriptors using epoll.
 * It also provides a set of functions to check if a file descriptor is readable, writable, closed or errored.
 * The class also has a template parameter T which is used to specify the class that will handle the onTick event.
 *
 * The FileManager class is used by the Socket class to manage file descriptors for sockets & processes pipes.
 *
 */
#pragma once

#include <shared.hpp>
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
#include <iostream>
#include <csignal>

#include <utils/Logger.hpp>

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
      return *this;
    }

    inline int getFd() const { return this->fd; }
    inline bool isReadable() const { return this->events & EPOLLIN; }
    inline bool isWritable() const { return this->events & EPOLLOUT; }
    inline bool isClosed() const { return this->events & (EPOLLRDHUP | EPOLLHUP); }
    inline bool isErrored() const { return this->events & EPOLLERR; }

    operator int() const { return this->fd; }
    void setEvents(int events) { this->events = events; }
  };

  template <typename T>
  class FileManager {
    typedef struct epoll_event epoll_event_t;

    std::set<File> fds;
    int epollFd;
    epoll_event_t* events;
    int maxEvents;
    bool running;
    int timeout;

    T* instance;
    void (T::* onTick)(const std::vector<File>&);
  public:
    FileManager() :
      fds(), epollFd(-1),
      events(new epoll_event_t[0]), maxEvents(0), running(false), timeout(-1),
      instance(NULL), onTick(NULL) {
      this->init();
    }
    FileManager(T* instance, void (T::* onTick)(const std::vector<File>&)) :
      fds(), epollFd(-1),
      events(new epoll_event_t[0]), maxEvents(0), running(false), timeout(-1),
      instance(instance), onTick(onTick) {
      this->init();
    }
    FileManager(const FileManager& other) :
      fds(other.fds), epollFd(other.epollFd),
      events(new epoll_event_t[other.maxEvents]), maxEvents(other.maxEvents), running(false), timeout(other.timeout),
      instance(other.instance), onTick(other.onTick) {
      for (int i = 0; i < this->maxEvents; i++)
        this->events[i] = other.events[i];
    }
    FileManager& operator=(const FileManager& other) {
      if (this == &other) return *this;
      this->fds = other.fds;
      this->epollFd = other.epollFd;
      if (this->events) delete[] this->events;
      this->events = new epoll_event_t[other.maxEvents];
      for (int i = 0; i < this->maxEvents; i++)
        this->events[i] = other.events[i];
      this->maxEvents = other.maxEvents;
      this->timeout = other.timeout;
      this->instance = other.instance;
      this->onTick = other.onTick;
      this->running = false;
      return *this;
    }

    ~FileManager() {
      this->running = false;
      if (this->events) delete[] this->events;
      for (std::set<File>::iterator it = this->fds.begin(); it != this->fds.end(); it++) {
        SYS_CLOSE(*it);
      }
      if (this->epollFd != -1) SYS_CLOSE(this->epollFd);
    }
  private:
    /*
     * Initializes the epoll pool.
     */
    void init() {
      if (this->epollFd != -1) return;
      this->epollFd = ::epoll_create1(0);
      if (this->epollFd == -1) {
        throw std::runtime_error("Failed to create epoll instance");
      }
      SYS_FNCTL(this->epollFd, F_SETFD, FD_CLOEXEC);
      std::signal(SIGPIPE, SIG_IGN);
    }
  public:
    /*
     * Add file descriptor with certain flags to the epoll pool
     */
    bool add(int fd, int flags) {
      if (this->has(fd))
        this->remove(fd, false);
      epoll_event_t event;
      event.events = flags;
      event.data.fd = fd;
      if (::epoll_ctl(this->epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
        return false;
      }
      SYS_FNCTL(this->epollFd, F_SETFD, FD_CLOEXEC);
      this->fds.insert(File(fd));
      this->maxEvents++;
      if (this->events) delete[] this->events;
      this->events = new epoll_event_t[this->maxEvents];
      Logger::debug
        << "Tracking file descriptor " << Logger::param(fd) << std::newl;
      return true;
    }

    inline bool has(int fd) const {
      return this->fds.count(fd) > 0;
    }

    const std::set<File>& getAll() const {
      return this->fds;
    }
    std::set<File>& getAll() {
      return this->fds;
    }

    const File& get(int fd) const {
      std::set<File>::iterator it = this->fds.find(fd);
      if (it == this->fds.end()) {
        throw std::runtime_error("File not found");
      }
      return *it;
    }

    File& get(int fd) {
      std::set<File>::iterator it = this->fds.find(fd);
      if (it == this->fds.end()) {
        throw std::runtime_error("File not found");
      }
      return const_cast<File&>(*it);
    }

    /*
     * Remove file descriptor from the epoll pool
     * If close is true, the file descriptor will be closed
     */
    bool remove(int fd, bool close) {
      if (!this->has(fd)) return false;
      epoll_event_t event;
      if (::epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, &event) == -1) {
        Logger::error
          << "Failed to remove file descriptor " << Logger::param(fd) << " from epoll instance: "
          << Logger::errstr()
          << std::newl;
        return false;
      }
      this->fds.erase(fd);
      if (close)
        SYS_CLOSE(fd);
      this->maxEvents--;
      if (this->events) delete[] this->events;
      this->events = new epoll_event_t[this->maxEvents];
      Logger::debug
        << "Stopped tracking file descriptor " << Logger::param(fd) << std::newl;
      return true;
    }

    /*
      * Update flags of file descriptor in the epoll pool
     */
    bool update(int fd, int flags) {
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

    inline void setTimeout(int timeout) {
      this->timeout = timeout;
    }

    inline void stop() { this->running = false; }

    /*
     * Start the epoll loop
     * This function will block until stop() is called
     * If instance is set, the onTick callback will be called every time the epoll pool returns events
     * Each file descriptor that has events will be processed into a File object
     * All File objects that changed will be passed to the onTick callback
     */
    void start() {
      if (this->running) return;
      this->running = true;
      while (this->running) {
        int n = ::epoll_wait(this->epollFd, this->events, this->maxEvents, this->timeout);
        if (n == -1) {
          if (errno == EINTR) continue;
          throw std::runtime_error("epoll_wait failed");
        }
        std::vector<File> changed(n);
        for (int i = 0; i < n; i++) {
          epoll_event_t& event = this->events[i];
          std::set<File>::iterator it = this->fds.find(event.data.fd);
          if (it == this->fds.end()) continue;
          File& file = const_cast<File&>(*it);
          file.setEvents(event.events);
          changed[i] = file;
        }
        if (this->instance && this->onTick) {
          (this->instance->*this->onTick)(changed);
        }
      }
    }

  };

}