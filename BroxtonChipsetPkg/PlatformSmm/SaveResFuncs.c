/** @file
  Supports functions that saves and restores to the global register table
  information of PIC, KBC, PCI, CpuState, Edge Level, GPIO, and MTRR.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Base.h>
#include <DeviceRegTable.h>
#include <ScAccess.h>
#include <Library/PciLib.h>
#include <IndustryStandard/Pci22.h>
#include <Library/DebugLib.h>
#include <Library/SmmOemSvcChipsetLib.h>
#include <MmioAccess.h>
#include <Library/BaseLib.h>
#include <Library/CpuIA32.h>

#define APIC_REGISTER_TPR_OFFSET              0x000000080
#define APIC_REGISTER_TIMER_VECTOR_OFFSET     0x000000320
#ifndef APIC_REGISTER_SPURIOUS_VECTOR_OFFSET
#define APIC_REGISTER_SPURIOUS_VECTOR_OFFSET  0x0000000F0
#endif

UINT32 ApicTPR;
UINT32 ApicDCR;
UINT32 ApicLVT[6];

//[-start-160421-IB10860195-remove]//
//[-start-160411-IB10860194-add]//
//EFI_STATUS
//SaveRestoreSmiEnable (
//  IN  BOOLEAN  SaveRestoreFlag 
//)
//{
//  UINT16 AcpiBase;
//  UINTN  Count;
//  
//  AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
//  
//  for (Count = 0; SaveSmiEnRegisters[Count] != (UINT32)-1; Count ++) {
//    if (SaveRestoreFlag) {
//      IoWrite32 (AcpiBase + SaveSmiEnRegisters[Count], SmiEnRegistersSave[Count]);
//    } else {
//      SmiEnRegistersSave[Count] = IoRead32 (AcpiBase + SaveSmiEnRegisters[Count]);
//    }
//  }
//  
//  return EFI_SUCCESS;
//}
//[-end-160411-IB10860194-add]//
//[-end-160421-IB10860195-remove]//
/**
  This function either saves or restores PMC registers.

  @param  SaveRestoreFlag -  True: Restores PMC registers.
                             False: Save PMC registers.
                            
  @retval EFI_SUCCESS
  
**/
EFI_STATUS
SaveRestorePMC (
  IN  BOOLEAN  SaveRestoreFlag 
)
{
  UINT32 PMCBase;
  UINTN  Count;
  
  
//[-start-160802-IB03090430-modify]//
  PMCBase        = PMC_BASE_ADDRESS;
//[-end-160802-IB03090430-modify]//
  
  for (Count = 0; SavePMCRegisters[Count] != (UINT32)-1; Count ++) {
    if (SaveRestoreFlag) {
      Mmio32 (PMCBase, SavePMCRegisters[Count]) = PMCRegistersSave[Count];
    } else {
      PMCRegistersSave[Count] = Mmio32 (PMCBase, SavePMCRegisters[Count]);
    }
  }
  
  return EFI_SUCCESS;
}

