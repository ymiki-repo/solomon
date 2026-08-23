///
/// @file solomon/acc/clause_tag.hpp
/// @author Yohei MIKI (The University of Tokyo)
/// @brief tag about optional clauses for OpenACC directives
///
/// @copyright Copyright (c) 2024 Yohei MIKI
///
/// The MIT License is applied to this software, see LICENSE.txt
///
#if !defined(SOLOMON_ACC_CLAUSE_TAG_HPP)
#define SOLOMON_ACC_CLAUSE_TAG_HPP

///
/// @brief matching macro about clause for parallel construct
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_PARALLEL(x) x

///
/// @brief matching macro about clause for kernels construct
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_KERNELS(x) x

///
/// @brief matching macro about clause for serial construct
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_SERIAL(x) x

///
/// @brief matching macro about clause for data construct
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_DATA(x) x

///
/// @brief matching macro about clause for enter data directive
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA(x) x

///
/// @brief matching macro about clause for exit data directive
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA(x) x

///
/// @brief matching macro about clause for host_data construct
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_HOST_DATA(x) x

///
/// @brief matching macro about clause for loop construct
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_LOOP(x) x

///
/// @brief matching macro about clause for atomic directive
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_ATOMIC(x) x

///
/// @brief matching macro about clause for update directive
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_UPDATE(x) x

///
/// @brief matching macro about clause for wait directive
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_WAIT(x) x

///
/// @brief matching macro about clause for routine directive
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_ROUTINE(x) x

///
/// @brief matching macro about clause for declare directive
///
#define COMPARE_SOLOMON_INTERNAL_TAG_ACC_DECLARE(x) x

#endif  // !defined(SOLOMON_ACC_CLAUSE_TAG_HPP)
