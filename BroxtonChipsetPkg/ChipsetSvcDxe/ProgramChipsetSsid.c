/** @file
 DXE Chipset Services - Program SSID/SVID. 

 It produces an instance of the DXE Chipset Services protocol to provide the chipset related functions 
 which will be used by Kernel or Project code. These protocol functions should be used by calling the 
 corresponding functions in DxeChipsetSvcLib to have the protocol size checking

***************************************************************************
* Copyright (c) 2012-2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <PiDxe.h>

//
// Libraries
//
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/PcdLib.h>

//
// Consumed Reference Code
//
#include <ScAccess.h>

#define DEVICE_ID_DONT_CARE           0xFFFF

typedef
VOID
(*SPECIAL_PROGRAM_MECHANISM) (
  IN     UINT8    Bus,
  IN     UINT8    Dev,
  IN     UINT8    Func,
  IN     UINT32   SsidSvid
  );

typedef struct {
  UINT16                       VendorId;
  UINT16                       DeviceId;
  SPECIAL_PROGRAM_MECHANISM    SpecialSsidSvidFunction;
} SPECIFIC_SSID_SVID_TABLE;



//
// Special Programming Table
//
SPECIFIC_SSID_SVID_TABLE mSpecificSsidSvidTable[] ={

  {DEVICE_ID_DONT_CARE  , DEVICE_ID_DONT_CARE   , NULL}
};

/**
 Program Chipset SSID

 @param[in]         Bus                 PCI Bus number
 @param[in]         Dev                 PCI Device number
 @param[in]         Func                PCI Function number
 @param[in]         VendorId            Vendor ID
 @param[in]         DeviceId            Device ID
 @param[in]         SsidVid             SsidVid

 @retval            EFI_SUCCESS         Function returns successfully
 @retval            EFI_UNSUPPORTED     1. The specific ID is not find.
                                        2. The specific device can not be set SSID.
*/
EFI_STATUS
ProgramChipsetSsid (
  IN UINT8                               Bus,
  IN UINT8                               Dev,
  IN UINT8                               Func,
  IN UINT16                              VendorId,
  IN UINT16                              DeviceId,
  IN UINT32                              SsidSvid
  )
{
  UINT8                               Index;

  for (Index = 0; mSpecificSsidSvidTable[Index].SpecialSsidSvidFunction != NULL; Index++ ) {
    if (mSpecificSsidSvidTable[Index].VendorId == VendorId) {
      if ((mSpecificSsidSvidTable[Index].DeviceId == DEVICE_ID_DONT_CARE)
        || (mSpecificSsidSvidTable[Index].DeviceId == DeviceId)) {
        mSpecificSsidSvidTable[Index].SpecialSsidSvidFunction (Bus, Dev, Func, SsidSvid);
        DEBUG ((EFI_D_INFO, "Speicial Program SVID/SID for B%d/D%d/F%d\n", Bus, Dev, Func));
        return EFI_SUCCESS;
      }
    }
  }
  return EFI_UNSUPPORTED;
} 
