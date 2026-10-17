#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>
#include <string>

#include "containers/array.hpp"

using riz::Vector;

TEST(Vector, DefaultEmpty) {
  Vector<int> v;
  EXPECT_EQ(v.size(), 0u);
  EXPECT_EQ(v.capacity(), 0u);
}

TEST(Vector, InitList) {
  Vector<int> v{1, 2, 3};
  ASSERT_EQ(v.size(), 3u);
  EXPECT_EQ(v[0], 1); EXPECT_EQ(v[1], 2); EXPECT_EQ(v[2], 3);
}

TEST(Vector, ResizeZeroFills) {
  Vector<int> v;
  v.resize(5);
  ASSERT_EQ(v.size(), 5u);
  for (size_t i = 0; i < 5; ++i) EXPECT_EQ(v[i], 0);
}

TEST(Vector, ResizeWithValue) {
  Vector<int> v{1, 2};
  v.resize(5, 7);
  ASSERT_EQ(v.size(), 5u);
  int exp[] = {1, 2, 7, 7, 7};
  for (size_t i = 0; i < 5; ++i) EXPECT_EQ(v[i], exp[i]);
}

TEST(Vector, ShrinkResize) {
  Vector<int> v{4, 5, 6};
  v.resize(1);
  ASSERT_EQ(v.size(), 1u);
  EXPECT_EQ(v[0], 4);
}

TEST(Vector, Reserve) {
  Vector<int> v{1, 2, 3};
  v.reserve(100);
  EXPECT_GE(v.capacity(), 100u);
  ASSERT_EQ(v.size(), 3u);
  EXPECT_EQ(v[0], 1); EXPECT_EQ(v[2], 3);
  size_t cap = v.capacity();
  v.reserve(10);
  EXPECT_EQ(v.capacity(), cap);
}

TEST(Vector, ShrinkToFit) {
  Vector<int> v{1, 2, 3};
  v.reserve(50);
  v.shrinkToFit();
  EXPECT_EQ(v.capacity(), 3u);
  EXPECT_EQ(v[1], 2);
  Vector<int> e;
  e.reserve(10);
  e.shrinkToFit();
  EXPECT_EQ(e.capacity(), 0u);
}

TEST(Vector, CopyIndependent) {
  Vector<int> a{1, 2, 3};
  Vector<int> b(a);
  Vector<int> c;
  c = a;
  a[0] = 99;
  EXPECT_EQ(b.size(), 3u); EXPECT_EQ(b[0], 1);
  EXPECT_EQ(c.size(), 3u); EXPECT_EQ(c[0], 1); EXPECT_EQ(c[2], 3);
}

TEST(Vector, Move) {
  Vector<int> a{1, 2, 3};
  Vector<int> b(std::move(a));
  EXPECT_EQ(a.size(), 0u); EXPECT_EQ(a.capacity(), 0u);
  EXPECT_EQ(b.size(), 3u);
  Vector<int> c;
  c = std::move(b);
  EXPECT_EQ(b.size(), 0u); EXPECT_EQ(b.capacity(), 0u);
  EXPECT_EQ(c[2], 3);
}

TEST(Vector, SelfAssign) {
  Vector<int> a{1, 2, 3};
  Vector<int>& r = a;
  a = r;
  EXPECT_EQ(a.size(), 3u); EXPECT_EQ(a[1], 2);
  a = std::move(r);
  EXPECT_EQ(a.size(), 3u); EXPECT_EQ(a[2], 3);
}

TEST(Vector, ReserveTooLargeThrows) {
  Vector<int> v;
  EXPECT_THROW(v.reserve(v.maxSize() + 1), std::length_error);
}

namespace {
struct Thrower {
  static int live;
  static int countdown;  // throw when reaches 0 on assignment
  int v = 0;
  Thrower() { ++live; }
  Thrower(int x) : v(x) { ++live; }
  Thrower(const Thrower& o) : v(o.v) { ++live; }
  ~Thrower() { --live; }
  Thrower& operator=(const Thrower& o) {
    if (countdown > 0 && --countdown == 0) throw std::runtime_error("assign");
    v = o.v;
    return *this;
  }
};
int Thrower::live = 0;
int Thrower::countdown = 0;
}  // namespace

TEST(Vector, ReserveExceptionSafe) {
  Thrower::countdown = 0;
  {
    Vector<Thrower> v{Thrower(1), Thrower(2), Thrower(3), Thrower(4)};
    size_t cap = v.capacity();
    Thrower::countdown = 3;
    EXPECT_THROW(v.reserve(100), std::runtime_error);
    Thrower::countdown = 0;
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v.capacity(), cap);
    for (int i = 0; i < 4; ++i) EXPECT_EQ(v[i].v, i + 1);
  }
  EXPECT_EQ(Thrower::live, 0);
}

TEST(Vector, StringResizeCycle) {
  Vector<std::string> v;
  v.resize(4, std::string(50, 'x'));
  v.resize(1);
  v.resize(6, "abc");
  ASSERT_EQ(v.size(), 6u);
  EXPECT_EQ(v[0], std::string(50, 'x'));
  EXPECT_EQ(v[1], "abc");
  v.resize(0);
  v.resize(3);
  EXPECT_EQ(v[2], "");
}
