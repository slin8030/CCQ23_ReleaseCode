/** @file
PEI Chipset Services.

 This file contains only one function that is PeiCsSvcPlatformStage1Init().
 The function PeiCsSvcPlatformStage1Init() use chipset services to install
 Firmware Volume Hob's once there is main memory
;******************************************************************************
;* Copyright (c) 2013-2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
//
// Libraries
//
#include <Library/PeiServicesLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/HobLib.h>
#include <Library/MtrrLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/Stage2HashLib.h>
#include <Library/PostCodeLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/TimerLib.h>
#include <PostCode.h>
#include <ChipsetSetupConfig.h>
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
#include <Ppi/EmuPei.h>
#endif
//[-end-161123-IB07250310-modify]//
#include <Library/CmosLib.h>
#include <FastRecoveryData.h>
#include <ScRegs/RegsPcu.h>
#include <ChipsetCmos.h>
#include <Guid/MtrrDataInfo.h>
#define ResetSystem        IoWrite8(R_PCH_RST_CNT, V_PCH_RST_CNT_FULLRESET)
#include <ScAccess.h>
#include <SaAccess.h>
#include <SeCChipset.h>
#include <TxeSecureBootRegs.h>
#include <Library/PciLib.h>
#include <Ppi/BootInTxeRecoveryMode.h>
#include <SaCommonDefinitions.h>
#ifndef BUNIT_BSMMRRL_OFFSET
#define  BUNIT_BSMMRRL_OFFSET      0x2E
#endif

#ifndef CHV_UNIT_BUNIT
#define  CHV_UNIT_BUNIT            0x03
#endif

#ifndef HeciPciRead32
#define HeciPciRead32(Register) MmioRead32 (MmPciAddress (0,SEC_BUS, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, Register))
#endif

EFI_PEI_PPI_DESCRIPTOR  mPpiListTxeRecoveryBootMode = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiBootInTxeRecoveryModePpiGuid,
  NULL
};

//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
BOOLEAN  mInRecoveryPei = FALSE;

VOID
FastCrisisRecoveryCheck (
  IN CONST EFI_PEI_SERVICES                 **PeiServices,
  OUT FAST_RECOVERY_DXE_TO_PEI_DATA         **EmuPeiPpi
  )
{
  EFI_STATUS     Status;
  
  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    Status = (**PeiServices).LocatePpi (PeiServices, &gEmuPeiPpiGuid, 0, NULL, (VOID **)EmuPeiPpi);    
    mInRecoveryPei = EFI_ERROR(Status) ? FALSE : TRUE;
  }
}
#endif
//[-end-161123-IB07250310-modify]//

/**
 Install Firmware Volume Hob's once there is main memory

 @param[in]         BootMode            Boot mode

 @retval            EFI_SUCCESS         EFI_SUCCESS if the interface could be successfully installed
*/
STATIC
EFI_STATUS
GetMemorySize (
  IN  CONST EFI_PEI_SERVICES    **PeiServices,
  OUT UINT64              *LowMemoryLength,
  OUT UINT64              *HighMemoryLength
  )
{
  EFI_STATUS              Status;
  EFI_PEI_HOB_POINTERS    Hob;

  *HighMemoryLength = 0;
  *LowMemoryLength = 0x100000;
  //
  // Get the HOB list for processing
  //
  Status = (*PeiServices)->GetHobList (PeiServices, (VOID **)&Hob.Raw);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Collect memory ranges
  //
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
      if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
        //
        // Need memory above 1MB to be collected here
        //
        if (Hob.ResourceDescriptor->PhysicalStart >= 0x100000 &&
            Hob.ResourceDescriptor->PhysicalStart < (EFI_PHYSICAL_ADDRESS) 0x100000000) {
          *LowMemoryLength += (UINT64) (Hob.ResourceDescriptor->ResourceLength);
        } else if (Hob.ResourceDescriptor->PhysicalStart >= (EFI_PHYSICAL_ADDRESS) 0x100000000) {
          *HighMemoryLength += (UINT64) (Hob.ResourceDescriptor->ResourceLength);
        }
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  return EFI_SUCCESS;
}

