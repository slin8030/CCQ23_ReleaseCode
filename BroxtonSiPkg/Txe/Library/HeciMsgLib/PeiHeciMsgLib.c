/** @file
  Implementation file for PEI specific HECI Message functionality

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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

@par Specification Reference

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/HeciMsgLib.h>

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
  )
{
  Data = NULL;

  return EFI_NOT_AVAILABLE_YET;
}

/**
  Write data to NVM file through HECI2.

  @param[in] FileName   The file name.
  @param[in] Offset     The offset of data.
  @param[in] Data       The data content.
  @param[in] DataSize   Data's size.
  @param[in] Truncate   Truncate the file.

  @return EFI_SUCCESS   Write NVM file success.
  @return Others        Write NVM file failed.
**/
EFI_STATUS
Heci2WriteNVMFile (
  IN CONST CHAR8   *FileName,
  IN       UINT32  Offset,
  IN CONST UINT8   *Data,
  IN       UINTN   DataSize,
  IN       BOOLEAN Truncate
  )
{
  return EFI_NOT_AVAILABLE_YET;
}

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
  )
{
  *FileSize = 0;

  return EFI_NOT_AVAILABLE_YET;
}

/**
  Send Get Proxy State message through Heci2.

  @return EFI_SUCCESS   Send message success.
  @return Others              Send message failed.
**/
EFI_STATUS
Heci2GetProxyState(
  VOID
  )
{
  return EFI_NOT_AVAILABLE_YET;
}

/**
  Check is the HECI2 device has interrupt.

  @return TRUE     HECI2 device interrupt.
  @return FALSE    No interrupt.
**/
BOOLEAN
Heci2GetInterrupt(
  VOID
  )
{
  return FALSE;
}

/**
  Lock Directory message through HECI2.

  @param[in] DirName       The Directory name.
  @param[in] Heci2Protocol The HECI protocol to send the message to HECI2 device.

  @return EFI_SUCCESS      Send EOP message success.
  @return Others           Send EOP message failed.
**/
EFI_STATUS
Heci2LockDirectory (
  IN UINT8               *DirName,
  IN EFI_HECI_PROTOCOL   *Heci2Protocol
  )
{
  return EFI_NOT_AVAILABLE_YET;
}

/**
  Send Get proxy State message through HEC2 but not waiting for response.

  @param[in] Heci2Protocol The HECI protocol to send the message to HECI2 device.

  @return EFI_SUCCESS      Send get proxy state message success.
  @return Others           Send get proxy state message failed.
**/
EFI_STATUS
Heci2GetProxyStateNoResp(
  IN EFI_HECI_PROTOCOL         *Heci2Protocol
  )
{
  return EFI_NOT_AVAILABLE_YET;
}

/**
  Returns whether trusted channel is enabled.

  @param[in]  None.

  @retval     TRUE if trusted channel is enabled.
              FALSE if trusted channel is disabled.
**/
BOOLEAN
IsTrustedChannelEnabled (
  VOID
  )
{
  return FALSE;
}

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
  )
{
  return EFI_NOT_AVAILABLE_YET;
}
