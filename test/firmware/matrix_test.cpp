// Copyright 2016, Dawid Kurek, <dawikur@gmail.com>

#include "matrix.hpp"

#include "gtest/gtest.h"

TEST(matrix_test, rows_and_columns_return_data_from_config) {
  Crow::Matrix matrix;

  ASSERT_EQ(Crow::RowsCount, matrix.rows());
  ASSERT_EQ(Crow::ColsCount, matrix.cols());
}

TEST(matrix_test, at_start_all_previous_rows_are_empty) {
  Crow::Matrix matrix;

  for (int i = 0; i < Crow::RowsCount; ++i) {
    ASSERT_EQ(Crow::Row{0}, matrix[i]);
  }
}

TEST(matrix_test, set_row_will_be_remembered) {
  Crow::Matrix matrix;

  matrix[3] = Crow::Row{178};

  ASSERT_EQ(Crow::Row{178}, matrix[3]);
}
