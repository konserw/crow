// Copyright 2016, Dawid Kurek, <dawikur@gmail.com>

#include "firmware.hpp"

#include <vector>

#include "gtest/gtest.h"

#include "mock.hpp"

/*
 * For future maintenance comments are added:
 * + press
 * - release
 * ? is pressed
 * ! is released
 */

namespace Functions {

#define set_rows(row0, row1, row2, row3, row4)                                 \
  EXPECT_CALL(hardware, getRow(0))                                             \
    .WillOnce(::testing::Return(Crow::Row{row0}));                             \
  EXPECT_CALL(hardware, getRow(1))                                             \
    .WillOnce(::testing::Return(Crow::Row{row1}));                             \
  EXPECT_CALL(hardware, getRow(2))                                             \
    .WillOnce(::testing::Return(Crow::Row{row2}));                             \
  EXPECT_CALL(hardware, getRow(3))                                             \
    .WillOnce(::testing::Return(Crow::Row{row3}));                             \
  EXPECT_CALL(hardware, getRow(4))                                             \
    .WillOnce(::testing::Return(Crow::Row{row4}));

#define expect_report(...)                                                     \
  EXPECT_CALL(usbHid, sendReport(std::vector<uint8_t>(__VA_ARGS__)))           \
    .WillOnce(::testing::Return(1))

Crow::Mock::Hardware *_hardware;
Crow::Mock::USBHid *_usbHid;

Crow::Row getRow(Crow::Index const i) {
  return _hardware->getRow(i);
}

void sendReport(Crow::Index const id,
                void const *const data,
                Crow::Index const size) {
  auto *bytes = reinterpret_cast<uint8_t const *>(data);
  std::vector<uint8_t> buffor(bytes, bytes + size);
  _usbHid->sendReport(buffor);
}

void setLayer(Crow::Index const) {}

}  // namespace Functions

class firmware_test : public ::testing::Test {
 protected:
  firmware_test()
    : firmware{Functions::getRow, Functions::sendReport, Functions::setLayer} {}

  void SetUp() override {
    Functions::_hardware = &hardware;
    Functions::_usbHid = &usbHid;

  }

  void TearDown() override {
    Functions::_usbHid = nullptr;
    Functions::_hardware = nullptr;
  }

  void loop() {
    firmware.loop();
  }

  Crow::Mock::Hardware hardware;
  Crow::Mock::USBHid usbHid;

 private:
  Crow::Firmware firmware;
};

using namespace Crow::Keymap;

TEST_F(firmware_test, not_pressing_any_key_will_not_send_report) {
  set_rows(0, 0, 0, 0, 0);                                                     //

  loop();
}

TEST_F(firmware_test, pressing_one_key_will_send_report_with_that_key) {
  set_rows(0, 0, 2, 0, 0);                                                     // + a
  expect_report({0, 0, Key_A, 0, 0, 0, 0, 0});                                 // ? a

  loop();
}

TEST_F(firmware_test, pressing_and_releasing_key_will_send_two_reports) {
  set_rows(0, 0, 4, 0, 0);                                                     // + s
  expect_report({0, 0, Key_S, 0, 0, 0, 0, 0});                                 // ? s

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - s
  expect_report({0, 0, 0, 0, 0, 0, 0, 0});                                     // ! s

  loop();
}

TEST_F(firmware_test,
       when_pressing_three_keys_they_will_be_send_in_one_report) {
  set_rows(32, 4, 64, 0,0);                                                    // + 5 w h
  expect_report({0, 0, Key_5, Key_W, Key_H, 0, 0, 0});                         // ? 5 w h

  loop();
}

TEST_F(firmware_test, pressed_modifier_will_be_send) {
  set_rows(0, 0, 1, 0, 0);                                                     // + CtrlL
  expect_report({Modifier_CtrlL, 0, 0, 0, 0, 0, 0, 0});                        // ? CtrlL

  loop();
}

TEST_F(firmware_test, pressed_multiple_modifiers_will_be_send) {
  set_rows(0, 0, 1, 1, 0);                                                     // + CtrlL ShiftL
  expect_report({Modifier_CtrlL | Modifier_ShiftL, 0, 0, 0, 0, 0, 0, 0});      // ? CtrlL ShiftL

  loop();
}

TEST_F(firmware_test, modifiers_can_be_unpressed) {
  set_rows(0, 0, 0, 0, 256);                                                   // + AltR
  expect_report({Modifier_AltR, 0, 0, 0, 0, 0, 0, 0});                         // ? AltR

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - AltR
  expect_report({0, 0, 0, 0, 0, 0, 0, 0});                                     // ! AltR

  loop();
}

TEST_F(firmware_test, modifiers_press_can_be_combined_with_normal_keys) {
  set_rows(0, 32, 1, 8, 256);                                                  // + AltR CtrlL t c
  expect_report({Modifier_AltR | Modifier_CtrlL, 0, Key_T, Key_C, 0, 0, 0, 0});// ? AltR CrtlL t c

  loop();
}

TEST_F(firmware_test, first_layer_can_be_reached) {
  set_rows(0, 0, 0, 0, 1);                                                     // + Layer1

  loop();

  set_rows(2, 0, 0, 0, 1);                                                     // + 2
  expect_report({0, 0, Key_F1, 0, 0, 0, 0, 0});                                // ? F2

  loop();
}

