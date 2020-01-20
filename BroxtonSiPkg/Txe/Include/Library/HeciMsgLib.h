/** @file
  Header file for Heci Message functionality

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2018 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/

#ifndef _HECI_MESSAGE_LIB_H_
#define _HECI_MESSAGE_LIB_H_

#include <CoreBiosMsg.h>

#include <Protocol/Heci.h>
#include <Protocol/SeCPlatformPolicy.h>
#include <HeciRegs.h>

#define MAX_MBP_ITEM_COUNT               0x100
#define MAX_MBP_ITEM_TOTAL_SIZE          (sizeof (UINT32) * (MAX_MBP_ITEM_COUNT))
#define HEADER_LENGTH                    0x2

//
// Defining MBP APP Ids
//
#define MBP_APP_ID_KERNEL       1
#define MBP_APP_ID_DNX          7
#define MBP_APP_ID_NVM          8

//
// Defining MBP Item Ids
//
#define MBP_ITEM_IAFW_DNX_REQ             1
#define MBP_ITEM_ID_SMM_TRUSTED_CHANNEL   2
#define MBP_ITEM_ID_BIOS_PLATFORM_KEY     4

///
/// Reset Request Origin Codes.
///
#define PEI_HECI_REQ_ORIGIN_BIOS_MEMORY_INIT    0x01
#define PEI_HECI_REQ_ORIGIN_BIOS_POST           0x02
#define PEI_HECI_REQ_ORIGIN_AMTBX_LAN_DISABLE   0x03

///
/// Unconfiguration Command status
///
#define SEC_UNCONFIG_SUCCESS                    0x03
#define SEC_UNCONFIG_IN_PROGRESS                0x01
#define SEC_UNCONFIG_NOT_IN_PROGRESS            0x02
#define SEC_UNCONFIG_ERROR                      0x80

//
// End of Post Codes
//
#define HECI_EOP_STATUS_SUCCESS                 0x0
#define HECI_EOP_PERFORM_GLOBAL_RESET           0x1

#define EFI_SEC_FW_SKU_VARIABLE_GUID \
  { \
    0xe1a21d94, 0x4a20, 0x4e0e, 0xae, 0x9, 0xa9, 0xa2, 0x1f, 0x24, 0xbb, 0x9e \
  }

typedef struct {
  UINT32  SeCEnabled : 1;          ///< [0]     SEC enabled/Disabled
  UINT32  Reserved : 12;           ///< [12:1]  Reserved, must set to 0
  UINT32  AtSupported : 1;         ///< [13]    AT Support
  UINT32  Reserved2 : 18;          ///< [31:14] Intel KVM supported
  UINT32  SeCMinorVer : 16;        ///< [47:32] SEC FW Minor Version.
  UINT32  SeCMajorVer : 16;        ///< [63:48] SEC FW Major Version.
  UINT32  SeCBuildNo : 16;         ///< [79:64] SEC FW Build Number.
  UINT32  SeCHotFixNo : 16;        ///< [95:80] SEC FW Hotfix Number
} SEC_CAP;

#define SET_LOCK_MASK                           0
#define GET_LOCK_MASK                           1

#define MAX_ASSET_TABLE_ALLOCATED_SIZE          0x2000
#define HECI_HWA_CLIENT_ID                      11
#define HWA_TABLE_PUSH_CMD                      0

#define MAX_FWU_DATA_SEGMENT_SIZE               128
#define FWU_PWD_MAX_SIZE                        32
#define HECI2_HMAC_SHA256_SIGNATURE_SIZE        32


// Typedef for the commands serviced by the Fw Update service
typedef enum
{
   FWU_GET_VERSION = 0,
   FWU_GET_VERSION_REPLY,
   FWU_START,
   FWU_START_REPLY,
   FWU_DATA,
   FWU_DATA_REPLY,    //5
   FWU_END,
   FWU_END_REPLY,
   FWU_GET_INFO,
   FWU_GET_INFO_REPLY,
   FWU_GET_FEATURE_STATE,  //10
   FWU_GET_FEATURE_STATE_REPLY,
   FWU_GET_FEATURE_CAPABILITY,
   FWU_GET_FEATURE_CAPABILITY_REPLY,
   FWU_GET_PLATFORM_TYPE,
   FWU_GET_PLATFORM_TYPE_REPLY,  //15
   FWU_VERIFY_OEMID,
   FWU_VERIFY_OEMID_REPLY,
   FWU_GET_OEMID,
   FWU_GET_OEMID_REPLY,
   FWU_IMAGE_COMPATABILITY_CHECK,    //20
   FWU_IMAGE_COMPATABILITY_CHECK_REPLY,
   FWU_GET_UPDATE_DATA_EXTENSION,
   FWU_GET_UPDATE_DATA_EXTENSION_REPLY,
   FWU_GET_RESTORE_POINT_IMAGE,
   FWU_GET_RESTORE_POINT_IMAGE_REPLY,
   FWU_GET_IPU_PT_ATTRB,
   FWU_GET_IPU_PT_ATTRB_REPLY,
   FWU_GET_FWU_INFO_STATUS,
   FWU_GET_FWU_INFO_STATUS_REPLY,
   GET_ME_FWU_INFO,
   GET_ME_FWU_INFO_REPLY,
   FWU_CONFIRM_LIVE_PING,
   FWU_CONFIRM_LIVE_PING_REPLY,
   FWU_VERIFY_PWD,
   FWU_VERIFY_PWD_REPLY,
   FWU_INVALID_REPLY = 0xFF
} FWU_HECI_MESSAGE_TYPE;

#define HBM_CMD_ENUM                    0x04
#define HBM_CMD_ENUM_REPLY              0x84
#define HBM_CMD_CLIENT_PROP             0x05
#define HBM_CMD_CLIENT_PROP_REPLY       0x85
#define HBM_CMD_CONNECT                 0x06
#define HBM_CMD_CONNECT_REPLY           0x86
#define HBM_CMD_DISCONNECT              0x07
#define HBM_CMD_DISCONNECT_REPLY        0x87
#define HBM_CMD_FLOW_CONTROL            0x08

#define HECI_HBM_MSG_ADDR               0
#define HECI_MKHI_FWU_GROUP_ID          0x06
#define HECI_MKHI_FWUSTATUS_CMD_ID      0x02

#define ONE_SECOND_TIMEOUT              1000000
#define FWU_TIMEOUT                     90

//
// IFWI Prepare For Update (IPFU) FLAG Status definitions
// Flags: 0b00 - In progress,  0b01 - not complete try again, 0b10 - Successful, 0b11 - Error,
//
#define IPFU_IN_PROGRESS  0x00
#define IPFU_TRY_AGAIN    0x01
#define IPFU_SUCCESS      0x02
#define IPFU_ERROR        0x03
#define IPFU_TIMEOUT      60  // 60 Sec

#pragma pack(1)

typedef enum _HWAI_TABLE_TYPE_INDEX
{
  HWAI_TABLE_TYPE_INDEX_FRU_DEVICE  = 0,
  HWAI_TABLE_TYPE_INDEX_MEDIA_DEVICE,
  HWAI_TABLE_TYPE_INDEX_SMBIOS,
  HWAI_TABLE_TYPE_INDEX_ASF,
  HWAI_TABLE_TYPE_INDEX_MAX         = 4,
} HWAI_TABLE_TYPE_INDEX;

typedef struct _SINGLE_TABLE_POSITION {
  UINT16  Offset;
  UINT16  Length;
} SINGLE_TABLE_POSITION;

typedef struct _TABLE_PUSH_DATA {
  SINGLE_TABLE_POSITION Tables[HWAI_TABLE_TYPE_INDEX_MAX];
  UINT8                 TableData[1];
} TABLE_PUSH_DATA;

typedef union {
  UINT32  Data;
  struct {
    UINT32  MessageLength : 16;
    UINT32  Command : 4;          // only supported command would be HWA_TABLE_PUSH=0;
    UINT32  Reserved : 12;
  } Fields;
} AU_MESSAGE_HEADER;

typedef struct _AU_TABLE_PUSH_MSG {
  AU_MESSAGE_HEADER Header;
  TABLE_PUSH_DATA   Data;
} AU_TABLE_PUSH_MSG;

typedef struct _UPDATE_FLAGS {
  UINT32  RestorePoint:1;
  UINT32  RestartOperation:1;
  UINT32  UserRollback:1;
  UINT32  Reserved:29;
 } UPDATE_FLAGS;

typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
}OEM_UUID;

typedef struct _VERSION {
  UINT16 Major;
  UINT16 Minor;
  UINT16 Hotfix;
  UINT16 Build;
} VERSION;

typedef struct _FWU_GET_VERSION_MSG {
  UINT32 MessageType;
} FWU_GET_VERSION_MSG;

typedef struct _FWU_GET_VERSION_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status;
  UINT32 Sku;
  UINT32 SCVer;
  UINT32 Vendor;
  UINT32 LastFwUpdateStatus;
  UINT32 HwSku;
  VERSION CodeVersion;
  VERSION AMTVersion;
  UINT16  EnabledUpdateInterfaces;
  UINT16  SvnInFlash;
  UINT32  DataFormatVersion;
  UINT32  LastUpdateResetType;
} FWU_GET_VERSION_MSG_REPLY;

typedef struct _FWU_GET_OEMID_MSG {
  UINT32  MessageType;
} FWU_GET_OEMID_MSG;

typedef struct _FWU_GET_OEMID_MSG_REPLY {
  UINT32  MessageType;
  UINT32  Status;
  OEM_UUID OemId;
} FWU_GET_OEMID_MSG_REPLY;

typedef struct _FWU_VERIFY_OEMID_MSG {
  UINT32 MessageType;
  OEM_UUID OemId;
} FWU_VERIFY_OEMID_MSG;

typedef struct _FWU_VERIFY_OEMID_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status;
} FWU_VERIFY_OEMID_MSG_REPLY;

typedef struct _FWU_START_MSG {
  UINT32 MessageType;
  UINT32 Length;                               ///< Length of update image
  UINT8  UpdateType;                           ///< 0 Full update, 1 parital update
  UINT8  PasswordLength;                       ///< Length of password not include NULL. For BXT, no POR
  UINT8  PasswordData[FWU_PWD_MAX_SIZE];       ///< Password data not include NULL byte. For BXT, no POR
  UINT32 IpuIdTobeUpdated;                     ///< Only for partial FWU
  UINT32 UpdateEnvironment;                    ///< 0 default to normal update, 1 for emergency IFR update
  UPDATE_FLAGS UpdateFlags;
  OEM_UUID OemId;
  UINT32 Resv[4];
} FWU_START_MSG;

typedef struct _FWU_START_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status;
  UINT32 Resv[4];
} FWU_START_MSG_REPLY;

typedef struct _FWU_DATA_MSG {
  UINT32 MesageType;
  UINT32 Length;
  UINT8  Reserved[3];
  UINT8  Data[1];
} FWU_DATA_MSG;

typedef struct _FWU_DATA_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status;
} FWU_DATA_MSG_REPLY;

typedef struct _FWU_END_MSG {
  UINT32 MessageType;
} FWU_END_MSG;

typedef struct _FWU_END_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status; ///< 0 indicate success, else failure
  UINT32 ResetType;
  UINT32 Resv[4];
}FWU_END_MSG_REPLY;

typedef struct _HBM_ENUM_MSG {
  UINT8  Cmd;
  UINT8  Resv[3];
} HBM_ENUM_MSG;

typedef struct _HBM_ENUM_MSG_REPLY {
  UINT8  CmdReply;
  UINT8  Resv[3];
  UINT32  ValidAddresses[8];   ///<Valid addresses. totally 256 bits. Earch bit corresponding to one address.
} HBM_ENUM_MSG_REPLY;

typedef struct _HBM_CLIENT_PROP_MSG {
  UINT8  Cmd;
  UINT8  Address;
  UINT8  Resv[2];
} HBM_CLIENT_PROP_MSG;

typedef struct _HBM_CLIENT_RPOP_MSG_REPLY {
  UINT8 CmdReply;
  UINT8 Address;
  UINT8 Status;
  UINT8 Resv;
  OEM_UUID ProtocolName;
  UINT8 ProtocolVersion;
  UINT8 MaximumConnections;
  UINT8 FixedAddress;
  UINT8 SingleRecvBuffer;
  UINT32 MaxMessageLength;
} HBM_CLIENT_PROP_MSG_REPLY;

typedef struct _HBM_CONNECT_MSG {
  UINT8  Cmd;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Resv;
} HBM_CONNECT_MSG;

typedef struct _HBM_CONNECT_MSG_REPLY {
  UINT8  CmdReply;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Status;
} HBM_CONNECT_MSG_REPLY;

typedef struct _HBM_DISCONNECT_MSG {
  UINT8  Cmd;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Resv;
} HBM_DISCONNECT_MSG;

typedef struct _HBM_DISCONNECT_MSG_REPLY {
  UINT8  CmdReply;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Status;
} HBM_DISCONNECT_MSG_REPLY;

typedef struct _HBM_FLOW_CONTROL_MSG {
  UINT8  Cmd;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Resv[5];
} HBM_FLOW_CONTROL_MSG;

typedef union _MEFWCAPS_SKU {
  UINT32  Data;
  struct {
    UINT32  FullNet : 1;              ///< [0] Full network manageability
    UINT32  StdNet : 1;               ///< [1] Standard network manageability
    UINT32  Manageability : 1;        ///< [2] Manageability
    UINT32  SmallBusiness : 1;        ///< [3] Small business technology
    UINT32  IntegratedTouch : 1;      ///< [4] Intel Integrated Touch
    UINT32  IntelAT : 1;              ///< [5] IntelR Anti-Theft (AT)
    UINT32  IntelCLS : 1;             ///< [6] IntelR Capability Licensing Service (CLS)
    UINT32  Reserved3 : 3;            ///< [9:7] Reserved
    UINT32  IntelMPC : 1;             ///< [10] IntelR Power Sharing Technology (MPC)
    UINT32  IccOverClocking : 1;      ///< [11] ICC Over Clocking
    UINT32  PAVP : 1;                 ///< [12] Protected Audio Video Path (PAVP)
    UINT32  NFF : 1;                  ///< [13] NFF
    UINT32  Reserved4 : 3;            ///< [16:14] Reserved
    UINT32  IPV6 : 1;                 ///< [17] IPV6
    UINT32  KVM : 1;                  ///< [18] KVM Remote Control (KVM)
    UINT32  OCH : 1;                  ///< [19] Outbreak Containment Heuristic (OCH)
    UINT32  VLAN : 1;                 ///< [20] Virtual LAN (VLAN)
    UINT32  TLS : 1;                  ///< [21] TLS
    UINT32  Reserved5 : 1;            ///< [22] Reserved
    UINT32  WLAN : 1;                 ///< [23] Wireless LAN (WLAN)
    UINT32  Reserved6 : 5;            ///< [28:24] Reserved
    UINT32  PTT : 1;                  ///< [29] Platform Trust Technoogy (PTT)
    UINT32  Reserved7 : 1;            ///< [30] Reserved
    UINT32  NFC : 1;                  ///< [31] NFC
  } Fields;
} MEFWCAPS_SKU;

#pragma pack()

/*
 ***********************************************
 * HECI return status definitions:
 ************************************************
 */

