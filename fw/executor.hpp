// Copyright 2016, Dawid Kurek, <dawikur@gmail.com>

#ifndef _EXECUTOR_HPP_
#define _EXECUTOR_HPP_

#include "config.hpp"
#include "event.hpp"

namespace Crow {

class Executor_ {
 public:
  Executor_() = default;
  Executor_(Executor_ const &) = delete;

  void operator() () {
    if (updateNeeded) {
      updateNeeded = false;
    }
  }

  void operator() (Event const event) {
    updateNeeded = true;
  }

 private:
  bool updateNeeded;
};

Executor_& Executor() {
  static Executor_ impl;
  return impl;
}

}  // namespace Crow

#endif  // _EXECUTOR_HPP_
