/** @file
This file does Multiplatform initialization

@copyright
Copyright (c) 2010 - 2016 Intel Corporation. All rights reserved.
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
This file contains a 'Sample Driver' and is licensed as such
under the terms of your license agreement with Intel or your
vendor. This file may be modified by the user, subject to
the additional terms of the license agreement.

@par Specification Reference:
**/

#include "BoardOemIdsAplk/BoardOemIds.h"
#include "BoardSsidSvidAplk/BoardSsidSvid.h"

#include <Library/MultiPlatformLib.h>
//[-start-151222-IB11270140-add]//
#include <Library/PeiOemSvcChipsetLib.h>
//[-end-151222-IB11270140-add]//
//[-start-160603-IB06720411-modify]//
/**
  Configure GPIO group GPE tier.

  @param[in]  PlatformInfo

  @retval     none.
**/
VOID
GpioGroupTierInit (
  IN EFI_PLATFORM_INFO_HOB  *PlatformInfoHob
  )
{
  DEBUG ((DEBUG_INFO, "GpioGroupTierInit Start\n"));
  switch (PlatformInfoHob->BoardId) {
    default:
//[-start-161128-IB07400820-modify]//
//      GpioSetGroupToGpeDwX (GPIO_BXTP_GROUP_7,  // map group 7 to GPE 0 ~ 31
//                            GPIO_BXTP_GROUP_0,  // map group 0 to GPE 32 ~ 63 // We don't have SCI pin in Group0 as of now, but still need to assign a unique group to this field.
//                            GPIO_BXTP_GROUP_1); // map group 1 to GPE 64 ~ 95 // We don't have SCI pin in Group1 as of now, but still need to assign a unique group to this field.
      GpioSetGroupToGpeDwX ((0x0200 | (UINT32)PcdGet8 (PcdGpe0bGpioGroup)),  
                            (0x0200 | (UINT32)PcdGet8 (PcdGpe0cGpioGroup)),  
                            (0x0200 | (UINT32)PcdGet8 (PcdGpe0dGpioGroup))); 
//[-end-161128-IB07400820-modify]//
      break;
  }
  
  DEBUG ((DEBUG_INFO, "GpioGroupTierInit End\n"));
}

