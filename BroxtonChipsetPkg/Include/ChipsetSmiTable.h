/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CHIPSET_SMI_TABLE_DEFINE_H_
#define _CHIPSET_SMI_TABLE_DEFINE_H_

typedef enum {
  //
  // Kernel must use 0x01~0x4F as SW_SMI command number. Register on SmiTable.h
  //
  //
  // OEM must use 0xc0~0xDF as SW_SMI command number. Register on OemSwSmi.h
  //
  
  APM_12_FUNCS                    = 0x50,
  SMI_SET_SMMVARIABLE_PROTOCOL    = 0x51,
  SMM_FROM_SMBASE_DRIVER          = 0x55,
  TPM_PTS                         = 0x5A,
//[-start-190314-IB07401090-add]//
  MSR_CTR_OVERRIDE_HANDLER        = 0x5C,
//[-end-190314-IB07401090-add]//
  USB_LEGACY_SUPPORT_SW_SMI       = 0x60, 
//[-start-170626-IB07400880-modify]//
  WAKE_ON_USB_FROM_S5_SMI         = 0x61,
//[-end-170626-IB07400880-modify]//
//[-start-160330-IB07220057-add]//
  SW_SMI_S3_RESTORE_MSR           = 0x62, // CpuPowerMgmt.h
//[-end-160330-IB07220057-add]//
  //
  // FRC used SW_SMI number(0x50~0xBF).
  // OS_COMMAND                   = 0x80
  // APP_COMMAND                  = 0x82
  // C_STATE_COMMAND              = 0x85
  SW_SMI_DTS                      = 0x66,
  INSYDE_POWER_STATE_SWITCH_SMI                     = 0x67,
  INSYDE_ENABLE_C_STATE_IO_REDIRECTION_SMI          = 0x68,
  INSYDE_DISABLE_C_STATE_IO_REDIRECTION_SMI         = 0x69,
  INSYDE_ENABLE_SMI_C_STATE_COORDINATION_SMI        = 0x6A,
  INSYDE_DISABLE_SMI_C_STATE_COORDINATION_SMI       = 0x6B,
  INSYDE_ENABLE_P_STATE_HARDWARE_COORDINATION_SMI   = 0x6C,
  INSYDE_DISABLE_P_STATE_HARDWARE_COORDINATION_SMI  = 0x6D,
  INSYDE_S3_RESTORE_MSR_SW_SMI                      = 0x6E,
  INSYDE_ENABLE_C6_RESIDENCY_SMI                    = 0x6F,
  //
  //  For Variable Editor Tool
  //
  IVE_SW_SMI                                        = 0x97 
} CHIPSET_SW_SMI_PORT_TABLE;

//[-start-170314-IB07400847-add]//
#define CHIPSET_MISC_SW_SMI_SHUTDOWN_HOOK  0x01
//[-end-170314-IB07400847-add]//

//[-start-160318-IB07400711-add]//
//[-start-160808-IB07400769-modify]//
//[-start-160928-IB07400789-modify]//
typedef enum {
  //
  // Chipset2 used SW_SMI number(0x90~0xBF).
  //
  CHIPSET_FEATURE_SW_SMI            = 0x90,
  USB_LEGACY_CONTROL_SW_SMI         = 0x91,
//[-start-170314-IB07400847-add]//
  CHIPSET_MISC_SW_SMI               = 0x92,
//[-end-170314-IB07400847-add]//
} CHIPSET_SW_SMI_PORT_TABLE2;
//[-end-160928-IB07400789-modify]//
//[-end-160808-IB07400769-modify]//
//[-end-160318-IB07400711-add]//
#endif

