/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TEST_SST_NV_COUNTERS_H__
#define __TEST_SST_NV_COUNTERS_H__

#include <stdint.h>
#include "tfm_sst_defs.h"
#include "platform/include/tfm_plat_nv_counters.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Reads the given non-volatile (NV) counter.
 *
 * \param[in]  counter_id  NV counter ID.
 * \param[out] val         Pointer to store the current NV counter value.
 *
 * \return  TFM_SST_ERR_SUCCESS if the value is read correctly, otherwise
 *          TFM_SST_ERR_OPERATION_FAILED
 */
enum tfm_sst_err_t test_sst_read_nv_counter(enum tfm_nv_counter_t counter_id,
                                            uint32_t *val);

/**
 * \brief Increments the given non-volatile (NV) counter.
 *
 * \param[in] counter_id  NV counter ID.
 *
 * \return  When the NV counter reaches its maximum value, the
 *          TFM_SST_ERR_OPERATION_FAILED error is returned to indicate the value
 *          cannot be incremented. Otherwise, TFM_SST_ERR_SUCCESS.
 */
enum tfm_sst_err_t test_sst_increment_nv_counter(
                                              enum tfm_nv_counter_t counter_id);

/**
 * \brief Decrements the given non-volatile (NV) counter.
 *
 * \param[in] counter_id  NV counter ID.
 *
 * \return  When the NV counter reaches its minimum value, the
 *          TFM_SST_ERR_OPERATION_FAILED error is returned to indicate the value
 *          cannot be decremented. Otherwise, TFM_SST_ERR_SUCCESS.
 */
enum tfm_sst_err_t test_sst_decrement_nv_counter(
                                              enum tfm_nv_counter_t counter_id);

/**
 * \brief Disables SST increment nv counter function to force
 *        TFM_SST_ERR_OPERATION_FAILED return value as an indication that NV
 *        counter reaches its maximum value.
 */
void test_sst_disable_increment_nv_counter(void);

/**
 * \brief Enables SST increment nv counter function to work normally.
 */
void test_sst_enable_increment_nv_counter(void);

/**
 * \brief Sets a new value into the given non-volatile (NV) counter.
 *
 * \param[in] counter_id  NV counter Id.
 * \param[in] value       New NV counter value.
 *
 * \return  When the NV counter reaches its maximum value, the
 *          TFM_SST_ERR_OPERATION_FAILED error is returned to indicate the value
 *          cannot be set. Otherwise, TFM_SST_ERR_SUCCESS.
 */
enum tfm_sst_err_t test_sst_set_nv_counter(enum tfm_nv_counter_t counter_id,
                                           uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_SST_NV_COUNTERS_H__ */