/**
 check if this MTRR is for BIOS ROM mapping memory

 @param[in]         UINT64            MtrrBase
 @param[in]         UINT64            MtrrMask

 @retval            BOOLEAN           TRUE if this MTRR is for BIOS ROM
*/
BOOLEAN
IsBiosRomMtrr (
  IN  UINT64              MtrrBase,
  IN  UINT64              MtrrMask
  )
{
  UINT64              CacheBase;
  UINT32              CacheSize;


  if ( MtrrBase == 0 || MtrrMask == 0 ){
    return FALSE;
  }
  //
  // it should be WriteProtected cache type
  //
  if ( (MtrrBase & CacheWriteProtected) != CacheWriteProtected ){
    return FALSE;
  
  } else {
    CacheBase =  (UINT64)( MtrrBase & MTRR_LIB_CACHE_VALID_ADDRESS );
    CacheSize = ~((UINT32) MtrrMask & MTRR_LIB_CACHE_VALID_ADDRESS ) + 1;    
    //
    // cache range should be : 1.Cache base  >= bios rom base address 
    //                         2.Cache size  <= bios rom size 
    //
    if  ( CacheBase >= (UINT64)FixedPcdGet32 (PcdFlashAreaBaseAddress) &&
          CacheSize <= (UINT32)FixedPcdGet32 (PcdFlashAreaSize)  ){
          return TRUE;       
    }
  } 
  
  return FALSE;   
}

VOID
RanOutMtrrSetting (
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN MTRR_SETTINGS           *MtrrSetting,
  IN UINT8                   NumberOfMtrrSetting,
  IN UINT32                  VariableMtrrLimit
  ) 
{
  UINT8                     Index1;
  UINT8                     Index2;
  HOB_MTRR_RESTORE_INFO     *MtrrRestoreInfoHob; 



  MtrrRestoreInfoHob = AllocatePool (sizeof (HOB_MTRR_RESTORE_INFO));                          
  if (MtrrRestoreInfoHob == NULL) {
    DEBUG ((EFI_D_ERROR, "AllocatePool Fail!!\n"));
    ASSERT(FALSE);
  } 

  //
  //  Clear the MTRR Info HOB buffer.
  //                        
  ZeroMem (MtrrRestoreInfoHob, sizeof (HOB_MTRR_RESTORE_INFO));
  //
  // Skip a MTRR setting for BIOS ROM  
  //
  for (Index1 = 0,Index2=0 ; Index1 < MTRR_MAX, Index2 < NumberOfMtrrSetting ;){
    if (IsBiosRomMtrr(MtrrSetting->Variables.Mtrr[Index2].Base,MtrrSetting->Variables.Mtrr[Index2].Mask)){
      Index2++;
      continue;
    }     
    MtrrRestoreInfoHob->MtrrData.MtrrBase[Index1] =   MtrrSetting->Variables.Mtrr[Index2].Base;
    MtrrRestoreInfoHob->MtrrData.MtrrSize[Index1] =   MtrrSetting->Variables.Mtrr[Index2].Mask; 

    Index1++;
    Index2++;
    
  }
  //
  // Assert code if it still run out for the maximum number of MTRR supported by the processor.
  //
  DEBUG ((EFI_D_INFO, "The new MTRR result :\n")); 
  for (Index1 = 0; Index1 < MTRR_MAX; Index1++) {
    if (MtrrRestoreInfoHob->MtrrData.MtrrBase[Index1] == 0){
  
        DEBUG ((EFI_D_INFO, "The usage of new MTRR = %d\n",Index1));       
        if ( Index1 > VariableMtrrLimit ){
          DEBUG ((EFI_D_ERROR, "Ran out of MTRR.\n"));
          ASSERT(FALSE);
          break;
        }        
        break;
    }
    DEBUG ((EFI_D_INFO, "Base=%lx, Mask=%lx\n",MtrrRestoreInfoHob->MtrrData.MtrrBase[Index1] ,MtrrRestoreInfoHob->MtrrData.MtrrSize[Index1]));
  }

  
  BuildGuidDataHob (&gMtrrDataInfoGuid, MtrrRestoreInfoHob, sizeof (HOB_MTRR_RESTORE_INFO));
  
  return ;
}

