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
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_PARALLEL(x) x

///
/// @brief pseudo tags of the clause sets that OpenACC 3.3 requires on the data directives (data 2.6.5, enter / exit data 2.6.6, host_data 2.8, update 2.14.4)
/// @details Used to route or stop an OpenMP data directive converted to OpenACC when none of them survives; REQ_NONE is carried by no clause
/// (a non-empty pick with it means a mark of an undefined name or of the clause limit), O2A_GENERIC_MAP by the generic OMP_TARGET_CLAUSE_MAP
/// that the OpenACC backend drops.
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_REQ_DATA(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_REQ_ENTER(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_REQ_EXIT(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_REQ_HOST_DATA(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_REQ_UPDATE(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_REQ_NONE(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_O2A_GENERIC_MAP(x) x
///
/// @brief pseudo tag carried by the delete clause of OMP_TARGET_CLAUSE_MAP_DELETE: the OpenMP exit data directive converted to OpenACC gets one finalize clause when it is picked
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_O2A_FINALIZE(x) x
///
/// @brief pseudo tags of OMP_TARGET_CLAUSE_NUM_THREADS / OMP_TARGET_CLAUSE_THREAD_LIMIT (both become vector_length): an OpenMP compute directive converted to OpenACC drops the thread_limit one when num_threads is given too
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_O2A_NUM_THREADS(x) x
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_O2A_THREAD_LIMIT(x) x

///
/// @brief matching macro about clause for kernels construct
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_KERNELS(x) x

///
/// @brief matching macro about clause for serial construct
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_SERIAL(x) x

///
/// @brief matching macro about clause for data construct
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_DATA(x) x

///
/// @brief matching macro about clause for enter data directive
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_ENTER_DATA(x) x

///
/// @brief matching macro about clause for exit data directive
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_EXIT_DATA(x) x

///
/// @brief matching macro about clause for host_data construct
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_HOST_DATA(x) x

///
/// @brief matching macro about clause for loop construct
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_LOOP(x) x

///
/// @brief matching macro about clause for atomic directive
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_ATOMIC(x) x

///
/// @brief matching macro about clause for update directive
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_UPDATE(x) x

///
/// @brief matching macro about clause for wait directive
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_WAIT(x) x

///
/// @brief matching macro about clause for routine directive
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_ROUTINE(x) x

///
/// @brief matching macro about clause for declare directive
///
#define SOLOMON_INTERNAL_COMPARE_SOLOMON_INTERNAL_TAG_ACC_DECLARE(x) x

#endif  // !defined(SOLOMON_ACC_CLAUSE_TAG_HPP)