#define BIOS_HECI_STATUS_OK               0x0
#define BIOS_HECI_STATUS_INVALID_PARAM    0x1
#define BIOS_HECI_STATUS_FILE_NOT_FOUND   0x2
#define BIOS_HECI_STATUS_AFTER_EOP        0x3
#define BIOS_HECI_STATUS_ERROR            0x4

/*
 ***********************************************
 * HECI2 connection definitions:
 ************************************************
 */

#define HECI2_BIOS_GUID_RR            {0x510f0ecb, 0xd19d, 0x46cd, {0x9c, 0x73, 0xaf, 0x61, 0x80, 0xc6, 0xf5, 0x8d}}
#define HECI2_BIOS_MAX_WRITE_MSG_SIZE 2048  // Max supported HECI2 transaction size
#define HECI2_BIOS_MAX_READ_MSG_SIZE  512   // Size of the HECI HW circular buffer
#define HECI2_BIOS_MCA_FIXED_ADDR     14    // TODO - check this value

/*
 ***********************************************
 * HECI2 global definitions:
 ************************************************
 */

#define MAX_DIR_NAME                      12
#define MAX_FILE_NAME                     25
#define HECI2_MAX_DATA_SIZE_READ_REQ      (HECI2_BIOS_MAX_READ_MSG_SIZE - (sizeof(HECI2_TRUSTED_CHANNEL_BIOS_HEADER) + sizeof(HECI2_READ_DATA_RESP)))
#define HECI2_MAX_DATA_SIZE_WRITE_REQ     (HECI2_BIOS_MAX_WRITE_MSG_SIZE - (sizeof(HECI2_TRUSTED_CHANNEL_BIOS_HEADER) + sizeof(HECI2_WRITE_DATA_REQ)))
#define MAX_HECI2_WRITE_DATA_SIZE         (ALIGN_VALUE (HECI2_MAX_DATA_SIZE_WRITE_REQ, sizeof (UINT32)) - sizeof (UINT32))
#define MAX_HECI2_READ_DATA_SIZE          (ALIGN_VALUE (HECI2_MAX_DATA_SIZE_READ_REQ, sizeof (UINT32)) - sizeof (UINT32))
/*
 ***********************************************
 * HECI2 commands definitions:
 ************************************************
 */

#define HECI2_READ_DATA_CMD_ID       0x1
#define HECI2_WRITE_DATA_CMD_ID      0x2
#define HECI2_FILE_SIZE_CMD_ID       0x3
#define HECI2_LOCK_DIR_CMD_ID        0x4
#define HECI2_GET_PROXY_STATE_CMD_ID 0x5
#define HECI2_GENERAL_ERR_ID         0xF     ///< Response for unknown command-id request

#define HECI2_BIOS_TO_CSE            0x0
#define HECI2_CSE_TO_BIOS            0x1

#pragma pack(push, 1)

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_HEADER {
  UINT8  Signature[HECI2_HMAC_SHA256_SIGNATURE_SIZE];  ///< HMAC-SHA2 of the message
                                                       ///< Note: This signature is not part of the hash
  UINT8  RequestResponse : 1;                          ///< Request = 0, Response = 1
  UINT8  CommandId       : 7;                          ///< Command ID (XXX_CMD_ID)
  UINT32 MonotonicCounter;                             ///< Counter value of the request
                                                       ///< This counter is incremented for each request
} HECI2_TRUSTED_CHANNEL_BIOS_HEADER;

#pragma pack(pop)

/*
 ***********************************************
 * HECI2 Requests
 ************************************************
 */
#pragma pack(push, 1)

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_READ_REQ {
    HECI2_TRUSTED_CHANNEL_BIOS_HEADER  TrustedChannelHeader;
    UINT8                              FileName [MAX_FILE_NAME];  ///< File Name in the BIOS directory
    UINT32                             Offset;                    ///< Offset within the file
    UINT16                             DataSize;                  ///< Number of bytes to read
} HECI2_TRUSTED_CHANNEL_BIOS_READ_REQ;

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_WRITE_REQ {
    HECI2_TRUSTED_CHANNEL_BIOS_HEADER  TrustedChannelHeader;
    UINT8                              FileName [MAX_FILE_NAME];  ///< File Name in the BIOS directory
    UINT32                             Offset;                    ///< Offset within the file
    UINT16                             DataSize;                  ///< Number of bytes to write
    UINT8                              Truncate;                  ///< If set, the file will be truncated
} HECI2_TRUSTED_CHANNEL_BIOS_WRITE_REQ;

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_NVMSIZE_REQ {
    HECI2_TRUSTED_CHANNEL_BIOS_HEADER  TrustedChannelHeader;
    UINT8                              FileName [MAX_FILE_NAME];  ///< File Name in the BIOS directory
} HECI2_TRUSTED_CHANNEL_BIOS_NVMSIZE_REQ;

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_LOCKDIR_REQ {
    HECI2_TRUSTED_CHANNEL_BIOS_HEADER  TrustedChannelHeader;
    UINT8                              FileName [MAX_DIR_NAME];  ///< File Name in the BIOS directory
} HECI2_TRUSTED_CHANNEL_BIOS_LOCKDIR_REQ;