/**
 This function either writes to or read from PCI registers.

 @param [in]        SaveRestoreFlag     True: write data to PCI registers.
                                        False: read data from PCI registers to global registers.

 @retval            EFI_SUCCESS         if read or write is successful.
*/
EFI_STATUS
SaveRestorePci (
  IN  BOOLEAN                           SaveRestoreFlag
  )
{
  UINTN                                 Index;
  UINT8                                 Reg;
  SR_OEM_DEVICE                         *SRDev;
  SR_OEM_DEVICE                         *SROemDev;
  EFI_STATUS                            Status;
  
  SROemDev = NULL;
  Status = OemSvcGetSaveRestorePciDeviceOemList (&SROemDev);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemSvcGetSaveRestorePciDeviceOemList, Status : %r\n", Status));
  if (Status == EFI_UNSUPPORTED) {
    return EFI_UNSUPPORTED;
  }

  if (!SaveRestoreFlag) {
    SRDev = SROemDev;
    while (SRDev->Device.RegNum != 0) {
      if (SRDev->P2PB.Bus + SRDev->P2PB.Dev + SRDev->P2PB.Fun != 0x00) {
        SRDev->Device.PciBus = PciRead8 (PCI_LIB_ADDRESS (
                                  SRDev->P2PB.Bus,
                                  SRDev->P2PB.Dev,
                                  SRDev->P2PB.Fun,
                                  PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET + 1
                                  ));
      }
      ++SRDev;
    }
  }
  
  SRDev = SROemDev;
  while (SRDev->Device.RegNum != 0) {
    if (PciRead32 (PCI_LIB_ADDRESS (SRDev->Device.PciBus, SRDev->Device.PciDev, SRDev->Device.PciFun, 0)) != 0xFFFFFFFF) {
      if (SRDev->Device.PciRegTable == NULL) {
        //
        // Save/Restore Full Page.
        //
        for (Reg = 0xFC, Index = 0; Index < 0x100; Reg -= 4, Index += 4) {
          if (SaveRestoreFlag) {
            PciWrite32 (
              PCI_LIB_ADDRESS (
                SRDev->Device.PciBus,
                SRDev->Device.PciDev,
                SRDev->Device.PciFun,
                Reg),
              SRDev->Device.PciRegTableSave[Index/4]);
          } else {
            SRDev->Device.PciRegTableSave[Index/4] = PciRead32 (PCI_LIB_ADDRESS (
                                                        SRDev->Device.PciBus,
                                                        SRDev->Device.PciDev,
                                                        SRDev->Device.PciFun,
                                                        Reg
                                                        ));
          }
        }
      } else {
        //
        // Save/Restore PCI configuration registers to SMRAM.
        //
        for (Index = 0; Index < SRDev->Device.RegNum; ++Index) {
          if (SaveRestoreFlag) {
            PciWrite32 (
              PCI_LIB_ADDRESS (
                SRDev->Device.PciBus,
                SRDev->Device.PciDev,
                SRDev->Device.PciFun,
                SRDev->Device.PciRegTable[Index]),
              SRDev->Device.PciRegTableSave[Index]);
          } else {
            SRDev->Device.PciRegTableSave [Index] = PciRead32 (PCI_LIB_ADDRESS (
                                                        SRDev->Device.PciBus,
                                                        SRDev->Device.PciDev,
                                                        SRDev->Device.PciFun,
                                                        SRDev->Device.PciRegTable[Index]
                                                        ));
          }
        }
      }
    }
    ++SRDev;
  }
  
  return EFI_SUCCESS;
}


/**
 Disable APIC

 @param None.

 @retval None.
 
**/
STATIC
VOID
DisableAPIC (
  )
{
  UINT64          ApicBaseReg;
  UINT8           *ApicBase;
  UINTN           Index;
  UINTN           Offset;
  
  ApicBaseReg = AsmReadMsr64(EFI_MSR_IA32_APIC_BASE);
  if (ApicBaseReg & EFI_APIC_GLOBAL_ENABLE) {
    ApicBase = (UINT8*)(UINTN)(ApicBaseReg & 0xffffff000);
    //
    // Backup the whole LVTs due to software disable APIC will set the mask value of LVTs
    //
    for (Index = 0, Offset = APIC_REGISTER_TIMER_VECTOR_OFFSET; Index < 6; Index ++, Offset += 0x10) {
      ApicLVT[Index] = *(UINT32*)(UINTN)(ApicBase + Offset);
    }
    //
    // Backup the TPR
    //
    ApicTPR = *(UINT32*)(UINTN)(ApicBase + APIC_REGISTER_TPR_OFFSET);
    //
    // Set the TPR to 0xff to block whole queuing interrupts(in the IRR)
    //
    *(UINT32*)(UINTN)(ApicBase + APIC_REGISTER_TPR_OFFSET) = 0xff;
    //
    // Software disable APIC
    //
    *(UINT32*)(UINTN)(ApicBase + APIC_REGISTER_SPURIOUS_VECTOR_OFFSET) &= ~0x100;
  }
}

/**
 Enable APIC

 @param None.

 @retval None.

**/
STATIC
VOID
EnableAPIC (
  )
{
  UINT64 ApicBaseReg;
  UINT8  *ApicBase;
  UINTN  Index;
  UINTN  Offset;

  ApicBaseReg = AsmReadMsr64(EFI_MSR_IA32_APIC_BASE);
  if (ApicBaseReg & EFI_APIC_GLOBAL_ENABLE) {
    ApicBase = (UINT8*)(UINTN)(ApicBaseReg & 0xffffff000);
    //
    // Software enable APIC
    //
    *(UINT32*)(UINTN)(ApicBase + APIC_REGISTER_SPURIOUS_VECTOR_OFFSET) |= 0x100;
    //
    // Restore whole LVTs
    //
    for (Index = 0, Offset = APIC_REGISTER_TIMER_VECTOR_OFFSET; Index < 6; Index ++, Offset += 0x10) {
      *(UINT32*)(UINTN)(ApicBase + Offset) = ApicLVT[Index];
    }
    //
    // Restore the TPR
    //
    *(UINT32*)(UINTN)(ApicBase + APIC_REGISTER_TPR_OFFSET) = ApicTPR;
  }
}

