/**
 * Instance.hpp
 * Singleton design pattern with templates.
 * Singleton: an object with a default construction that is only constructed once.
 * This is useful for objects that are used everywhere, like ServerManager.
*/
#pragma once

// Singleton

class Instance {
public:
  template <typename T>
  static T* Get() {
    static T instance;
    return &instance;
  }
};