/** @file
  Header file of Checkpoint related functions

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#ifndef _INTERNAL_CHECKPOINT_H_
#define _INTERNAL_CHECKPOINT_H_

/**
  Internal function to initialize H2O_BDS_CP_BOOT_FAILED_PROTOCOL data and trigger
  gH2OBdsCpBootFailedProtocolGuid checkpoint.

  @param[in]  ExitData            Exit data returned from failed boot.
  @param[in]  ExitDataSize        Exit data size returned from failed boot.
  @param[in]  Option              Pointer to Boot Option that succeeded to boot.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpBootFailedProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpBootFailedProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpBootFailed (
  IN  EFI_STATUS         Status,
  IN  CHAR16             *ExitData,
  IN  UINTN              ExitDataSize
  );

/**
  Internal function to initialize H2O_BDS_CP_BOOT_SUCCESS_PROTOCOL data and trigger
  gH2OBdsCpBootSuccessProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpBootSuccessProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpBootSuccessProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpBootSuccess (
  VOID
  );

/**
  Initalize H2O_BDS_CP_CON_IN_CONNECT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpConInConnectAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConInConnectAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConInConnectAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConInConnectAfter (
  VOID
  );

/**
  Initalize H2O_BDS_CP_CON_IN_CONNECT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpConInConnectBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConInConnectBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConInConnectBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConInConnectBefore (
  VOID
  );

/**
  Initalize H2O_BDS_CP_CON_OUT_CONNECT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpConOutConnectAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConOutConnectAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConOutConnectAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConOutConnectAfter (
  VOID
  );

/**
  Initalize H2O_BDS_CP_CON_OUT_CONNECT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpConOutConnectBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConOutConnectBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConOutConnectBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConOutConnectBefore (
  VOID
  );

/**
  Internal function to initialize H2O_BDS_CP_CONNECT_ALL_AFTER_PROTOCOL data and trigger
  gH2OBdsCpConnectAllAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConnectAllAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConnectAllAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConnectAllAfter (
  VOID
  );

/**
  Internal function to initialize H2O_BDS_CP_CONNECT_ALL_BEFORE_PROTOCOL data and trigger
  gH2OBdsCpConnectAllBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpConnectAllBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpConnectAllBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpConnectAllBefore (
  VOID
  );

#endif