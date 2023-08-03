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