#pragma pack(pop)

typedef struct _HECI2_READ_DATA_REQ
{
  UINT8          FileName [MAX_FILE_NAME];  ///< File Name in the BIOS directory
  UINT32         Offset;                    ///< Offset within the file
  UINT16         DataSize;                  ///< number of bytes to read/write

} HECI2_READ_DATA_REQ;

typedef struct _HECI2_WRITE_DATA_REQ
{
  UINT8          FileName [MAX_FILE_NAME];  /// File Name in the BIOS directory
  UINT32         Offset;                    ///< Offset within the file
  UINT16         DataSize;                  ///< number of bytes to read/write
  UINT8          Ttruncate;                 ///< If set the file will be truncated to length 0 before writing

} HECI2_WRITE_DATA_REQ;


typedef struct _HECI2_FILE_SIZE_REQ
{
  UINT8          FileName [MAX_FILE_NAME];      ///< File Name in the BIOS directory
} HECI2_FILE_SIZE_REQ;

typedef struct _HECI2_LOCK_DIR_REQ
{
  UINT8          DirName [MAX_DIR_NAME];      ///< Dir Name in the BIOS directory
} HECI2_LOCK_DIR_REQ;


/*
 ***********************************************
 * HECI2 Responses
 ************************************************
 */
#pragma pack(push, 1)

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_READ_RESP {
    HECI2_TRUSTED_CHANNEL_BIOS_HEADER  TrustedChannelHeader;
    UINT8                              Status;                ///< Status of the read operation
    UINT16                             DataSize;              ///< Number of bytes read
} HECI2_TRUSTED_CHANNEL_BIOS_READ_RESP;

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_WRITE_RESP {
    HECI2_TRUSTED_CHANNEL_BIOS_HEADER  TrustedChannelHeader;
    UINT8                              Status;                ///< Number of bytes written
} HECI2_TRUSTED_CHANNEL_BIOS_WRITE_RESP;

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_NVMSIZE_RESP {
    HECI2_TRUSTED_CHANNEL_BIOS_HEADER  TrustedChannelHeader;
    UINT8                              Status;                ///< Status of the read operation
    UINT32                             DataSize;              ///< Number of bytes read
} HECI2_TRUSTED_CHANNEL_BIOS_NVMSIZE_RESP;

typedef struct _HECI2_TRUSTED_CHANNEL_BIOS_LOCKDIR_RESP {
    HECI2_TRUSTED_CHANNEL_BIOS_HEADER  TrustedChannelHeader;
    UINT8                              Status;                ///< Number of bytes written
} HECI2_TRUSTED_CHANNEL_BIOS_LOCKDIR_RESP;


#pragma pack(pop)

typedef struct _HECI2_READ_DATA_RESP
{
  UINT8           Status;               ///< Status - see error list defined above
  UINT16          DataSize;             ///< number of bytes read

} HECI2_READ_DATA_RESP;


typedef struct _HECI2_WRITE_DATA_RESP
{
  UINT8           Status;               ///< Status - see error list defined above

} HECI2_WRITE_DATA_RESP;


typedef struct _HECI2_FILE_SIZE_RESP
{
  UINT8           Status;               ///< Status - see error list defined above
  UINT32          FileSize;             ///< File size in bytes

} HECI2_FILE_SIZE_RESP;


typedef struct _HECI2_LOCK_DIR_RESP
{
  UINT8           Status;               ///< Status - see error list defined above

} HECI2_LOCK_DIR_RESP;

/*
 ***********************************************
 * HECI2 BIOS message definition
 ************************************************
 */

typedef struct _HECI2_BIOS_HEADER
{
  UINT8  req_resp       : 1;  ///< request = 0, response = 1  (HECI2_MSG_TYPE_XXX)
  UINT8  cmd_id         : 7;  ///< Command ID (XXX_CMD_ID)

} HECI2_BIOS_HEADER;

typedef struct _HECI2_BIOS_MESSAGE
{
  HECI2_BIOS_HEADER header;
  union
  {
    HECI2_READ_DATA_REQ     readReq;
    HECI2_WRITE_DATA_REQ    writeReq;
    HECI2_FILE_SIZE_REQ     fileSizeReq;
    HECI2_LOCK_DIR_REQ      lockDirReq;
    HECI2_READ_DATA_RESP    readResp;
    HECI2_WRITE_DATA_RESP   writeResp;
    HECI2_FILE_SIZE_RESP    fileSizeResp;
    HECI2_LOCK_DIR_RESP     lockDirResp;
    UINT8                   Status;
  }Body;

} HECI2_BIOS_MESSAGE;

//
//
typedef HECI2_BIOS_MESSAGE HECI2_BIOS_MASSAGE;

typedef struct _WRITE_TO_BIOS_DATA_CMD_REQ_DATA
{
  HECI2_BIOS_HEADER HECIHeader;
  UINT8             FileName [25];
  UINT32            Offset;
  UINT16            Size;
  UINT8             Truncate;
} WRITE_TO_BIOS_DATA_CMD_REQ_DATA;

typedef struct _WRITE_TO_BIOS_DATA_CMD_CMD_RESP_DATA
{
  HECI2_BIOS_HEADER       HECIHeader;
  UINT8                   Status;
} WRITE_TO_BIOS_DATA_CMD_RESP_DATA;

typedef struct _READ_TO_BIOS_DATA_CMD_REQ_DATA
{
  HECI2_BIOS_HEADER  HECIHeader;
  UINT8              FileName [25];
  UINT32             Offset;
  UINT16             Size;
} READ_TO_BIOS_DATA_CMD_REQ_DATA;

typedef struct _READ_TO_BIOS_DATA_CMD_CMD_RESP_DATA
{
  HECI2_BIOS_HEADER       HECIHeader;
  UINT8                   Status;               ///< Status - see error list defined above
  UINT16                  DataSize;             ///< number of bytes read
} READ_TO_BIOS_DATA_CMD_RESP_DATA;

typedef struct _GET_FILE_SIZE_CMD_REQ_DATA
{
  HECI2_BIOS_HEADER  HECIHeader;
  UINT8              FileName [25];
} GET_FILE_SIZE_CMD_REQ_DATA;

typedef struct _GET_FILE_SIZE_CMD_CMD_RESP_DATA
{
  HECI2_BIOS_HEADER       HECIHeader;
  UINT8                   Status;               ///< Status - see error list defined above
  UINT32                  FileSize;             ///< number of bytes read
} GET_FILE_SIZE_CMD_RESP_DATA;

typedef struct _HECI2_GET_PROXY_STATE_REQ
{
  HECI2_BIOS_HEADER        Header;
} HECI2_GET_PROXY_STATE_REQ;

typedef struct _HECI2_GET_PROXY_STATE_RESP
{
  HECI2_BIOS_HEADER        Header;
  UINT8                    Status;
} HECI2_GET_PROXY_STATE_RESP;


#pragma pack()

#define     MAX_MCA_FILE_SIZE           4096 ///< biggeset new value of file after set size (in bytes)
#define     MCA_MAX_FILE_PATH_SIZE      255
#define     MCA_MAX_FILE_NAME           25

/**
********************************************************************************
** Command types:
********************************************************************************
**/
#define     GET_FILE_ATTRIBUTES             0x01
#define     READ_FILE                       0x02
#define     SET_FILE                        0x03
#define     COMMIT_FILES                    0x04
#define     COMMIT_TO_FPF                   0x06
#define     RPMC_BIND                       0x07
#define     GET_RPMC_STATUS                 0x08
#define     CDS_GET_ATTRIBUTES              0x09
#define     CDS_READ_FILE                   0x0A
#define     CDS_WRITE_FILE                  0x0B

//BXT BIOS HECI message
#define   HECI1_READ_DATA                   0x0C
#define   HECI1_WRITE_DATA                  0x0D
#define   HECI1_FILE_SIZE                   0x0E
#define   HECI1_REQUEST_DEVICE_OWNERSHIP    0x0F
#define   HECI1_LOCK_BIOS_DIRECTORY         0x10
#define   HECI1_EOS                         0x11



#define     MCA_COMMAND_MAX                 0x10 ///< any larger type value is illegal

/**
********************************************************************************
** Command flags:
********************************************************************************
**/

#define     NVAR_HASH          0x01
#define     GET_DEFAULT        0x02
#define     TRUNCATE           0x01


/**
********************************************************************************
** Manufacturing Command status:
********************************************************************************
**/

#define     MCA_OK                      0x00
#define     MCA_GLOBAL_RESET_REQUIERED  0x01
#define     MCA_ACCESS_DENIED           0x02
#define     MCA_NO_FILE                 0x03
#define     MCA_BAD_SEEK                0x04
#define     MCA_READ_FAIL               0x05
#define     MCA_CANT_MALLOC             0x06
#define     MCA_NO_FILE_ATTRIBUTE       0x07
#define     MCA_FILE_NO_MANUF           0x08
#define     MCA_MANUF_NOT_ALLOWED       0x09
#define     MCA_AFTER_LOCK              0x0A
#define     MCA_INVALID_INPUT           0x0B
#define     MCA_RESET_FAILED            0x0C
#define     MCA_SIZE_EXCEED_LIMIT       0x0D
#define     MCA_INVALID_ATTR_HASH       0x0E
#define     MCA_FPF_FILE_NOT_WRITTEN    0x0F
#define     MCA_FPF_INTERNAL            0x10
#define     MCA_FPF_CANARY              0x11
#define     MCA_FPF_FATAL               0x12
#define     MCA_NO_QUOTA                0x13
#define     MCA_GENERAL_ERROR           0x14

