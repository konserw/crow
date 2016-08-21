// Copyright 2016, Dawid Kurek, <dawikur@gmail.com>

#include "firmware.hpp"

#include <vector>

#include "gtest.h"

#include "mock.hpp"

namespace Functions {

Crow::Mock::Hardware *_hardware;
Crow::Mock::USBHid *_usbHid;

Crow::Row getRow(Crow::Index const i) {
  return _hardware->getRow(i);
}

void sendReport(void const *const data, Crow::Index const size) {
  auto *bytes = reinterpret_cast<uint8_t const *>(data);
  std::vector<uint8_t> buffor(bytes, bytes + size);
  return _usbHid->sendReport(buffor);
}

}  // namespace Functions

class firmware_test : public ::testing::Test {
 protected:
  void SetUp() override {
    Functions::_hardware = &hardware;
    Functions::_usbHid = &usbHid;
    firmware.setup(Functions::getRow, Functions::sendReport);
  }

  void TearDown() override {
    Functions::_usbHid = nullptr;
    Functions::_hardware = nullptr;
  }

  void expect_rows(std::vector<Crow::RawRow> const &rows) {
    for (Crow::Index index = 0; index < Crow::RowsCount; ++index) {
      EXPECT_CALL(hardware, getRow(index))
        .WillOnce(::testing::Return(Crow::Row{rows[index]}));
    }
  }

  void expect_report(std::vector<uint8_t> const &bytes) {
    EXPECT_CALL(usbHid, sendReport(bytes))
      .Times(1);
  }

  Crow::Firmware firmware;

  Crow::Mock::Hardware hardware;
  Crow::Mock::USBHid usbHid;
};

TEST_F(firmware_test, not_pressing_any_key_will_not_send_report) {
  expect_rows({0, 0, 0, 0, 0});

  firmware.loop();
}

TEST_F(firmware_test, pressing_one_key_will_send_report_with_that_key) {
  expect_rows({0,0, 0x02, 0, 0});
  expect_report({0, 0, Crow::Keymap::Key_A, 0, 0, 0, 0, 0});

  firmware.loop();
}


