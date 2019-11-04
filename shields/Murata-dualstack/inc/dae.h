/*! \file dae.h
 *

 *  \copyright (C) Copyright 2015 University of Antwerp and others (http://oss-7.cosys.be)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @{
 * \brief D7A Data Elements contain D7A mandatory or optional configuration parameters, or user-defined data.
 * \author glenn.ergeerts@uantwerpen.be
 * \author	philippe.nunes@cortus.com
 */

#ifndef DAE_H_
#define DAE_H_

#include "stdint.h"
//#include "framework_defs.h"
#include "stdbool.h"

#define SUBPROFILES_NB	4
#define SUBBANDS_NB		8

#define ACCESS_SPECIFIER(val) (uint8_t)(val >> 4 & 0x0F)
#define ACCESS_MASK(val) (uint8_t)(val & 0x0F)

#ifndef FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE
#define FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE 16
#endif

typedef enum
{
    D7A_ACT_COND_LIST = 0, // check for existence
    D7A_ACT_COND_READ = 1,
    D7A_ACT_COND_WRITE = 2,
    D7A_ACT_COND_WRITEFLUSH = 3
} dae_act_condition_t;

typedef struct __attribute__((__packed__))
{
    uint16_t channel_index_start;
    uint16_t channel_index_end;
    int8_t eirp;
    uint8_t cca;  // Default Clear channel assessment threshold (-dBm)
    uint8_t duty; // Maximum per-channel transmission duty cycle in per-mil (‰)
} subband_t;

typedef struct __attribute__((__packed__))
{
    uint8_t subband_bitmap; // Bitmap of used subbands
    uint8_t scan_automation_period;
} subprofile_t;

typedef struct __attribute__((__packed__))
{
    uint8_t ch_coding: 2;     /**< The 'coding' field in the channel header */
    uint8_t ch_class: 2;      /**< The 'class' field in the channel header */
    uint8_t ch_freq_band: 3;  /**< The frequency 'band' field in the channel header */
    uint8_t _rfu: 1;
} channel_header_t;

typedef struct __attribute__((__packed__))
{
    union
    {
        uint8_t channel_header_raw;          /**< The raw (8-bit) channel header */
        channel_header_t channel_header; /**< The channel header */
    };
    subprofile_t subprofiles[SUBPROFILES_NB];
    subband_t subbands[SUBBANDS_NB];
} dae_access_profile_t;

typedef struct {
    uint8_t key_counter;
    uint32_t frame_counter;
} dae_nwl_security_t;


typedef struct {
    uint8_t key_counter;
    uint32_t frame_counter;
    uint8_t addr[8];
    //bool used;  /* to be used if it is possible to remove a trusted node from the table */
} dae_nwl_trusted_node_t;

typedef struct {
    uint8_t filter_mode;
    uint8_t trusted_node_nb;
    dae_nwl_trusted_node_t trusted_node_table[FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE];
} dae_nwl_ssr_t;

#endif /* DAE_H_ */
