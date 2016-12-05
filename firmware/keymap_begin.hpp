// Copyright 2016, Dawid Kurek, <dawikur@gmail.com>

#ifndef FIRMWARE_KEYMAP_BEGIN_HPP_
#define FIRMWARE_KEYMAP_BEGIN_HPP_

#include "config.hpp"
#include "layer.hpp"
#include "report.hpp"

#include "keymap_keys.hpp"

// Key
#define K(in_key)                                                              \
  [](Crow::Report &report, Crow::Layer &, bool const wasPressed) {             \
    report.key(Crow::Keymap::Key_##in_key, wasPressed);                        \
  }

// Modifier
#define M(in_key)                                                              \
  [](Crow::Report &report, Crow::Layer &, bool const wasPressed) {             \
    report.modifier(Crow::Keymap::Modifier_##in_key, wasPressed);              \
  }

// Modifier Lock
#define ML(in_key)                                                             \
  [](Crow::Report &report, Crow::Layer &, bool const wasPressed) {             \
    if (wasPressed) {                                                          \
      report.toggleModifierLock(Crow::Keymap::Modifier_##in_key);              \
    }                                                                          \
  }

// Layer
#define L(number)                                                              \
  [](Crow::Report &, Crow::Layer &layer, bool const wasPressed) {              \
    layer.set(wasPressed ? number : Crow::Layer::Base);                        \
  }

// Layer Lock
#define LL()                                                                   \
  [](Crow::Report &, Crow::Layer &layer, bool const wasPressed) {              \
    if (wasPressed) {                                                          \
      layer.toggleLock();                                                      \
    }                                                                          \
  }

// Customer Media
#define C(in_key)                                                              \
  [](Crow::Report &report, Crow::Layer &, bool const wasPressed) {             \
    report.media(Crow::Keymap::Media_##in_key, wasPressed);                    \
  }

#define Nop() [](Crow::Report &, Crow::Layer &, bool const) {}

#endif  // FIRMWARE_KEYMAP_BEGIN_HPP_