#define     GROUP_ID_MCA                0xA

#define NFC_BITMASK             0x80000000
#define CLEAR_FEATURE_BIT       0x00000000
#define STR_NFC_DISABLED        L"Disabled"
#define STR_NFC_MODULE_1        L""
#define STR_NFC_MODULE_2        L"NXP PN547"
#define STR_NFC_MODULE_3        L"NXP"

#define NFC_ENABLED             1
#define NFC_DISABLED            0

#define PTT_BITMASK             0x20000000  //BIT29
#define CLEAR_PTT_BIT           0x00000000

typedef enum {
  BOOT_FROM_EMMC  = 0,
  BOOT_FROM_UFS   = 1,
  BOOT_FROM_SPI   = 2
} BOOT_MEDIA;

#pragma pack(1)

typedef struct _GET_FILESIZE_CMD_REQ_DATA
{
  MKHI_MESSAGE_HEADER   MKHIHeader;
  UINT8                 FileName[MCA_MAX_FILE_NAME];
} GET_FILESIZE_CMD_REQ_DATA;

typedef struct _GET_FILESIZE_CMD_RESP_DATA
{
  MKHI_MESSAGE_HEADER   MKHIHeader;
  UINT32                FileSize;
} GET_FILESIZE_CMD_RESP_DATA;

typedef struct _MCA_CDS_WRITE_COMMAND
{
    MKHI_MESSAGE_HEADER   MKHIHeader;
    UINT8                 FileName[MCA_MAX_FILE_NAME];      ///< cds file name
    UINT32                Offset;                           ///< Offset of file
    UINT32                Size;                             ///< Size to read
    UINT8                 Truncate;                         ///< bit 0 - 1 to truncate ile
} MCA_CDS_WRITE_COMMAND;

typedef struct _MCA_CDS_READ_COMMAND
{
  MKHI_MESSAGE_HEADER MKHIHeader;
  UINT8               FileName [12];
  UINT32              Offset;
  UINT32              Size;

} MCA_CDS_READ_COMMAND;

typedef struct _MCA_CDS_READ_COMMAND_RESP_DATA
{
  MKHI_MESSAGE_HEADER       MKHIHeader;
  UINT32                    DataSize;               ///< number of bytes read
} MCA_CDS_READ_COMMAND_RESP_DATA;

typedef struct _GET_OWNERSHIP_CMD_REQ_DATA
{
  MKHI_MESSAGE_HEADER   MKHIHeader;
} GET_OWNERSHIP_CMD_REQ_DATA;

typedef struct _GET_OWNERSHIP_CMD_RESP_DATA
{
  MKHI_MESSAGE_HEADER   MKHIHeader;
} GET_OWNERSHIP_CMD_RESP_DATA;

typedef struct _READ_FROM_RPMB_STORAGE_CMD_REQ_DATA
{
  MKHI_MESSAGE_HEADER   MKHIHeader;
  UINT8             FileName [MCA_MAX_FILE_NAME];
  UINT32            Offset;
  UINT32            Size;
  UINT32            DstAddressLower;
  UINT32            DstAddressUpper;
} READ_FROM_RPMB_STORAGE_CMD_REQ_DATA;

typedef struct _READ_FROM_RPMB_STORAGE_CMD_RESP_DATA
{
  MKHI_MESSAGE_HEADER   MKHIHeader;
  UINT32                DataSize;
} READ_FROM_RPMB_STORAGE_CMD_RESP_DATA;

typedef struct _WRITE_TO_RPMB_STORAGE_CMD_REQ_DATA
{
  MKHI_MESSAGE_HEADER   MKHIHeader;
  UINT8                 FileName [MCA_MAX_FILE_NAME];
  UINT32                Offset;
  UINT32                Size;
  UINT32                Truncate;
  UINT32                SrcAddressLower;
  UINT32                SrcAddressUpper;
} WRITE_TO_RPMB_STORAGE_CMD_REQ_DATA;

typedef struct _WRITE_TO_RPMB_STORAGE_CMD_RESP_DATA
{
  MKHI_MESSAGE_HEADER   MKHIHeader;
} WRITE_TO_RPMB_STORAGE_CMD_RESP_DATA;

#define  EOP_GROUP_ID         0xFF
#define  EOP_CMD_ID           0xC
#define MKHI_GEN_DNX_GROUP_ID 0x0D
#define  CSE_DNX_REQ_SET      0x1
#define  IAFW_DNX_REQ_CLEAR   0x2

//
//EOP-REQ
//End of POST message
//
typedef struct _GEN_END_OF_POST
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
} GEN_END_OF_POST;
typedef struct _GEN_END_OF_SERVICES
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
} GEN_END_OF_SERVICES;
typedef struct _GEN_END_OF_SERVICES_ACK
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
} GEN_END_OF_SERVICES_ACK;

typedef struct _DATA_CLEAR_LOCK_REQ
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
} DATA_CLEAR_LOCK_REQ;
typedef struct _DATA_CLEAR_LOCK_RES
{
  MKHI_MESSAGE_HEADER  Header;
} DATA_CLEAR_LOCK_RES;
typedef struct _MBP_CMD_REQ_DATA
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
  UINT32               Flag;
} MBP_CMD_REQ_DATA;
typedef struct _MBP_CMD_RESP_DATA
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
  UINT8                Length;
  UINT8                ItemsNum;
  UINT8                Flags;
  UINT8                Reserved;
} MBP_CMD_RESP_DATA;

typedef struct _IAFW_DNX_REQ_CLEAR_REQ_DATA
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
  UINT32               Flag;
} IAFW_DNX_REQ_CLEAR_REQ_DATA;

typedef struct _IAFW_DNX_REQ_CLEAR_RESP_DATA
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
} IAFW_DNX_REQ_CLEAR_RESP_DATA;

typedef struct _IAFW_DNX_REQ_SET_REQ_DATA
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
} IAFW_DNX_REQ_SET_REQ_DATA;

typedef struct _IAFW_DNX_REQ_SET_RESP_DATA
{
  MKHI_MESSAGE_HEADER  MKHIHeader;
  UINT32               ReqBiosAction;
} IAFW_DNX_REQ_SET_RESP_DATA;

typedef struct _MBP_ITEM_HEADER
{
  UINT8        AppID;
  UINT8        ItemID;
  UINT8        Length;
  UINT8        Flag;
} MBP_ITEM_HEADER;

typedef struct _MBP_CURRENT_BOOT_MEDIA
{
  UINT32      BPDTOffset;
  UINT32      PhysicalData;
  UINT32      PhysicalDeviceArea;
  UINT32      LogicalData;
} MBP_CURRENT_BOOT_MEDIA;

typedef struct _MBP_IFWI_DNX_REQUEST
{
  UINT32      MbpItemHeader;
  UINT32      EnterRecovery;
} MBP_IFWI_DNX_REQUEST;

typedef struct _MBP_SMM_TRUSTED_KEY
{
  UINT8       SmmTrustedKey[32];
  UINT32      MonotonicCounter;
} MBP_SMM_TRUSTED_KEY;

typedef struct _MBP_NFC_DEVICE_TYPE
{
  UINT32      MbpItemHeader;
  UINT32      NfcDeviceData;
} MBP_NFC_DEVICE_TYPE;

typedef struct _MBP_ME_FW_CAPS
{
  UINT32          MbpItemHeader;
  MEFWCAPS_SKU    CurrentFeatures;
} MBP_ME_FW_CAPS;

#define MKHI_GEN_GROUP_ID          0xFF
#define FW_VER_CMD_REQ_ID          0x1C
#define FW_VERSION_CMD_RESP        0x9A
#define MAX_IMAGE_MODULE_NUMBER    0x06

typedef struct _FW_VER_CMD_REQ {
  MKHI_MESSAGE_HEADER      MKHIHeader;
} FW_VER_CMD_REQ;

typedef struct _MODULE_VERSION_DATA {
  UINT8       EntryName[12];
  UINT16      Major;
  UINT16      Minor;
  UINT16      Hotfix;
  UINT16      Build;
} MODULE_VERSION_DATA;

typedef struct _FW_VERSION_CMD_RESP_DATA {
  MKHI_MESSAGE_HEADER      MKHIHeader;
  UINT32                   NumModules;
  MODULE_VERSION_DATA      ModuleEntries[MAX_IMAGE_MODULE_NUMBER];
} FW_VERSION_CMD_RESP_DATA;

//
// IFWI Prepare for Update
//
typedef struct _IFWI_PREPARE_FOR_UPDATE {
  MKHI_MESSAGE_HEADER      MKHIHeader;
  UINT8                    ResetType;
} IFWI_PREPARE_FOR_UPDATE;

typedef struct _IFWI_PREPARE_FOR_UPDATE_ACK {
  MKHI_MESSAGE_HEADER      MKHIHeader;
  UINT8                    Flags;
} IFWI_PREPARE_FOR_UPDATE_ACK;

//
// Data Clear
//
typedef struct _IFWI_UPDATE_DATA_CLEAR {
  MKHI_MESSAGE_HEADER      MKHIHeader;
} IFWI_UPDATE_DATA_CLEAR;

typedef struct _IFWI_UPDATE_DATA_CLEAR_ACK {
  MKHI_MESSAGE_HEADER     MKHIHeader;
} IFWI_UPDATE_DATA_CLEAR_ACK;

//
// Get ARB Status
//
typedef struct _GET_ARB_STATUS {
  MKHI_MESSAGE_HEADER      MKHIHeader;
} GET_ARB_STATUS;

typedef struct _GET_ARB_STATUS_ACK {
  MKHI_MESSAGE_HEADER     MKHIHeader;
  UINT8                   DirtySvns[16];
  UINT32                  CurSvns[128];
  UINT32                  NewSvns[128];
  UINT8                   EnabledSvns[16];
  UINT8                   DynamicSvns[16];
} GET_ARB_STATUS_ACK;