/*++
Routine Description:

  Platform Type detection. Because the PEI globle variable
  is in the flash, it could not change directly.So use
  2 PPIs to distinguish the platform type.

Arguments:

  FfsHeader    -  Pointer to Firmware File System file header.
  PeiServices  -  General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS  -  Memory initialization completed successfully.
  Others       -  All other error conditions encountered result in an ASSERT.

--*/
EFI_STATUS
MultiPlatformInfoInit (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB  *PlatformInfoHob
  )
{
  EFI_STATUS             Status;
  
#if (ENBDT_PF_ENABLE == 1)
  #if RVVP_ENABLE
     PlatformInfoHob->BoardId = BOARD_ID_BXT_RVVP;
     PlatformInfoHob->FABID   = FAB1;
     
     DEBUG ((EFI_D_ERROR, "RVVP BXTP GPIO Platform BoardId:%x FabId%x\n", PlatformInfoHob->BoardId, PlatformInfoHob->FABID));
  
  #else
//[-start-160623-IB07400747-remove]//
//    #if !(BXTI_PF_ENABLE)
//      //
//      // Get Board Id and Fab Id from EC
//      // Note: The calls to EC will fail in pre-si
//      //
//      if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
////[-start-151222-IB11270140-modify]//
//        UINT8 BoardId = BOARD_ID_APL_UNKNOWN;
//    	  UINT8 FabId   = FAB1;
//    Status = OemSvcGetBoardFabIds(&BoardId, &FabId);
//    if (Status == EFI_MEDIA_CHANGED) {
//      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcGetBoardFabIds, Status : %r\n", Status)); 
//      DEBUG((EFI_D_ERROR | EFI_D_INFO, "Platform BoardId:%x FabId%x\n", BoardId, FabId));
//    } else {    
//    	  Status = GetBoardIdFabId (&BoardId, &FabId);
//    }    
////[-end-151222-IB11270140-modify]//
//    	  PlatformInfoHob->BoardId = BoardId;
//    	  PlatformInfoHob->FABID   = FabId;
//        
//    	  if (Status == EFI_SUCCESS) {
//    	    DEBUG ((EFI_D_INFO, "Platform BoardId:%x FabId%x\n", PlatformInfoHob->BoardId, PlatformInfoHob->FABID));
//    	  }
//      }
//    #else
//      DEBUG ((EFI_D_INFO, "Platform BoardId:%x FabId%x\n", PlatformInfoHob->BoardId, PlatformInfoHob->FABID));
//    #endif
//[-end-160623-IB07400747-remove]//
  #endif
#endif
  
  //
  // Device ID
  //
  PlatformInfoHob->IohSku = MmPci16(0, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, PCI_DEVICE_ID_OFFSET);

  PlatformInfoHob->IohRevision = MmPci8(0, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, PCI_REVISION_ID_OFFSET);
  
  //
  // Don't support BASE above 4GB currently
  //
  PlatformInfoHob->PciData.PciExpressSize     = 0x04000000;
  PlatformInfoHob->PciData.PciExpressBase     = (UINTN) PcdGet64 (PcdPciExpressBaseAddress);

  PlatformInfoHob->PciData.PciResourceMem32Base  = (UINT32) (PlatformInfoHob->PciData.PciExpressBase - RES_MEM32_MIN_LEN);
  PlatformInfoHob->PciData.PciResourceMem32Limit = (UINT32) (PlatformInfoHob->PciData.PciExpressBase -1);

  PlatformInfoHob->PciData.PciResourceMem64Base   = RES_MEM64_36_BASE;
  PlatformInfoHob->PciData.PciResourceMem64Limit  = RES_MEM64_36_LIMIT;
  PlatformInfoHob->CpuData.CpuAddressWidth        = 36;

  PlatformInfoHob->MemData.MemMir0 = PlatformInfoHob->PciData.PciResourceMem64Base;
  PlatformInfoHob->MemData.MemMir1 = PlatformInfoHob->PciData.PciResourceMem64Limit + 1;

  PlatformInfoHob->PciData.PciResourceMinSecBus  = 1;  //can be changed by SystemConfiguration->PciMinSecondaryBus;

  //
  // Set MemMaxTolm to the lowest address between PCIe Base and PCI32 Base
  //
  if (PlatformInfoHob->PciData.PciExpressBase > PlatformInfoHob->PciData.PciResourceMem32Base ) {
    PlatformInfoHob->MemData.MemMaxTolm = (UINT32) PlatformInfoHob->PciData.PciResourceMem32Base;
  } else {
    PlatformInfoHob->MemData.MemMaxTolm = (UINT32) PlatformInfoHob->PciData.PciExpressBase;
  }
  PlatformInfoHob->MemData.MemTolm = PlatformInfoHob->MemData.MemMaxTolm;

  //
  // Platform PCI MMIO Size in unit of 1MB
  //
  PlatformInfoHob->MemData.MmioSize = 0x1000 - (UINT16)(PlatformInfoHob->MemData.MemMaxTolm >> 20);

  //
  // Enable ICH IOAPIC
  //
  PlatformInfoHob->SysData.SysIoApicEnable  = ICH_IOAPIC;

  DEBUG ((EFI_D_INFO,  "PlatformFlavor is %x (%x=tablet,%x=mobile,%x=desktop)\n", PlatformInfoHob->PlatformFlavor, FlavorTablet, FlavorMobile, FlavorDesktop));

  //
  // Get Platform Info and fill the Hob
  //
  PlatformInfoHob->RevisonId = PLATFORM_INFO_HOB_REVISION;

  if ((PLATFORM_ID == VALUE_REAL_PLATFORM) ||
  	  (PLATFORM_ID == VALUE_PURE_SIMICS)   ||
      (PLATFORM_ID == VALUE_PUREVP)        ||
      (PLATFORM_ID == VALUE_SLE)
     ) {
    //
    // Get GPIO table
    //
    Status = MultiPlatformGpioTableInit (PeiServices, PlatformInfoHob);
    ASSERT_EFI_ERROR (Status);
    
    //
    // Program GPIO
    //
    Status = MultiPlatformGpioProgram (PeiServices, PlatformInfoHob);
    
    if (GetBxtSeries () == BxtP) {
      GpioGroupTierInit (PlatformInfoHob);
    }
  }

  // Update OemId
  Status = InitializeBoardOemId (PeiServices, PlatformInfoHob);
  Status = InitializeBoardSsidSvid (PeiServices, PlatformInfoHob);

  return EFI_SUCCESS;
}
//[-end-160603-IB06720411-modify]//
