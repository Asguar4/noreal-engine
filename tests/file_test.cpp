#include <gtest/gtest.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "engine/io.hpp"

static_assert(std::is_nothrow_move_constructible_v<riz::File> && std::is_nothrow_move_assignable_v<riz::File>);
static_assert(std::is_copy_constructible_v<riz::File> && std::is_copy_assignable_v<riz::File>);

namespace {
std::string tmpPath(const char* n) {
  return (std::filesystem::temp_directory_path() / n).string();
}
}

TEST(File, Missing) {
  const std::string path = tmpPath("riz_does_not_exist_xyz");
  EXPECT_THROW(riz::File f(path), std::runtime_error);
  try {
    riz::File f(path);
    FAIL() << "expected throw";
  } catch (const std::runtime_error& e) {
    EXPECT_NE(std::string(e.what()).find(path), std::string::npos);
  }
}

TEST(File, Empty) {
  auto p = tmpPath("riz_empty.bin");
  { std::ofstream o(p, std::ios::binary); }
  riz::File f(p);
  EXPECT_EQ(f.getSize(), 0u);
  EXPECT_STREQ(f.getData(), "");
  std::filesystem::remove(p);
}

TEST(File, LargeBinaryCopyMove) {
  auto p = tmpPath("riz_big.bin");
  const size_t n = 3u * 1024 * 1024 + 17;
  std::string src(n, '\0');
  for (size_t i = 0; i < n; ++i) src[i] = static_cast<char>((i * 31 + 7) % 251);
  src[5] = '\0'; src[1024 * 1024] = '\0'; src[n - 1] = '\0';
  { std::ofstream o(p, std::ios::binary); o.write(src.data(), static_cast<std::streamsize>(n)); }
  riz::File f(p);
  ASSERT_EQ(f.getSize(), n);
  EXPECT_EQ(std::memcmp(f.getData(), src.data(), n), 0);
  EXPECT_EQ(f.getData()[n], '\0');

  riz::File c(f);
  EXPECT_EQ(c.getSize(), n);
  EXPECT_NE(c.getData(), f.getData());
  EXPECT_EQ(std::memcmp(c.getData(), src.data(), n), 0);
  auto small = tmpPath("riz_small.bin");
  { std::ofstream o(small, std::ios::binary); o << "abc"; }
  riz::File d(small);
  d = f;
  EXPECT_EQ(d.getSize(), n);
  EXPECT_NE(d.getData(), f.getData());

  riz::File m(std::move(f));
  EXPECT_EQ(f.getSize(), 0u);
  EXPECT_STREQ(f.getData(), "");
  EXPECT_EQ(m.getSize(), n);
  riz::File m2(small);
  m2 = std::move(m);
  EXPECT_EQ(m.getSize(), 0u);
  EXPECT_STREQ(m.getData(), "");
  EXPECT_EQ(m2.getSize(), n);
  std::filesystem::remove(p);
  std::filesystem::remove(small);
}
