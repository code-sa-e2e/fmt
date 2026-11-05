// Formatting library for C++ - tests for fmt_vprint
//
// Copyright (c) 2012 - present, Victor Zverovich and {fmt} contributors
// All rights reserved.
//
// For the license information refer to format.h.

#include "fmt/fmt-c.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_STR_EQ(actual, expected)                                   \
  do {                                                                    \
    if (strcmp(actual, expected) != 0) {                                  \
      fprintf(stderr,                                                     \
              "\nAssertion failed:\n  Expected: \"%s\"\n  Got: \"%s\"\n", \
              expected, actual);                                          \
      exit(1);                                                            \
    }                                                                     \
  } while (0)

#define ASSERT_INT_EQ(actual, expected)                                   \
  do {                                                                    \
    if ((actual) != (expected)) {                                         \
      fprintf(stderr, "\nAssertion failed:\n  Expected: %d\n  Got: %d\n", \
              (int)(expected), (int)(actual));                            \
      exit(1);                                                            \
    }                                                                     \
  } while (0)

static FILE* open_tmp(void) {
  FILE* f = tmpfile();
  if (!f) {
    fprintf(stderr, "\nFailed to create temporary file\n");
    exit(1);
  }
  return f;
}

static void read_all(FILE* f, char* buf, size_t size) {
  rewind(f);
  size_t n = fread(buf, 1, size - 1, f);
  buf[n] = '\0';
}

static void test_each_type(void) {
  char buf[256];
  FILE* f = open_tmp();
  fmt_arg args[9] = {
      fmt_from_int(-7),        fmt_from_uint(8u),
      fmt_from_bool(true),     fmt_from_char('c'),
      fmt_from_float(1.5f),    fmt_from_double(2.25),
      fmt_from_long_double(0.5L), fmt_from_str("str"),
      fmt_from_ptr((void*)0x10)};
  int ret = fmt_vprint(f, "{} {} {} {} {} {} {} {} {}", args, 9);
  ASSERT_INT_EQ(ret, 0);
  read_all(f, buf, sizeof(buf));
  ASSERT_STR_EQ(buf, "-7 8 true c 1.5 2.25 0.5 str 0x10");
  fclose(f);
}

static void test_zero_args(void) {
  char buf[100];
  FILE* f = open_tmp();
  int ret = fmt_vprint(f, "No arguments", NULL, 0);
  ASSERT_INT_EQ(ret, 0);
  read_all(f, buf, sizeof(buf));
  ASSERT_STR_EQ(buf, "No arguments");
  fclose(f);
}

static void test_macro(void) {
  char buf[100];
  FILE* f = open_tmp();
  int ret = fmt_print(f, "{} and {}", 42, "foo");
  ASSERT_INT_EQ(ret, 0);
  read_all(f, buf, sizeof(buf));
  ASSERT_STR_EQ(buf, "42 and foo");
  fclose(f);
}

static void test_append(void) {
  char buf[100];
  FILE* f = open_tmp();
  fmt_arg a[1] = {fmt_from_int(1)};
  fmt_arg b[1] = {fmt_from_int(2)};
  ASSERT_INT_EQ(fmt_vprint(f, "a{}", a, 1), 0);
  ASSERT_INT_EQ(fmt_vprint(f, "b{}", b, 1), 0);
  read_all(f, buf, sizeof(buf));
  ASSERT_STR_EQ(buf, "a1b2");
  fclose(f);
}

static void test_max_args(void) {
  char buf[100];
  fmt_arg args[17];
  int i;
  for (i = 0; i < 17; ++i) args[i] = fmt_from_int(i % 10);
  FILE* f = open_tmp();
  int ret = fmt_vprint(f, "{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}", args, 16);
  ASSERT_INT_EQ(ret, 0);
  read_all(f, buf, sizeof(buf));
  ASSERT_STR_EQ(buf, "0123456789012345");
  fclose(f);

  f = open_tmp();
  ret = fmt_vprint(f, "{}", args, 17);
  ASSERT_INT_EQ(ret, fmt_error_invalid_arg);
  read_all(f, buf, sizeof(buf));
  ASSERT_STR_EQ(buf, "");
  fclose(f);
}

static void test_invalid_arg_type(void) {
  char buf[100];
  FILE* f = open_tmp();
  fmt_arg args[1];
  args[0].type = (fmt_type)99;
  memset(&args[0].value, 0, sizeof(args[0].value));
  int ret = fmt_vprint(f, "{}", args, 1);
  ASSERT_INT_EQ(ret, fmt_error_invalid_arg);
  read_all(f, buf, sizeof(buf));
  ASSERT_STR_EQ(buf, "");
  fclose(f);
}

static void test_format_error(void) {
  FILE* f = open_tmp();
  fmt_arg args[1] = {fmt_from_int(1)};
  ASSERT_INT_EQ(fmt_vprint(f, "{", args, 1), fmt_error);
  ASSERT_INT_EQ(fmt_vprint(f, "{} {}", args, 1), fmt_error);
  fclose(f);
}

int main(void) {
  printf("Running fmt_vprint tests\n");
  test_each_type();
  test_zero_args();
  test_macro();
  test_append();
  test_max_args();
  test_invalid_arg_type();
  test_format_error();
  printf("fmt_vprint tests passed\n");
  return 0;
}