//
// Commit ARB SVN Updates
//

typedef struct _COMMIT_ARB_SVN_UPDATES {
  MKHI_MESSAGE_HEADER      MKHIHeader;
  UINT8                   CommitSvns[16];
} COMMIT_ARB_SVN_UPDATES;

typedef struct _COMMIT_ARB_SVN_UPDATES_ACK {
  MKHI_MESSAGE_HEADER     MKHIHeader;
} COMMIT_ARB_SVN_UPDATES_ACK;

//
// Update Image Check
//
typedef struct _IFWI_UPDATE_IMAGE_CHECK {
  MKHI_MESSAGE_HEADER      MKHIHeader;
  UINT32                   ImageBaseAddrLower32b;
  UINT32                   ImageBaseAddrUpper32b;
  UINT32                   ImageSize;
} IFWI_UPDATE_IMAGE_CHECK;

typedef struct _IFWI_UPDATE_IMAGE_CHECK_ACK {
  MKHI_MESSAGE_HEADER      MKHIHeader;
  UINT32                   ImageErrorValue;
  UINT32                   ArbSvnDowngradeBitMap[4];   // bit127 - bit0 (16 bytes)
  UINT8                    RollBackPossible;
} IFWI_UPDATE_IMAGE_CHECK_ACK;

#pragma pack()

/**
  Get the HECI2 read buffer size.

  @param       None

  @retval      UINTN   The HECI2 read buffer size in bytes.
**/
UINTN
EFIAPI
HeciGetHeci2ReadBufferSize (
  VOID
  );

/**
  Get the HECI2 write buffer size.

  @param       None

  @retval      UINTN   The HECI2 write buffer size in bytes.
**/
UINTN
EFIAPI
HeciGetHeci2WriteBufferSize (
  VOID
  );

/**
  Check Firmware Update Progress Status throught HECI

  @param[in,out] InProgress       Firmware update progress status

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciCheckFwuInProgress(
  IN OUT UINT8 *InProgress
)
;

/**
  Send OEM ID message to Sec for verifying through HECI.

  @param[in] OemId                OEM ID for verifying
  @param[in] SecAddress           Sec address

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciVerifyOemId(
  IN  OEM_UUID *OemId,
  IN  UINT8  SecAddress
);

/**
  Waits for last firmware update to be finished. Blocking call that might at most stall the system for 90 seconds.

  @param[in]  None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciFwuWaitOnLastUpdate(
)
;

/**
  Setup a dynamic connection for FWU.

  @param[in,out]  SecAddress      Returns the SecAddress to be used by other FWU APIs
  @param[out]  MaxBufferSize      Specifies the maximum buffer size the FWU link allows.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciConnectFwuInterface(
  IN OUT UINT8 *SecAddress,
  OUT UINT32 *MaxBufferSize
)
;

/**
  Tear down a dynamic connection for FWU.

  @param[in]  SecAddress          Returns the SecAddress to be used by other FWU APIs
  @param[in]  MaxBufferSize       Specifies the maximum buffer size the FWU link allows.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciDisconnectFwuInterface(
  IN UINT8 SecAddress,
  IN UINT32 MaxBufferSize
)
;

/**
  Called by each FWU request API when both sec and host needs to inform each other  it's ready to accept any input.

  @param[in]  SecAddress          Returns the SecAddress to be used by other FWU APIs

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciBiDirectionFlowControl(
  IN UINT8 SecAddress
)
;

/**
  Called by each FWU request API when host needs to inform SEC it's ready to accept any input.

  @param[in]  SecAddress          The dynamic sec address the flow control

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHostToSecFlowControl(
  IN UINT8 SecAddress
)
;

/**
  Called by each FWU request API when there's a flow control message expected on the link.

  @param[in]  None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSecToHostFlowControl()
;

/**
  Get firmware version from FWU interface instead of MKHI.

  @param[out]                     Version - Returns the version number of current running SEC FW.
  @param[in]                      SecAddress - Dynamic sec address for FWU connection

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuGetVersionMsg(
  OUT VERSION*  Version,
  IN UINT8 SecAddress
)
;

/**
  Send Local Firmware Update start message to firmware thru HECI. THe HECI link might be disconnectd passively
    by the SEC side due to unexpected error. Supports full update only.

  @param[in]  ImageLength          Firmware image length to be updated
  @param[in]  OemId                The OemId to be passed to firmware for verification
  @param[in]  SecAddress           Dynamic sec address for FWU connection
  @param[in]  MaxBufferSizeMaximum buffer size for replied message

  @exception EFI_UNSUPPORTED       Current Sec mode doesn't support this function
  @retval EFI_SUCCESS              Command succeeded
  @retval EFI_DEVICE_ERROR         HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT              HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuStartMsg(
  IN UINT32 ImageLength,
  IN OEM_UUID *OemId,
  IN UINT8 SecAddress,
  IN UINT32 MaxBufferSize
)
;

/**
  Send Local Firmware Update Data to firmware thru HECI.

  @param[in]  FwuData             Firmware update data to be updated
  @param[in]  FwuDataSize         Firmware update data size to be passed to firmware
  @param[in]  SecAddress          Dynamic Sec Address used by FWU client
  @param[in]  MaxBufferSize       The max buffer length for FWU client allowed by SEC. Retrieved from the connect API.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuDataMsg(
  IN UINT8*  FwuData,
  IN UINT32  FwuDataSize,
  IN UINT8  SecAddress,
  IN UINT32 MaxBufferSize
)
;

/**
  Send Local Firmware Update Data to firmware thru HECI

  @param[out]  ResetType          Reset type required for finishing firmware update.
  @param[in]  SecAddress          Dynamic Sec Address used by FWU client

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuEndMsg(
  OUT UINT32 *ResetType,
  IN  UINT8  SecAddress
)
;

/**
  Send Local Firmware Update Data to firmware thru HECI

  @param[out]  OemId              The OEM ID returned by firmware.
  @param[in]  SecAddress          Dynamic sec address for FWU.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuGetOemIdMsg(
 OUT  OEM_UUID  *OemId,
 IN   UINT8 SecAddress
)
;

/**
  Query firmware update progress thru HECI

  @param[in,out]  Percentage        Update percentage
  @param[in,out]  CurrentStage      current stage of the update
  @param[in,out]  TotalStages       Total stages of the update
  @param[in,out]  LastUpdateStatus  The latest error code for the update
  @param[in,out]  LastResetType     Last reset type required for the update
  @param[in,out]  InProgress        Indicates if the update is in progress.

  @exception EFI_UNSUPPORTED        Current Sec mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciFwuQueryUpdateStatus(
  IN OUT UINT32 *Percentage,
  IN OUT UINT32 *CurrentStage,
  IN OUT UINT32 *TotalStages,
  IN OUT UINT32 *LastUpdateStatus,
  IN OUT UINT32 *LastResetType,
  IN OUT UINT8  *InProgress
)
;

/**
  Send Hardware Asset Tables to firmware through HECI.

  @param[in]  AssetTableData      Hardware Asset Table Data
  @param[in]  TablesDataSize      Size of Asset table

  @retval EFI_SUCCESS             Table sent
  @retval EFI_ABORTED             Could not allocate Memory
**/
EFI_STATUS
HeciAssetUpdateFwMsg (
  IN  TABLE_PUSH_DATA *AssetTableData,
  IN  UINT16          TableDataSize
  )
;

/**
  Send Core BIOS Reset Request Message through HECI.

  @param[in] ResetOrigin          Reset source
  @param[in] ResetType            Global or Host reset

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendCbmResetRequest (
  IN  UINT8                             ResetOrigin,
  IN  UINT8                             ResetType
  )
;

/**
  Send Reset Request Message through HECI. Used in PEI, included in the HeciMsgLib.

  @param[in] ResetOrigin          Reset source
  @param[in] ResetType            Global or Host reset

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendResetRequest (
  IN  UINT8                             ResetOrigin,
  IN  UINT8                             ResetType
  )
;

/**
  Send End of Post Request Message through HECI.

  @param[in]  None

  @retval  EFI_STATUS
**/
EFI_STATUS
HeciSendEndOfPostMessage (
  IN  VOID
  )
;