TEST_F(firmware_test, can_click_on_first_layer_and_than_back_on_default_one) {
  set_rows(0, 0, 0, 0, 1);                                                     // + Layer1

  loop();

  set_rows(0, 0, 512, 0, 1);                                                   // + l
  expect_report({0, 0, Key_Right, 0, 0, 0, 0, 0});                             // ? RightKey

  loop();


  set_rows(0, 0, 0, 0, 0);                                                     // - Layer1 l
  expect_report({0, 0, 0, 0, 0, 0, 0, 0});                                     // !

  loop();

  set_rows(4, 0, 0, 0, 0);                                                     // + 2
  expect_report({0, 0, Key_2, 0, 0, 0, 0, 0});                                 // ? 2

  loop();
}

TEST_F(firmware_test, layer_can_be_toggled) {
  set_rows(0, 0, 0, 0, 1);                                                     // + Layer1

  loop();

  set_rows(0, 1, 0, 0, 1);                                                     // + LayerToggle

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // -

  loop();

  set_rows(2, 0, 0, 0, 0);                                                     // + 1
  expect_report({0, 0, Key_F1, 0, 0, 0, 0, 0});                                // ? F1

  loop();

  set_rows(0, 0, 64, 0, 0);                                                    // + h
  expect_report({0, 0, Key_Left, 0, 0, 0, 0, 0});                              // ? Leftkey

  loop();
}

TEST_F(firmware_test, layer_can_be_toggled_twice) {
  set_rows(0, 0, 0, 0, 1);                                                     // + Layer1

  loop();

  set_rows(0, 1, 0, 0, 1);                                                     // + LayerToggle

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - Layer1 LayerToggle

  loop();

  set_rows(8, 0, 0, 0, 0);                                                     // + 3
  expect_report({0, 0, Key_F3, 0, 0, 0, 0, 0});                                // ? F3

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - 3
  expect_report({0, 0, 0, 0, 0, 0, 0, 0});

  loop();

  set_rows(0, 1, 0, 0, 1);                                                     // + Layer1 LayerToggle

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - Layer1 LayerToggle

  loop();

  set_rows(8, 0, 0, 0, 0);                                                     // + 3
  expect_report({0, 0, Key_3, 0, 0, 0, 0, 0});                                 // ? 3

  loop();
}

TEST_F(firmware_test, shift_locking_and_unlocking_works_on_layer_2) {
  set_rows(0, 0, 0, 0, 2048);                                                  // + Layer2

  loop();

  set_rows(0, 0, 0, 2048, 2048);                                               // + ShiftLock
  expect_report({Modifier_ShiftR, 0, 0, 0, 0, 0, 0, 0});

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - Layer2 ShiftLock

  loop();

  set_rows(4, 0, 0, 0, 0);                                                     // + 2
  expect_report({Modifier_ShiftR, 0, Key_2, 0, 0, 0, 0, 0});                   // ? @

  loop();

  set_rows(0, 0, 0, 0, 2048);                                                  // + Layer2
  expect_report({Modifier_ShiftR, 0, 0, 0, 0, 0, 0, 0});

  loop();

  set_rows(0, 0, 0, 2048, 2048);                                               // + ShiftLock
  expect_report({0, 0, 0, 0, 0, 0, 0, 0});

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - Layer2 ShiftLock

  loop();

  set_rows(4, 0, 0, 0, 0);                                                     // + 2
  expect_report({0, 0, Key_2, 0, 0, 0, 0, 0});                                 // ? 2

  loop();
}

TEST_F(firmware_test, can_lock_both_shift_keys) {
  set_rows(0, 0, 0, 0, 2048);                                                  // + Layer2

  loop();

  set_rows(0, 0, 0, 2049, 2048);                                               // + ShiftLock
  expect_report({Modifier_ShiftL | Modifier_ShiftR, 0, 0, 0, 0, 0, 0, 0});     // ? Shift? ShiftR

  loop();
}

TEST_F(firmware_test, when_changing_layer_all_keys_are_cleared) {
  set_rows(0, 0, 0, 0, 1);                                                     // + Layer1

  loop();

  set_rows(1, 0, 0, 0, 1);                                                     // + Esc
  expect_report({0, 0, Key_Esc, 0, 0, 0, 0, 0});                               // ? Esc

  loop();

  set_rows(1, 0, 0, 0, 0);                                                     // - Layer1
  expect_report({0, 0, 0, 0, 0, 0, 0, 0});                                     // ! Esc

  loop();
}

TEST_F(firmware_test, when_changing_layer_modifier_keys_are_not_cleared) {
  set_rows(0, 0, 0, 1, 1);                                                     // + Layer1 ShiftL
  expect_report({Modifier_ShiftL, 0, 0, 0, 0, 0, 0, 0});                       // ? ShiftL

  loop();

  set_rows(0, 0, 0, 1, 0);                                                     // - Layer1

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - ShiftL
  expect_report({0, 0, 0, 0, 0, 0, 0, 0});                                     // ! ShiftL

  loop();
}

TEST_F(firmware_test, on_non_default_layer_we_can_switch_to_base_one) {
  set_rows(0, 0, 0, 0, 1);                                                     // + Layer1

  loop();

  set_rows(0, 1, 0, 0, 1);                                                     // + LayerLock

  loop();

  set_rows(0 ,0, 0, 0, 0);                                                     // - Layer1 LayerLock

  loop();

  set_rows(2, 0, 0, 0, 0);                                                     // + 1
  expect_report({0, 0, Key_F1, 0, 0, 0, 0, 0});                                // ? F1

  loop();

  set_rows(0, 0, 0, 0, 0);                                                     // - 1
  expect_report({0, 0, 0, 0, 0, 0, 0, 0});                                     // ! 1

  loop();

  set_rows(0, 0, 0, 0, 1);                                                     // + Layer0

  loop();

  set_rows(2, 0, 0, 0, 1);                                                     // + 1
  expect_report({0, 0, Key_1, 0, 0, 0, 0, 0});                                 // ? 1

  loop();
}
