///
/// @file filename.cpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief file names of the outputs
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#include "filename.hpp"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void set_filename(char* buf, const size_t size, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  const int len = vsnprintf(buf, size, format, ap);
  va_end(ap);
  if (len < 0) {
    fprintf(stderr, "ERROR: failed to write a file name with \"%s\"\n", format);
    exit(1);
  }
  if (static_cast<size_t>(len) >= size) {
    fprintf(stderr, "ERROR: the file name \"%s...\" is too long (%d bytes; at most %zu bytes)\n", buf, len, size - 1);
    exit(1);
  }
}