/**
  Send Get Firmware SKU Request to Sec

  @param[in,out] MsgGenGetFwCapsSku     Return message for Get Firmware Capability SKU
  @param[in,out] MsgGenGetFwCapsSkuAck  Return message for Get Firmware Capability SKU ACK

  @exception EFI_UNSUPPORTED        Current Sec mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetFwCapsSkuMsg (
  IN OUT GEN_GET_FW_CAPSKU       *MsgGenGetFwCapsSku,
  IN OUT GEN_GET_FW_CAPS_SKU_ACK *MsgGenGetFwCapsSkuAck
  )
;

/**
  Send Get Firmware Version Request to Sec

  @param[in,out] MsgGenGetFwVersionAck   Return themessage of FW version

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetFwVersionMsg (
  IN OUT GEN_GET_FW_VER_ACK     *MsgGenGetFwVersionAck
  )
;

/**
  Send Thermal Reporting message through HECI.

  @param[in]  Heci                The pointer of Heci protocol
  @param[in]  TrConfig            Thermal Reporting Configuration Setting

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciTrConfigMsg (
  IN  EFI_HECI_PROTOCOL  *Heci,
  IN  TR_CONFIG          *TrConfig
  )
;

/**
  Sends a message to Sec to unlock a specified SPI Flash region for writing and receiving a response message.
  It is recommended that HMRFPO_ENABLE HECI message needs to be sent after all OROMs finish their initialization.

  @param[in] Nonce                Nonce received in previous HMRFPO_ENABLE Response Message
  @param[out] Result              HMRFPO_ENABLE response

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHmrfpoEnable (
  IN  UINT64                          Nonce,
  OUT UINT8                           *Result
  )
;

/**
  Sends a message to Sec to lock a specified SPI Flash region for writing and receiving a response message.

  @param[out] Nonce               Random number generated by Ignition Sec FW. When BIOS
                                  want to unlock region it should use this value
                                  in HMRFPO_ENABLE Request Message
  @param[out] FactoryDefaultBase  The base of the factory default calculated from the start of the ME region.
                                  BIOS sets a Protected Range (PR) register "Protected Range Base" field with this value
                                  + the base address of the region.
  @param[out] FactoryDefaultLimit The length of the factory image.
                                  BIOS sets a Protected Range (PR) register "Protected Range Limit" field with this value
  @param[out] Result              Status report

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHmrfpoLock (
  OUT UINT64                          *Nonce,
  OUT UINT32                          *FactoryDefaultBase,
  OUT UINT32                          *FactoryDefaultLimit,
  OUT UINT8                           *Result
  )
;

/**
  System BIOS sends this message to get status for HMRFPO_LOCK message.

  @param[out] Result              HMRFPO_GET_STATUS response

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHmrfpoGetStatus (
  OUT UINT8                           *Result
  )
;

/**
  System BIOS sends this message to get status for HMRFPO_LOCK message.

  @param[in]  QueryType           QueryType:
                                    0 - Query Request
                                    1 - Cancel Request
  @param[out] Result              HMRFPO_GET_STATUS response
                                    1h - Continue, KVM session established.
                                    2h - Continue, KVM session cancelled.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciQueryKvmRequest (
  IN  UINT32                         QueryType,
  OUT UINT32                         *ResponseCode
  )
;

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to query the local firmware update interface status.

  @param[out] RuleData            1 - local firmware update interface enable
                                  0 - local firmware update interface disable

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetLocalFwUpdate (
  OUT UINT32         *RuleData
  )
;

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to enable or disable the local firmware update interface.
  The firmware allows a single update once it receives the enable command

  @param[in] RuleData             1 - local firmware update interface enable
                                  0 - local firmware update interface disable

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciSetLocalFwUpdate (
  IN UINT8         RuleData
  )
;

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to enable the Sec State. The firmware allows a single
  update once it receives the enable command. Once firmware receives this message,
  the firmware will be in normal mode after a global reset.

  @param[in] None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             ME enabled message sent
**/
EFI_STATUS
HeciSetSeCEnableMsg (
  IN VOID
  )
;

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to disable the Sec State. The firmware allows a single
  update once it receives the disable command Once firmware receives this message,
  the firmware will work in "Soft Temporary Disable" mode (HFS[19:16] = 3) after a
  global reset. Note, this message is not allowed when AT firmware is enrolled/configured.

  @param[in] RuleData             Rule data
                                    0 - Disable
                                    1 - Enable

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Sec is disabled
**/
EFI_STATUS
HeciSetSeCDisableMsg (
  IN UINT8 ruleData
  )
;

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to get platform type.
  One of usages is to utilize this command to determine if the platform runs in
  Consumer or Corporate SKU size firmware.

  @param[out] RuleData            PlatformBrand,
                                  IntelMeFwImageType,
                                  SuperSku,
                                  PlatformTargetUsageType

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetPlatformTypeMsg (
  OUT PLATFORM_TYPE_RULE_DATA   *RuleData
  )
;

/**
  This message is sent by the BIOS on the boot where the host wants to get the firmware provisioning state.
  The firmware will respond to AMT BIOS SYNCH INFO message even after the End of Post.

  @param[out] RuleData            Bit [2:0] Reserved
                                  Bit [4:3] Provisioning State
                                    00 - Pre -provisioning
                                    01 - In -provisioning
                                    02 - Post !Vprovisioning
                                  Bit [31:5] Reserved

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Firmware provisioning state returned
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciAmtBiosSynchInfo (
  OUT UINT32         *RuleData
  )
;

/**
  The firmware will respond to GET OEM TAG message even after the End of Post (EOP).

  @param[out] RuleData            Default is zero. Tool can create the OEM specific OEM TAG data.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetOemTagMsg (
  OUT UINT32         *RuleData
  )
;

/**
  Enables/disables clocks. Used to turn off clocks in unused pci/pcie slots.

  @param[in]  Enables           Each bit means corresponding clock should be turned on (1) or off (0)
  @param[in]  EnablesMask       Each bit means corresponding enable bit is valid (1) or should be ignored (0)
  @param[in]  Parameters        Retain clock enables at resume for S3', other bits = reserved, must be 0
  @param[in]  Nonce             Secret number used to validate caller

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
EFIAPI
HeciSetIccClockEnables (
  IN UINT32 Enables,
  IN UINT32 EnablesMask,
  IN UINT32 Parameters,
  IN UINT64 Nonce
  )
;

/**
  Sets or reads Lock mask on ICC registers.

  @param[in]      AccessMode      0 - set, 1 - get
  @param[in,out]  Mask            Mask of registers to become (for 'set' mode) or are (for 'get' mode) locked. Each bit represents a register. 0=lock, 1=don't lock
  @param[in,out]  Nonce           This secret number must be used for every attempt to lock registers except first

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciLockIccRegisters (
  IN UINT8       AccessMode,
  IN OUT UINT32  *Mask,
  IN OUT UINT64  *Nonce
  )
;

/**
  Retrieves the number of currently used ICC clock profile

  @param[out]  Profile            number of current ICC clock profile

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetIccProfile (
  OUT UINT8 *Profile
  )
;

/**
  Sets ICC clock profile to be used on next and following boots

  @param[in]  Profile             Number of profile to be used

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciSetIccProfile (
  IN UINT8 Profile
  )
;

/**
  This message is used to enable the IntelR SEC Debug Event Service capability.
  Once firmware receives this message, the firmware will enable MDES capability.
  The firmware will automatically disable MDES capability after receiving End Of Post.

  @param[out]  Data               0x00   : Enable Success
                                  Others : Enable Failure

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciMdesCapabilityEnableMsg (
  OUT UINT8        *Data
  )
;

/**
  Get AT State Information From FW

  @param[in,out]  AtStateInfo     Pointer to structure to hold AT FW Data

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciGetAtFwStateInfoMsg (
  IN OUT AT_STATE_STRUCT       *AtStateInfo
  )
;

/**
  Sends the MKHI Enable/Disable manageability message

  @param[in]  EnableState - Enable Bit Mask
  @param[in]  DisableState - Disable Bit Mask

  @retval  EFI_STATUS
**/
EFI_STATUS
HeciFwFeatureStateOverride (
  IN UINT32  EnableState,
  IN UINT32  DisableState
  )
;

/**
  The Get FW Feature Status message is based on MKHI interface.
  This command is used by BIOS/IntelR MEBX to get firmware runtime status.
  The GET FW RUNTIME STATUS message doesn't need to check the HFS.
  FWInitComplete value before sending the command.
  It means this message can be sent regardless of HFS.FWInitComplete.

  @param[out] RuleData            MEFWCAPS_SKU message

  @exception EFI_UNSUPPORTED      Current ME mode doesn't support this function
  @retval EFI_SUCCESS             HECI interfaces disabled by Sec
**/
EFI_STATUS
HeciGetFwFeatureStateMsg (
  OUT SECFWCAPS_SKU                *RuleData
  )
;

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to disable the SEC State.
  The firmware allows a single update once it receives the disable command

  @param[out]  CmdStatus      Return command status

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSeCUnconfigurationMsg (
  OUT UINT32    *CmdStatus
  )
;

/**
  This message is sent by the BIOS prior to the End of Post (EOP) on the boot
   where host wants to Unconfigure the SEC State

  @param[in]  CmdStatus         // return the data
                                 0 = SEC_UNCONFIG_SUCCESS
                                 1 = SEC_UNCONFIG_IN_PROGRESS
                                 2 = SEC_UNCONFIG_NOT_IN_PROGRESS
                                 3 = SEC_UNCONFIG_ERROR

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSeCUnconfigurationStatusMsg (
  IN UINT32    *CmdStatus
  )
;

/**
  HeciSendDIDMessage

  @param[in]  UMABase
  @param[in]  UMASize
  @param[out] BiosAction

  @retval  EFI_STATUS
**/
EFI_STATUS
HeciSendDIDMessage (
  UINT32 UMABase,
  UINT8 IsS3,
  UINT32 *UMASize,
  UINT8  *BiosAction
  );

