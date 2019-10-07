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
 * \author glenn.ergeerts@uantwerpen.be
 *
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

typedef enum {
  EM_MODE_OFF = 0,
  EM_MODE_CONTINUOUS_TX = 1,
  EM_MODE_TRANSIENT_TX = 2,
  EM_MODE_PER_RX = 3,
  EM_MODE_PER_TX = 4
} engineering_mode_mode_t;

/* \brief The coding schemes and corresponding indices as defined in D7A
 *
 */
typedef enum
{
    PHY_CODING_PN9 = 0x00,
    PHY_CODING_RFU = 0x01,
    PHY_CODING_FEC_PN9 = 0x02,
    PHY_CODING_CW = 0x03
} phy_coding_t;

/* \brief The channel bands and corresponding band indices as defined in D7A
 *
 */
typedef enum
{
    PHY_BAND_433 = 0x02,
    PHY_BAND_868 = 0x03,
    PHY_BAND_915 = 0x04,
} phy_channel_band_t;

/* \brief The channel classes and corresponding indices as defined in D7A
 *
 */
typedef enum
{
    PHY_CLASS_LO_RATE = 0x00, // 9.6 kbps
#ifdef USE_SX127X
    PHY_CLASS_LORA = 0x01, // LoRa SF9, BW 125, CR 4/5. Note this is _not_ part of D7A spec (for now), and subject to change (or removal)
#endif
    PHY_CLASS_NORMAL_RATE = 0x02, // 55.555 kbps
    PHY_CLASS_HI_RATE = 0x03 // 166.667 kbps
} phy_channel_class_t;

/* \brief The channel header as defined in D7AP
 */
typedef struct __attribute__((__packed__))
{
    phy_coding_t ch_coding: 2; 	/**< The 'coding' field in the channel header */
    phy_channel_class_t ch_class: 2;  	/**< The 'class' field in the channel header */
    phy_channel_band_t ch_freq_band: 3;	/**< The frequency 'band' field in the channel header */
    uint8_t _rfu: 1;
} phy_channel_header_t;

/** \brief channel id used to identify the spectrum settings
 *
 * This struct adheres to the 'Channel ID' format the Dash7 PHY layer. (@17/03/2015)
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        uint8_t channel_header_raw; 	/**< The raw (8-bit) channel header */
        phy_channel_header_t channel_header; /**< The channel header */
    };
    uint16_t center_freq_index;		/**< The center frequency index of the channel id */
} channel_id_t;

typedef struct __attribute__((__packed__))
{
  uint8_t mode;
  uint8_t flags;
  uint8_t timeout;
  channel_id_t channel_id;
  int8_t eirp;
} em_file_t;

typedef enum
{
    CSMA_CA_MODE_UNC = 0,
    CSMA_CA_MODE_AIND = 1,
    CSMA_CA_MODE_RAIND = 2,
    CSMA_CA_MODE_RIGD = 3
} csma_ca_mode_t; // TODO move

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

// typedef struct {
//     uint8_t filter_mode;
//     uint8_t trusted_node_nb;
//     dae_nwl_trusted_node_t trusted_node_table[FRAMEWORK_FS_TRUSTED_NODE_TABLE_SIZE];
// } dae_nwl_ssr_t;

typedef enum
{
    FS_STORAGE_TRANSIENT = 0,
    FS_STORAGE_VOLATILE = 1,
    FS_STORAGE_RESTORABLE = 2,
    FS_STORAGE_PERMANENT = 3
} fs_storage_class_t;

typedef enum
{
    ALP_ACT_COND_LIST = 0,
    ALP_ACT_COND_READ = 1,
    ALP_ACT_COND_WRITE = 2,
    ALP_ACT_COND_WRITEFLUSH = 3
} alp_act_condition_t;

typedef struct __attribute__((__packed__))
{
    fs_storage_class_t storage_class : 2;
    uint8_t _rfu : 2;
    alp_act_condition_t action_condition : 3;
    bool action_protocol_enabled : 1;
} fs_file_properties_t;

typedef struct __attribute__((__packed__))
{
    uint8_t file_permissions; // TODO not used for now
    fs_file_properties_t file_properties;
    uint8_t alp_cmd_file_id;
    uint8_t interface_file_id;
    uint32_t length;
    uint32_t allocated_length;
} fs_file_header_t;
// TODO ALP depends on this struct for now, but this is D7A specific. Refactor?

#endif /* DAE_H_ */