/**
 Install Firmware Volume Hob's once there is main memory

 @param[in]         BootMode            Boot mode

 @retval            EFI_SUCCESS         EFI_SUCCESS if the interface could be successfully installed
*/
EFI_STATUS
Stage1MemoryDiscoverCallback (
  IN  EFI_BOOT_MODE       *BootMode
  )
{
  UINT8                                 Index;
  MTRR_SETTINGS                         MtrrSetting;
  UINT64                                MemoryLength;
  UINT64                                MemOverflow;  
  UINT64                                MemoryLengthUc;
  UINT64                                MaxMemoryLength;
  UINT64                                LowMemoryLength;
  UINT64                                HighMemoryLength;
  UINT64                                MsrData;
  CONST EFI_PEI_SERVICES                **PeiServices;
  PHYSICAL_ADDRESS                      Stage2Address;
  UINTN                                 Stage2Size;
  EFI_STATUS                            Status;
#ifndef BIOS_2MB_BUILD
//   UINT8                                 *FdHashData;
//   PHYSICAL_ADDRESS                      Stage2HashAddress;
//   UINTN                                 Stage2HashSize;
//   UINT8                                 *HashData;
//   INTN                                  HashCompareResult; 
//   UINTN                                 FlashFvRecovery2Base;
//   UINTN                                 FlashFvBackupBase;
#endif
  FAST_RECOVERY_DXE_TO_PEI_DATA         *EmuPeiPpi;
  UINTN                                 *Stage2RomToRamPtr;
  UINTN                                 TargetFvMainBase;
  UINT8                                 MtrrConsume;
  UINT8                                 TempMtrrIndex;
  MTRR_SETTINGS                         TempMtrrSetting;                                 
  UINT32                                VariableMtrrLimit;
  UINT32                                Buffer32;
  UINT64                                BiosSize;
  UINT32                                TSegBase;

//[-start-160616-IB07220100-add]//
  MtrrConsume = 0;
//[-end-160616-IB07220100-add]//
  EmuPeiPpi= NULL;
  Buffer32 = 0;
  BiosSize = 0;
//  TSegBase = 0;
  DEBUG ((EFI_D_INFO, "Stage1MemCallback start\n"));
  
  PeiServices = GetPeiServicesTablePointer ();

//  Todo: find the method to get TSegBase
//   MsgBus32Read(CHV_UNIT_BUNIT, BUNIT_BSMMRRL_OFFSET, Buffer32);
//   TSegBase = (EFI_PHYSICAL_ADDRESS) (LShiftU64((Buffer32 & 0x0000FFFF), 20));
//   //
//   // should has Tseg 
//   //
//   ASSERT( TSegBase !=0 );

  TargetFvMainBase = (UINTN)PcdGet32(PcdFlashFvMainBase);
  Stage2Address = 0;
  Stage2Size = 0;
  Stage2RomToRamPtr = NULL;
  //
  // Variable initialization
  //
  LowMemoryLength = 0;
  HighMemoryLength = 0;
  MemoryLengthUc = 0;

  //
  // Determine memory usage
  //
  GetMemorySize (
    PeiServices,
    &LowMemoryLength,
    &HighMemoryLength
    );

  
  //
  // SC_PO, A0 PO Workaround - The algorithm is not determining memory properly
  // W/A forces low memory to TSEG base to match memory config.
  //
  TSegBase = (UINT32)MmPciAddress (
                      0,
                      0, //SA_MC_BUS,
                      0, //SA_MC_DEV,
                      0, //SA_MC_FUN,
                      0xb8 //R_SA_TSEG
                    );
  LowMemoryLength = (UINT64)(MmioRead32 (TSegBase) & 0xFFFFFFF0);

  MaxMemoryLength = LowMemoryLength;
  // Round up to nearest 256MB
  MemOverflow = (LowMemoryLength & 0x0fffffff);
  if (MemOverflow != 0) {
    MaxMemoryLength = LowMemoryLength + (0x10000000 - MemOverflow);
  }  

  ZeroMem (&MtrrSetting, sizeof(MTRR_SETTINGS));
    
  for (Index = 0; Index < 2; Index++){
    MtrrSetting.Fixed.Mtrr[Index]=0x0606060606060606;
   }
  
  for (Index = 2; Index < 11; Index++) {
    MtrrSetting.Fixed.Mtrr[Index]=0x0505050505050505;
   }
    
  //
  // Cache the flash area to improve the boot performance in PEI phase
  //
  //  
  // If fTPM is Enable the MTRR settings will not be enough, modify BIOS cache size form 0x00300000 (3M) to 0x00400000 (4M) for providing extra MTRR of MMIO.
  //
  Index = 0;  
  for ( MemOverflow = (UINT64)FixedPcdGet32 (PcdFlashAreaBaseAddress) + (UINT64)FixedPcdGet32 (PcdFlashAreaSize), MemoryLengthUc = (UINT64)FixedPcdGet32 (PcdFlashAreaSize);   
        MemoryLengthUc != 0;
        MemoryLengthUc -= MemoryLength, MemOverflow -= MemoryLength, Index++) { 
    MemoryLength = GetPowerOfTwo64(MemoryLengthUc);
    MtrrSetting.Variables.Mtrr[Index].Base = ((MemOverflow - MemoryLength) | CacheWriteProtected);  
    MtrrSetting.Variables.Mtrr[Index].Mask = (((~(MemoryLength - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED);
    DEBUG ((EFI_D_INFO, "%d Base=%016lx, Mask=%016lx\n", Index, MtrrSetting.Variables.Mtrr[Index].Base, MtrrSetting.Variables.Mtrr[Index].Mask));               
  } 

  CopyMem(&TempMtrrSetting,&MtrrSetting,sizeof(MTRR_SETTINGS));
  TempMtrrIndex = Index;
  //
  // use two method to set the cachce for the below 4G memory  
  //  
  DEBUG ((EFI_D_INFO, "use mothod 1 \n"));
   //Index = 1;

  MemOverflow =0;
  
  while (LowMemoryLength > MemOverflow){
    MtrrSetting.Variables.Mtrr[Index].Base = (MemOverflow & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheWriteBack;  
    MemoryLength = LowMemoryLength - MemOverflow;
    MemoryLength = GetPowerOfTwo64 (MemoryLength);
    MtrrSetting.Variables.Mtrr[Index].Mask = ((~(MemoryLength - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
    DEBUG ((EFI_D_INFO, "Base=%lx, Mask=%lx\n",MtrrSetting.Variables.Mtrr[Index].Base ,MtrrSetting.Variables.Mtrr[Index].Mask));
    
    MemOverflow += MemoryLength;
    Index++;
  }

  DEBUG ((EFI_D_INFO, "test mothod 2 \n"));
  MemOverflow = 0;
  while (MaxMemoryLength > MemOverflow){
    TempMtrrSetting.Variables.Mtrr[TempMtrrIndex].Base = (MemOverflow & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheWriteBack;  
    MemoryLength = MaxMemoryLength - MemOverflow;
    MemoryLength = GetPowerOfTwo64 (MemoryLength);
    TempMtrrSetting.Variables.Mtrr[TempMtrrIndex].Mask = ((~(MemoryLength - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
    DEBUG ((EFI_D_INFO, "TempMtrr--Base=%lx, Mask=%lx\n",TempMtrrSetting.Variables.Mtrr[TempMtrrIndex].Base ,TempMtrrSetting.Variables.Mtrr[TempMtrrIndex].Mask));
      
    MemOverflow += MemoryLength;
    TempMtrrIndex++;
  }

    MemoryLength = LowMemoryLength;

    while (MaxMemoryLength != MemoryLength) {
      MemoryLengthUc = GetPowerOfTwo64 (MaxMemoryLength - MemoryLength);
      TempMtrrSetting.Variables.Mtrr[TempMtrrIndex].Base = ((MaxMemoryLength - MemoryLengthUc) & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheUncacheable;
      TempMtrrSetting.Variables.Mtrr[TempMtrrIndex].Mask= ((~(MemoryLengthUc   - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
      DEBUG ((EFI_D_INFO, "TempMtrr--Base=%lx, Mask=%lx\n",TempMtrrSetting.Variables.Mtrr[TempMtrrIndex].Base ,TempMtrrSetting.Variables.Mtrr[TempMtrrIndex].Mask));
      MaxMemoryLength -= MemoryLengthUc;
      TempMtrrIndex++;
    }
  //
  // if method 2 is better, than use it instead.
  //
  if ( TempMtrrIndex < Index ){
    DEBUG ((EFI_D_INFO, "use method 2 instead\n"));
    CopyMem(&MtrrSetting,&TempMtrrSetting,sizeof(MTRR_SETTINGS));
    Index = TempMtrrIndex;
  }

  MemOverflow = 0x100000000;
  while (HighMemoryLength > 0) {
    MtrrSetting.Variables.Mtrr[Index].Base = (MemOverflow & MTRR_LIB_CACHE_VALID_ADDRESS) | CacheWriteBack;  
    MemoryLength = HighMemoryLength;
    MemoryLength = GetPowerOfTwo64 (MemoryLength);

    if (MemoryLength > MemOverflow){
      MemoryLength = MemOverflow;  //Cap at Max 4G
    }

    MtrrSetting.Variables.Mtrr[Index].Mask = ((~(MemoryLength - 1)) & MTRR_LIB_CACHE_VALID_ADDRESS) | MTRR_LIB_CACHE_MTRR_ENABLED;
    
    MemOverflow += MemoryLength;
    HighMemoryLength -= MemoryLength;
    Index++;
  }
  DEBUG ((EFI_D_INFO, "Final MTRR setting : \n"));
  for (Index = 0; Index < MTRR_NUMBER_OF_VARIABLE_MTRR; Index++) {
    if (MtrrSetting.Variables.Mtrr[Index].Base == 0){
        MtrrConsume = Index;
        break;
    }
    DEBUG ((EFI_D_INFO, "Base=%lx, Mask=%lx\n",MtrrSetting.Variables.Mtrr[Index].Base ,MtrrSetting.Variables.Mtrr[Index].Mask));
  }
  //
  // Get the maximum number of MTRR supported by the processor.
  //
 
  VariableMtrrLimit = (UINT32) (AsmReadMsr64 (MTRR_LIB_IA32_MTRR_CAP) & 0xFF);
  DEBUG ( (EFI_D_INFO | EFI_D_ERROR, "MtrrConsume : %d, VariableMtrrLimit=%d\n",MtrrConsume , VariableMtrrLimit) );
  //
  // Check if MTRR is ran out
  //
  Status = EFI_SUCCESS;
  if (MtrrConsume > VariableMtrrLimit) {
    DEBUG ((EFI_D_INFO | EFI_D_ERROR, "Ran out of MTRR - No MTRR reserve for DXE.\n"));
    Status = EFI_OUT_OF_RESOURCES;
  }

  if (EFI_ERROR (Status)) {
    //
    // Re-program MTRR settings during DXE phase if MTRR registers ran out.
    //
    RanOutMtrrSetting (PeiServices,&MtrrSetting,MtrrConsume,VariableMtrrLimit);
  }
  //
  // set FE/E bits for IA32_MTRR_DEF_TYPE
  //
  MtrrSetting.MtrrDefType |=  3 << 10;
  
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
  FastCrisisRecoveryCheck (PeiServices, &EmuPeiPpi);
#endif
//[-end-161123-IB07250310-modify]//
  
  if (EmuPeiPpi == NULL) {

    //
    // Disable No-Eviction Mode
    //
    MsrData = AsmReadMsr64 (0x2E0);
    MsrData &= ~BIT1;
    AsmWriteMsr64 (0x2E0, MsrData);
    
    MsrData = AsmReadMsr64 (0x2E0);
    MsrData &= ~BIT0;
    AsmWriteMsr64 (0x2E0, MsrData);
  
    AsmInvd ();   
  }

  MtrrSetAllMtrrs(&MtrrSetting);

  // code needed for PUNIT
  if (PLATFORM_ID != VALUE_INNOVATOR_HVP) {
    if (MmioRead32 (MCH_BASE_ADDRESS + 0x7078) == 0xffffffff) {
      //
      // punit not found - skip this flow
      //
      DEBUG ((EFI_D_ERROR, "Warning!! -- Punit MMIO not available - Skipping set RST_CPL.\n"));
    } else {
      //
      // Need IAFW to indicate BIOS done to Pcode to complete Phase 7 of Punit bring up
      // Note: If punit patch is loaded, the issue with MCH mmio regs getting erased here is resovled
      //
      DEBUG ((EFI_D_INFO, "Setting Pcode RST_CPL (0x%x = 0x%x)\n", (MCH_BASE_ADDRESS + 0x7078), MmioRead32(MCH_BASE_ADDRESS + 0x7078) ));
      MmioOr32 ((MCH_BASE_ADDRESS + 0x7078), BIT0);
      //
      // Poll for bit 8 in same reg (RST_CPL). It will be set once pcode observes bit 0 being set.
      //
      DEBUG ((EFI_D_INFO, "Waiting for Pcode to acknowledge RST_CPL being set...\n"));
      while ( !(MmioRead32 (MCH_BASE_ADDRESS + 0x7078) & BIT8) ) {
        MicroSecondDelay (100);
      }
      //Set the "MemValid" bit in the SRAM which will be used to indicate memory contents need to be preserved to the MRC
      // on warm/cold reset and S3.
      MmioOr32 ( (PcdGet32 ( PcdPmcSsramBaseAddress0 ) + 0x1410), BIT0 );
    }
  }


  return EFI_SUCCESS;
}