/**
  Send Get IFWI Version Request to SEC

  @param[in,out]  MsgGenGetIFWIVersionAck   Return the message of IFWI version

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetIFWIVersionMsg (
  IN OUT GEN_GET_IFWI_VER_ACK     *MsgGenGetIFWIVersionAck
  )
;

/**
  Retrieves the current RPMB config file

  @param[out]  MsgGenGetRPMBConfigFileAck   Return the current RPMB config file

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetRPMBconfigfile (
  OUT GEN_GET_RPMB_CONFIG_FILE_ACK     *MsgGenGetRPMBConfigFileAck
  )
;

/**
  Set the current RPMB config file

  @param[in,out]  MsgGenSetRPMBConfigFileAck   Set the current RPMB config file

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetRPMBconfigfile (
  IN OUT GEN_SET_RPMB_CONFIG_FILE_ACK     *MsgGenSetRPMBConfigFileAck
  )
;

/**
  Read the Boot Partition

  @param[in,out]  MsgGenBootPartitionReadAck   Read the current Boot Partition

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciBootPartitionRead (
  IN OUT GEN_BOOT_PARTITION_READ_ACK     *MsgGenBootPartitionReadAck
  )
;

/**
  Read the Boot Partition

  @param[in,out]  MsgGenBootPartitionReadAck   Read the current Boot Partition

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciMassStorageRead (
  IN OUT GEN_MASS_STORAGE_READ_ACK     *MsgGenMassStorageReadAck
  )
;

/**
  Request Device Ownership

  @param[in,out]  MsgGenRequestDeviceOwnershipAck   Request Device Ownership

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciRequestDeviceOwnership(
  IN OUT GEN_REQUEST_DEVICE_OWNERSHIP_ACK     *MsgGenRequestDeviceOwnershipAck
  )
;

/**
  Grant Device Ownership

  @param[in,out]  MsgGenGrantDeviceOwnershipAck   Grant Device Ownership

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGrantDeviceOwnership(
  IN OUT GEN_GRANT_DEVICE_OWNERSHIP_ACK     *MsgGenGrantDeviceOwnershipAck
  )
;

/**
  SMIP Read

  @param[in,out]  MsgGenSMIPReadAck  SMIP Read

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSMIPRead(
  IN OUT GEN_SMIP_READ_ACK     *MsgGenSMIPReadAck
  )
;

/**
  Auth Policy Manifest

  @param[in,out]  MsgGenAuthPolicyManifestAck   Auth Policy Manifest

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciAuthPolicyManifest(
  IN OUT GEN_AUTH_POLICY_MANIFEST_ACK     *MsgGenAuthPolicyManifestAck
  )
;

/**
  Get Boot Type data.

  @param[in,out]  MsgGenBootTypeAck   Boot Type

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciBootType(
  IN OUT GEN_BOOT_TYPE_ACK     *MsgGenBootTypeAck
  )
;

/**
  Send AUTH Kernel Request to SEC

  @param[in,out]  MsgGenAuthKernelAck   Auth Kernel

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciAuthKernel(
  IN OUT GEN_AUTH_KERNEL_ACK     *MsgGenAuthKernelAck
  )
;

/**
  RSA Offload

  @param[in,out]  MsgGenRSAOffloadAck    RSA Offload

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciRSAOffload(
  IN OUT GEN_RSA_OFFLOAD_ACK     *MsgGenRSAOffloadAck
  )
;

/**
  Get MBP Request from SEC

  @param[in,out]  MsgGenGetMBPAck    GET MBP

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetMBP(
  IN OUT GEN_GET_MBP_ACK     *MsgGenGetMBPAck
  )
;

/**
  Load OBB

  @param[in,out]  MsgGenLoadOBBAck    Load OBB

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciLoadOBB(
  IN OUT GEN_LOAD_OBB_ACK     *MsgGenLoadOBBAck
  );

/**
  BIOS requests the size of a specific file.

  @param[in]  FileName            File Name in the BIOS directory
  @param[OUT] FileSize            Number of bytes to read

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_INVALID_PARAMETER   Invalid parameters
  @retval EFI_NOT_FOUND           File not found
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
**/
EFI_STATUS
HeciGetNVMFileSize (
  IN  CONST  CHAR8    *FileName,
  OUT        UINTN    *FileSize
  );

/**
  Get NVM file's size through HECI2.

  @param[in]  FileName       The file name.
  @param[out] FileSize       The file's size.
  @param[in]  Heci2Protocol  A pointer to a HECI2 protocol instance.

  @return     EFI_SUCCESS    Get NVM file size success.
  @return     Others         Get NVM file size failed.
**/
EFI_STATUS
Heci2GetNVMFileSize (
  IN  CONST  UINT8              *FileName,
  OUT        UINTN              *FileSize,
  IN  CONST  EFI_HECI_PROTOCOL  *Heci2Protocol
  );

/**
  BIOS requests to write data to directory.

  @param[in] FileName               File Name in the BIOS directory
  @param[in] Offset                 Offset in bytes, within a file
  @param[in] Data                   Data to be written
  @param[in] DataSize               Number of bytes to write
  @param[in] Truncate               Truncate the file.

  @retval    EFI_SUCCESS            Command succeeded
  @retval    EFI_INVALID_PARAMETER  Invalid parameters
  @retval    EFI_NOT_FOUND          File not found
  @retval    EFI_DEVICE_ERROR       HECI Device error, command aborts abnormally
**/
EFI_STATUS
HeciWriteNVMFile (
  IN CONST CHAR8    *FileName,
  IN       UINT32   Offset,
  IN CONST UINT8    *Data,
  IN       UINTN    DataSize,
  IN       BOOLEAN  Truncate
  );

/**
  BIOS requests variable from its directory.

  @param[in]      FileName               File Name in the BIOS directory
  @param[in]      Offset                 Offset in bytes, within a file
  @param[out]     Data                   Data to be read
  @param[in, out] DataSize               Number of bytes to read

  @retval         EFI_SUCCESS            Command succeeded
  @retval         EFI_INVALID_PARAMETER  Invalid parameters
  @retval         EFI_NOT_FOUND          File not found
  @retval         EFI_DEVICE_ERROR       HECI Device error, command aborts abnormally
**/
EFI_STATUS
HeciReadNVMFile (
  IN      CONST  UINT8     *FileName,
  IN             UINT32    Offset,
  OUT            UINT8     *Data,
  IN OUT         UINTN     *DataSize
  );

/**
  Lock Directory message through HECI2.

  @param[in] DirName   The Directory name.
  @param[in] Heci2Protocol The HECI protocol to send the message to HECI2 device.

  @return EFI_SUCCESS   Send EOP message success.
  @return Others              Send EOP message failed.
**/
EFI_STATUS
Heci2LockDirectory(
  IN UINT8               *DirName,
  IN EFI_HECI_PROTOCOL   *Heci2Protocol
  );

/**
  Get proxy state through HECI.

  @param[in] Heci2Protocol        HECI2 protocol

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
Heci2GetProxyStateNoResp(
  IN EFI_HECI_PROTOCOL         *Heci2Protocol
);

/**
  Returns whether trusted channel is enabled.

  @param[in]  None.

  @retval     TRUE if trusted channel is enabled.
              FALSE if trusted channel is disabled.
**/
BOOLEAN
IsTrustedChannelEnabled (
  VOID
);

/**
  Updates the SHA256 signature and monotonic counter fields of a HECI message header.

  @param[in]  MessageHeader   A pointer to the message header
  @param[in]  TotalHeaderSize The total header size
  @param[in]  TotalDataSize   The total data size

  @retval     Whether the header could be updated
**/
EFI_STATUS
EFIAPI
UpdateTrustedHeader (
  IN OUT  UINT8       *MessageHeader,
  IN      UINT32      TotalHeaderSize,
  IN      UINT32      TotalDataSize
  );

/**
  Write data to NVM file through HECI2.

  @param[in] FileName    The file name.
  @param[in] Offset      The offset of data.
  @param[in] Data        The data content.
  @param[in] DataSize    Data's size.
  @param[in] Truncate    Truncate the file.

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_INVALID_PARAMETER   Invalid parameters
  @retval EFI_NOT_FOUND           File not found
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
.
**/
EFI_STATUS
Heci2WriteNVMFile (
  IN CONST CHAR8    *FileName,
  IN       UINT32   Offset,
  IN CONST UINT8    *Data,
  IN       UINTN    DataSize,
  IN       BOOLEAN  Truncate
  );

/**
  BIOS requests to write data to directory without response.

  @param[in] FileName             File Name in the BIOS directory
  @param[in] Offset               Offset in bytes, within a file
  @param[in] Data                 Data to be written
  @param[in] DataSize             Number of bytes to write
  @param[in] Heci2Protocol        HECI2 protocol

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_INVALID_PARAMETER   Invalid parameters
  @retval EFI_NOT_FOUND           File not found
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
**/
EFI_STATUS
Heci2WriteNVMFileNoResp(
  IN UINT8                *FileName,
  IN UINT32                Offset,
  IN UINT8                *Data,
  IN UINTN                 DataSize,
  IN EFI_HECI_PROTOCOL    *Heci2Protocol
);

/**
  Read NVM file data through HECI2.

  @param[in]     FileName       The file name.
  @param[in]     Offset         The offset of data.
  @param[out]    Data           The data buffer.
  @param[in,out] DataSize       Data's size.
  @param[in]     Heci2Protocol  Pointer to a HECI2 protocol instance.

  @return        EFI_SUCCESS    Read NVM file success.
  @return        Others         Read NVM file failed.
**/
EFI_STATUS
Heci2ReadNVMFile (
  IN     CONST  UINT8               *FileName,
  IN            UINT32              Offset,
  OUT           UINT8               *Data,
  IN OUT        UINTN               *DataSize,
  IN     CONST  EFI_HECI_PROTOCOL   *Heci2Protocol
  );

EFI_STATUS
Heci2ReadNVMFileNoResp(
IN UINT8              *FileName,
IN UINT32              Offset,
IN UINT8              *Data,
IN UINTN              *DataSize,
IN EFI_HECI_PROTOCOL  *Heci2Protocol
);

/**
  Send End of Post Request Message through HECI.

  @param[in] None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciEndOfPost(
  );

/**
  BIOS send End of Services Message through HECI.

  @param[in] None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciEndOfServices(
  );

/**
  The CSE returns the version number for each module.

  @param[out] MsgGetFwVersionRespData  Return Firmware version response data

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetImageFwVerMsg (
  OUT FW_VERSION_CMD_RESP_DATA    *MsgGetFwVersionRespData
  );

/**
  Send IAFW DNX request set message throught HECI1.

  @param[in] Resp       Buffer to receive the CSE response data.

  @return EFI_SUCCESS   Set request success.
  @return Others        Set request failed.
**/
EFI_STATUS
HeciIafwDnxReqSet(
  IN OUT IAFW_DNX_REQ_SET_RESP_DATA                *Resp
  );

/**
  Send IAFW DnX require clear command to CSE.

  @param[in] Flag                 Require flag

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciIafwDnxReqClear(
  IN UINT32              Flag
  );

/**
  The Get FW Feature Status message is based on MKHI interface.
  This command is used by BIOS/IntelR MEBX to get firmware runtime status.
  The GET FW RUNTIME STATUS message doesn't need to check the HFS.
  FWInitComplete value before sending the command.
  It means this message can be sent regardless of HFS.FWInitComplete.

  @param[out] RuleData            MEFWCAPS_SKU message

  @exception EFI_UNSUPPORTED      Current ME mode doesn't support this function
**/
EFI_STATUS
HeciGetFwFeatureStateMsgII (
  OUT MEFWCAPS_SKU                *RuleData
  );