/**
 Before setting MTRR, we need to disable interrupt and cache.

 @param None.

 @retval None

**/
EFI_STATUS
EfiPreMtrrChange()
{
  UINT64                      TempQword;

  //
  // Disable Interrupt ==> Braswell will hang in dual mode.
  //
  DisableAPIC ();

  //
  // if it is P4 processor, Disable Cache, else don't disable cache,
  // otherwise the system will hang ==> Braswell will hang.
  //
  //AsmDisableCache ();

  //
  // Disable Cache MTRR
  //
  TempQword = AsmReadMsr64(EFI_CACHE_IA32_MTRR_DEF_TYPE);
  AsmWriteMsr64(EFI_CACHE_IA32_MTRR_DEF_TYPE, TempQword & ~EFI_CACHE_MTRR_VALID);

  return EFI_SUCCESS;
}

/**
 Enable cache after we set MTRR.

 @param None.

 @retval None

**/
EFI_STATUS
EfiPostMtrrChange()
{
  UINT64                      TempQword=0;

  //
  // Enable Cache MTRR
  //
  TempQword = AsmReadMsr64(EFI_CACHE_IA32_MTRR_DEF_TYPE);
  AsmWriteMsr64(EFI_CACHE_IA32_MTRR_DEF_TYPE, TempQword | EFI_CACHE_MTRR_VALID);

  //
  // if it is P4 processor, re- enable L2 cache
  //
  //AsmEnableCache ();
  
  //
  // Enable Interrupt
  //
  EnableAPIC();  

  return EFI_SUCCESS;
}

/**
 This function either saves or restores Mtrr registers.

 @param [in]        *FlagBuffer         True: write data to MTRR registers.
                                        False: read data from MTRR registers to global registers.

 @retval            EFI_SUCCESS         if read or write is successful.
*/
EFI_STATUS
EFIAPI
SaveRestoreMtrr (
  IN  VOID        *FlagBuffer
  )
{
  UINT32                           Index;
  BOOLEAN                          SaveRestoreFlag;

  SaveRestoreFlag = *(BOOLEAN *)FlagBuffer;
  
  if (SaveRestoreFlag) {
    EfiPreMtrrChange();
  }

  // Save/Restore Fixed Mtrr
  Index = 0;
  while (FixedMtrr[Index] != 0xFFFFFFFF) {
    if (SaveRestoreFlag) {
      AsmWriteMsr64 (FixedMtrr[Index], FixedMtrrSave[Index]);
    } else {
      FixedMtrrSave[Index] = AsmReadMsr64 (FixedMtrr[Index]);
    }
    Index++;
  }
   for (Index = EFI_CACHE_VARIABLE_MTRR_BASE; Index < EFI_CACHE_VARIABLE_MTRR_END; Index += 2) {
     if (SaveRestoreFlag) {
       AsmWriteMsr64 (Index, VariableMtrrSave[Index - EFI_CACHE_VARIABLE_MTRR_BASE]);
       AsmWriteMsr64 (Index + 1, VariableMtrrSave[Index - EFI_CACHE_VARIABLE_MTRR_BASE + 1]);
     } else {
       VariableMtrrSave [Index - EFI_CACHE_VARIABLE_MTRR_BASE] = AsmReadMsr64 (Index);
       VariableMtrrSave [Index - EFI_CACHE_VARIABLE_MTRR_BASE + 1] = AsmReadMsr64 (Index + 1);
     }
   }
  if (SaveRestoreFlag) {
    //
    // Set MTRRdefType register
    //
    AsmWriteMsr64 (EFI_CACHE_IA32_MTRR_DEF_TYPE, EFI_MTRR_DEF_TYPE_ENABLE);
    EfiPostMtrrChange();
  }

  return  EFI_SUCCESS;
}
