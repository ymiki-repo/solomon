///
/// @file filename.hpp
/// @author Yohei MIKI (Information Technology Center, The University of Tokyo)
/// @brief file names of the outputs
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#pragma once

#include <stddef.h>

///
/// @brief write a file name into buf (printf-style); stop the program when the name does not fit
///
/// @param[out] buf file name
/// @param[in] size size of buf in bytes
/// @param[in] format printf-style format of the file name
///
void set_filename(char* buf, const size_t size, const char* format, ...) __attribute__((format(printf, 3, 4)));
