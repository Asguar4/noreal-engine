#include <gtest/gtest.h>

#include "containers/array.hpp"

TEST(Smoke, VectorStartsEmpty) {
  riz::Vector<int> v;
  EXPECT_EQ(v.size(), 0u);
  EXPECT_EQ(v.capacity(), 0u);
}