/**
  Get NFC device type from CSE.

  @param[in] Mbp_Nfc_Device_Type  NFC Device type data

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetNfcDeviceType (
  IN MBP_NFC_DEVICE_TYPE    *Mbp_Nfc_Device_Type
  );

/**
Get ME FW Capability from MBP.

@param[out] MBP_ME_FW_CAPS  Output data buffer for ME FW Capability.

@return EFI_SUCCESS         Get ME FW Capability success.
@return Others              Get ME FW Capability failed.
**/

EFI_STATUS
HeciGetMeFwCapability(
  OUT MBP_ME_FW_CAPS    *Mbp_Me_Fw_Caps
);

/**
  Get IFWI DnX request data.

  @param[in] IfwiDnxRequestData   IFWI DnX request data

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetIfwiDnxRequest (
  IN MBP_IFWI_DNX_REQUEST    *IfwiDnxRequestData
  );

/**
Send IFWI PREPARE FOR UPDATE(IPFU) Command through HECI1.
This command provide necessary synchronization between HOST & CSE when
BIOS Performance IFWI Update process.

@return EFI_SUCCESS         Send IFWI Prepare For Update command succeeded
@return Others              Send IFWI Prepare For Update command failed.
**/
EFI_STATUS
HeciIfwiPrepareForUpdate(
 VOID
);

/**
  Send RPMB/Device Extention region Data Clear message through HECI1.

  @return EFI_SUCCESS         Send DataClear message success.
  @return Others              Send DataClear message failed.
**/
EFI_STATUS
HeciDataClear (
  VOID
  );

/**
Send 'Update Image Check' command to CSE to verify IFWI Image and confirm
that appropriate for FW Update. (This command will be sending as part of Capsule Update)

@param[in] ImageBaseAddr  FW Image Base address. 64 Bit Wide
@param[in] ImageSize      FW Image Size
@param[in] *HeciResponse  To capture Response Info (Optional)
@param[in] ResponseSize   Size of Response (Optional)

@return EFI_SUCCESS       Image was checked (best effort) and verified to be appropriate for FW update.
@return 0x01               IMAGE_FAILED.
@return 0x02               IMG_SIZE_INVALID.
@return 0x05               SIZE_ERROR
@return 0x89               STATUS_NOT_SUPPORTED.
@return 0x8D               STATUS_INVALID_COMMAND.
@return 0xFF               STATUS_UNDEFINED.
**/
EFI_STATUS
HeciUpdateImageCheck(
IN UINT64        ImageBaseAddr,
IN UINT32        ImageSize,
IN OUT UINT32    *HeciResponse,
IN UINT32        ResponseSize
);

/**
  Checks Platform Trust Technology enablement state.

  @param[out] IsPttEnabledState  TRUE if PTT is enabled, FALSE othewrwise.

  @retval EFI_SUCCESS            Command succeeded
  @retval EFI_DEVICE_ERROR       HECI Device error, command aborts abnormally
**/
EFI_STATUS
EFIAPI
PttHeciGetState (
  OUT BOOLEAN   *IsPttEnabledState
  );

/**
  Changes current Platform Trust Technology state.

  @param[in] PttEnabledState     TRUE to enable, FALSE to disable.

  @retval EFI_SUCCESS            Command succeeded
  @retval EFI_DEVICE_ERROR       HECI Device error, command aborts abnormally
**/
EFI_STATUS
EFIAPI
PttHeciSetState (
  IN  BOOLEAN   PttEnabledState
  );

//
// -----------------------------------------------------------------------------------------------------------------------
// PEI Phase protocol declaration
// -----------------------------------------------------------------------------------------------------------------------
//
/**
  Send Get Firmware SKU Request to SEC

  @param[in,out] FwCapsSku          SEC Firmware Capability SKU

  @exception EFI_UNSUPPORTED        Current SEC mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetFwCapsSkuMsg(
  IN OUT SECFWCAPS_SKU             *FwCapsSku
  );

/**
  This message is sent by the BIOS or IntelR MEBX. One of usages is to utilize
  this command to determine if the platform runs in Consumer or Corporate SKU
  size firmware.

  @param[out] RuleData            PlatformBrand,
                                  IntelMeFwImageType,
                                  SuperSku,
                                  PlatformTargetUsageType

  @exception EFI_UNSUPPORTED      Current SEC mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetPlatformTypeMsg(
  OUT PLATFORM_TYPE_RULE_DATA     *RuleData
  );

/**
  This message is sent by the BIOS or IntelR MEBX. To Get Firmware Version Request to SEC.

  @param[in,out] MsgGenGetFwVersionAck    Return themessage of FW version

  @exception EFI_UNSUPPORTED      Current SEC mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
PeiHeciGetFwVersionMsg(
  IN OUT GEN_GET_FW_VER_ACK     *MsgGenGetFwVersionAck
  );

/**
Wait for CSE reset bit set and then Reset HECI interface.

@param[out] Reset_Status        Reset Status value

@retval EFI_SUCCESS             Command succeeded
@retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
CheckCseResetAndIssueHeciReset(
OUT BOOLEAN *Reset_Status
);
typedef struct {
  MKHI_MESSAGE_HEADER    MkhiHeader;
} CORE_BIOS_DONE;
typedef struct {
  MKHI_MESSAGE_HEADER    MkhiHeader;
} CORE_BIOS_DONE_ACK;
#define COREBIOS_DONE        1
#define COREBIOS_NOT_DONE    0
VOID
EFIAPI
HeciCoreBiosDoneMsg (
    void
  );
  EFI_STATUS
HeciDataClearLock (
  VOID
  );
  VOID
EFIAPI
HeciDXECallback (
  IN EFI_EVENT       Event,
  IN VOID            *ParentImageHandle
  );

/**
  Get SMM Trusted Key.

  @param[out] SmmTrustedKeyData       Smm trusted data buffer.

  @return EFI_SUCCESS   Get SMM Trusted success.
  @return Others              Get SMM Trusted failed.
**/
EFI_STATUS
HeciGetSMMTrustedKey (
  OUT MBP_SMM_TRUSTED_KEY    *SmmTrustedKeyData
  );

/**
Get ARB Status
Send 'Get ARB Status' command to CSE to get ARB Status information in the received buffer.
Host Calls this API on boot after ifwi update.

@param[out] GetArbStatus  Structure filled with ARB status info

@return EFI_SUCCESS       SVN info populated correctly. Refer ARB Out put structure for info
@return other values      Refer BIOS CSE Interface document
**/
EFI_STATUS
HeciGetArbStatus(
IN OUT GET_ARB_STATUS_ACK     *GetArbStatus
);

/**
Commit ARB SVN Updates
Send 'Commit ARB SVN Updates' command to CSE to commit ARB SVN information.
Host Calls this API on boot after ifwi update.

@param[in] CommitSvns    Structure filled with ARB SVN Commit info

@return EFI_SUCCESS       SVN info populated correctly. Refer ARB Out put structure for info
@return other values      Refer BIOS CSE Interface document
**/
EFI_STATUS
HeciCommitArbSvnUpdates(
IN UINT8     *CommitSvns
);

/**
BIOS requests to write bios2ish file for ISH.

@param[in] Ish2CseData          Pointer to ish2cse binary includeing Header

@retval EFI_SUCCESS             Command succeeded
**/
EFI_STATUS
HeciWriteIshNVMFile(
IN  ISH_SRV_HECI_SET_FILE_REQUEST *Ish2CseData
);

/**
This Function gets MBP data from CSE and creates Dedicated HOB for Nonsensitive and Sensitive MBP data

@retval     EFI_SUCCESS      Get MBP data from CSE and Created HOB for Sensitive and Non_Sensitive successfully.
@retval     EFI_NOT_FOUND    Fail to create a HOB
@retval     Others           CSME fail to return MBP data

**/
EFI_STATUS
HeciMBP (
  VOID
  );

/**
This Function checks for Non_Sensitive Data Hob, if Hob not found return error status

@param[out]  NonSensitiveMBPData    Pointer to Non_Sensitive MBP data

@retval     EFI_SUCCESS            Return NonSensitive HeciMBP data successfully.
@retval     EFI_INVALID_PARAMETER  The parameter NonSensitiveMBPData is NULL.
@retval     EFI_NOT_FOUND          NonSensitive HeciGetMBPData HOB not found
**/
EFI_STATUS
NonSensitiveHeciMBP (
  OUT UINT32 *NonSensitiveMBPData
  );

//[-start-190321-IB07401094-add]//
#pragma pack (1)
#define BOOTLOADER_SEED_MAX_ENTRIES 4
typedef struct
{
    UINT8                               Svn;
    UINT8                               Reserved[3];
    UINT8                               Seed[32];
}  BUP_BOOTLOADER_SEED_INFO;

typedef struct
{
    MKHI_MESSAGE_HEADER                MKHIHeader;
}  BUP_COMMON_BOOTLOADER_GET_SEED_LIST_Request;

typedef struct
{
    MKHI_MESSAGE_HEADER                     MKHIHeader;
    UINT32                                  NumOfSeeds;
    BUP_BOOTLOADER_SEED_INFO                SeedList[BOOTLOADER_SEED_MAX_ENTRIES];
}  BUP_COMMON_BOOTLOADER_GET_SEED_LIST_Response;

typedef struct
{
    UINT32                                  NumOfSeeds;
    BUP_BOOTLOADER_SEED_INFO                SeedList[BOOTLOADER_SEED_MAX_ENTRIES];
}  BUP_BOOTLOADER_SEED_LIST_INFO;


/**
  Get Bootloader Seed List

  @param[out] BLSList      Output CSE Seed List data

  @return EFI_SUCCESS   Get seed success.
  @return Others        Get seed failed.
**/
EFI_STATUS
HeciGetBootloaderSeedList(
IN  BUP_BOOTLOADER_SEED_LIST_INFO *BlSList
);
#pragma pack ()
//[-end-190321-IB07401094-add]//

#endif


