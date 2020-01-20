//;******************************************************************************
//;* Copyright (c) 1983-2014, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//; 
//; 
//; 
//;------------------------------------------------------------------------------
//; 
//; Abstract: Provide some misc subfunctions
//;
#include <Library/BaseMemoryLib.h>
#include <ScAccess.h>
#include "BiosRegionLockInfo.h"
#include "BiosRegionLockHelpFun.h"

#define B_SPI_PRB_MASK        0x00001FFF

//EFI_STATUS
//GetBiosRegionAddress (
//  IN  BIOS_REGION_TYPE   Type,
//  OUT UINTN             *Base,
//  OUT UINTN             *Length
//)
//{
//  EFI_STATUS Status = EFI_SUCCESS;
//  
//  switch (Type) {
//  case FVMAIN:
//    *Base   = FLASH_REGION_FVMAIN_BASE;
//    *Length = FLASH_REGION_FVMAIN_SIZE;
//    break;
//  case NV_COMMON_STORE:
//    *Base   = FLASH_REGION_NV_COMMON_STORE_BASE;
//    *Length = FLASH_REGION_NV_COMMON_STORE_SIZE;
//    break;
//  case NV_COMMON_STORE_MICROCODE:
//    *Base   = FLASH_REGION_NV_COMMON_STORE_SUBREGION_MICROCODE_BASE;
//    *Length = FLASH_REGION_NV_COMMON_STORE_SUBREGION_MICROCODE_SIZE;
//    break;
//  case NV_COMMON_STORE_OEM_DMI_STORE:
//    *Base   = FLASH_REGION_NV_COMMON_STORE_SUBREGION_OEM_DMI_STORE_BASE;
//    *Length = FLASH_REGION_NV_COMMON_STORE_SUBREGION_OEM_DMI_STORE_SIZE;
//    break;
//  case NV_COMMON_STORE_RESERVED1:
//    *Base   = FLASH_REGION_NV_COMMON_STORE_SUBREGION_RESERVED1_BASE;
//    *Length = FLASH_REGION_NV_COMMON_STORE_SUBREGION_RESERVED1_SIZE;
//    break;
//  case NV_COMMON_STORE_NV_BVDT:
//    *Base   = FLASH_REGION_NV_COMMON_STORE_SUBREGION_NV_BVDT_BASE;
//    *Length = FLASH_REGION_NV_COMMON_STORE_SUBREGION_NV_BVDT_SIZE;
//    break;
//  case NV_COMMON_STORE_RESERVED2:
//    *Base   = FLASH_REGION_NV_COMMON_STORE_SUBREGION_RESERVED2_BASE;
//    *Length = FLASH_REGION_NV_COMMON_STORE_SUBREGION_RESERVED2_SIZE;
//    break;
//  case NVSTORAGE:
//    *Base   = FLASH_REGION_NVSTORAGE_BASE;
//    *Length = FLASH_REGION_NVSTORAGE_SIZE;
//    break;
//  case NVSTORAGE_NV_VARIABLE_STORE:
//    *Base   = FLASH_REGION_NVSTORAGE_SUBREGION_NV_VARIABLE_STORE_BASE;
//    *Length = FLASH_REGION_NVSTORAGE_SUBREGION_NV_VARIABLE_STORE_SIZE;
//    break;
//  case NVSTORAGE_NV_FTW_WORKING:
//    *Base   = FLASH_REGION_NVSTORAGE_SUBREGION_NV_FTW_WORKING_BASE;
//    *Length = FLASH_REGION_NVSTORAGE_SUBREGION_NV_FTW_WORKING_SIZE;
//    break;
//  case NVSTORAGE_NV_FTW_SPARE:
//    *Base   = FLASH_REGION_NVSTORAGE_SUBREGION_NV_FTW_SPARE_BASE;
//    *Length = FLASH_REGION_NVSTORAGE_SUBREGION_NV_FTW_SPARE_SIZE;
//    break;
//  case FV_RECOVERY:
//    *Base   = FLASH_REGION_FV_RECOVERY_BASE;
//    *Length = FLASH_REGION_FV_RECOVERY_SIZE;
//    break;
//  default:
//    Status = EFI_INVALID_PARAMETER;
//    break;
//  }
//
//  return Status;
//}

STATIC
VOID
Sort (
  IN OUT BIOS_REGION_LOCK_BASE *Array,
  IN     UINTN                  Length
  )
/*++

Routine Description:

  This function sort the BIOS regions described in BiosLock record array.

Arguments:

  Array    - Private protected BIOS region record.
  Length   - The length of input Array

Returns:

  None

--*/
{
  UINTN Index1;
  UINTN Index2;
  UINTN Swap;
  
  for (Index1 = 0; Index1 < Length; ++Index1) {
    for (Index2 = Index1 + 1; Index2 <= Length; ++Index2) {
      if (Array[Index1].Base < Array[Index2].Base) {
        Swap = Array[Index1].Base;
        Array[Index1].Base = Array[Index2].Base;
        Array[Index2].Base = Swap;
        
        Swap = Array[Index1].Length;
        Array[Index1].Length = Array[Index2].Length;
        Array[Index2].Length = Swap;  
      }
    }
  }
}

EFI_STATUS
MergeToBase (
  IN BIOS_REGION_LOCK_BASE  *BiosLock,
  IN UINTN                   Base,
  IN UINTN                   Length
  )
/*++

Routine Description:

  This function merges the requested BIOS region to the private protected BIOS region record.

Arguments:

  BiosLock    - Private protected BIOS region record.
  BaseAddress - The start address of the BIOS region which need to be merged.
  Length      - The Length of the BIOS region which need to be merged.

Returns:

  EFI_OUT_OF_RESOURCES   - The max number of BIOS protect regions have been reached and the requested region 
                           can not be merge to existing protected region.
  EFI_SUCCESS            - Merge successfully

--*/
{
  INTN                  Index1;
  INTN                  Index2;
  UINTN                 Top1;
  UINTN                 Top2;
  BIOS_REGION_LOCK_BASE LockTemp[MAX_BIOS_REGION_LOCK + 1];

  ZeroMem ((VOID *)LockTemp, sizeof(BIOS_REGION_LOCK_BASE) * (MAX_BIOS_REGION_LOCK + 1));
  CopyMem ((VOID *)LockTemp, (VOID *)BiosLock, sizeof(BIOS_REGION_LOCK_BASE) * MAX_BIOS_REGION_LOCK);

  for (Index1 = 0; Index1 <= MAX_BIOS_REGION_LOCK; ++Index1) {
    if (LockTemp[Index1].Base == 0) {
      LockTemp[Index1].Base   = Base;
      LockTemp[Index1].Length = Length;
      break;
    } 
  }
  
#ifdef EFI_DEBUG
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "\nMergeToBase\n"));
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "Array Status (Before Sort)....\n"));
  for (Index1 = 0; Index1 <= MAX_BIOS_REGION_LOCK; ++Index1) {
    Top1 = LockTemp[Index1].Base + LockTemp[Index1].Length - 1;
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "%d => Base:%8x  Length:%8x Top:%8x\n", Index1 + 1,  LockTemp[Index1].Base, LockTemp[Index1].Length, Top1) );
  }
#endif

  Sort (LockTemp, MAX_BIOS_REGION_LOCK);

#ifdef EFI_DEBUG
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "\nArray Status (After Sort)....\n"));
  for (Index1 = 0; Index1 <= MAX_BIOS_REGION_LOCK; ++Index1) {
    Top1 = LockTemp[Index1].Base + LockTemp[Index1].Length - 1;
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "%d => Base:%8x  Length:%8x Top:%8x\n", Index1 + 1,  LockTemp[Index1].Base, LockTemp[Index1].Length, Top1) );
  }
#endif

  for (Index1 = MAX_BIOS_REGION_LOCK - 1; Index1 >= 0; --Index1) {
    Index2 = Index1 + 1;
    if (LockTemp[Index2].Base == 0) {
      continue;
    }
    
    Top1 = LockTemp[Index1].Base + LockTemp[Index1].Length - 1;
    Top2 = LockTemp[Index2].Base + LockTemp[Index2].Length - 1;
    
    if (LockTemp[Index2].Base == LockTemp[Index1].Base) {
      if (LockTemp[Index2].Length > LockTemp[Index1].Length) {
        LockTemp[Index1].Length = LockTemp[Index2].Length;
      }
      
      LockTemp[Index2].Base    = 0;
      LockTemp[Index2].Length  = 0;
    } else if ((LockTemp[Index1].Base - 1) <= Top2) {
      if (Top2 > Top1) {
        LockTemp[Index1].Base   = LockTemp[Index2].Base;
        LockTemp[Index1].Length = LockTemp[Index2].Length;
      } else {
        LockTemp[Index1].Base   = LockTemp[Index2].Base;
        LockTemp[Index1].Length = Top1 - LockTemp[Index2].Base + 1;
      }
      
      LockTemp[Index2].Base   = 0;
      LockTemp[Index2].Length = 0;
    }
  }

  Sort (LockTemp, MAX_BIOS_REGION_LOCK);

#ifdef EFI_DEBUG
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "\nArray Status (After Merge)....\n"));
  for (Index1 = 0; Index1 <= MAX_BIOS_REGION_LOCK; ++Index1) {
    Top1 = LockTemp[Index1].Base + LockTemp[Index1].Length - 1;
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "%d => Base:%8x  Length:%8x Top:%8x\n", Index1 + 1,  LockTemp[Index1].Base, LockTemp[Index1].Length, Top1) );
  }
#endif

  if (LockTemp[MAX_BIOS_REGION_LOCK].Base != 0) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem ((VOID *)BiosLock, (VOID *)LockTemp, sizeof(BIOS_REGION_LOCK_BASE) * MAX_BIOS_REGION_LOCK);

  return EFI_SUCCESS;
}

EFI_STATUS
RemoveFromBase (
  IN BIOS_REGION_LOCK_BASE  *BiosLock,
  IN UINTN                   Base,
  IN UINTN                   Length
  )
/*++

Routine Description:

  This function removes the requested BIOS region from the private protected BIOS region record.

Arguments:

  BiosLock    - Private protected BIOS region record.
  BaseAddress - The start address of the BIOS region which need to be removed.
  Length      - The Length of the BIOS region which need to be removed.

Returns:

  EFI_OUT_OF_RESOURCES   - The BIOS protect region registers are not enough to set for all discontinuous BIOS region
  EFI_SUCCESS            - Remove successfully

--*/
{
  UINTN                 Index;
  UINTN                 Top1;
  UINTN                 Top2;
  BIOS_REGION_LOCK_BASE LockTemp[MAX_BIOS_REGION_LOCK + 1];

  CopyMem ((VOID *)LockTemp, (VOID *)BiosLock, sizeof(BIOS_REGION_LOCK_BASE) * MAX_BIOS_REGION_LOCK);

#ifdef EFI_DEBUG
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "\nRemoveFromBase\n"));
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "Array Status (Before Remove)....\n"));
  for (Index = 0; Index <= MAX_BIOS_REGION_LOCK; ++Index) {
    Top1 = LockTemp[Index].Base + LockTemp[Index].Length - 1;
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "%d => Base:%8x  Length:%8x Top:%8x\n", Index + 1,  LockTemp[Index].Base, LockTemp[Index].Length, Top1) );
  }
#endif

  for (Index = 0; Index < MAX_BIOS_REGION_LOCK; ++Index) {
    Top1 = LockTemp[Index].Base + LockTemp[Index].Length - 1;
    Top2 = Base + Length - 1;

    if (Top1 <= (Base - 1) || Top2 <= (LockTemp[Index].Base - 1)) {
      continue;
    }
    
    if (LockTemp[Index].Base >= Base) {
      if (Top1 <= Top2) {
        LockTemp[Index].Base   = 0;
        LockTemp[Index].Length = 0;

        if (Top1 == Top2) {
          break;
        }
      } else {
        LockTemp[Index].Base = Top2 + 1;
        LockTemp[Index].Length = Top1 - Top2;
      }
    } else {
      LockTemp[Index].Length = Base - LockTemp[Index].Base;
      if (Top1 == Top2) {
        break;
      } else if (Top1 > Top2) {
        LockTemp[MAX_BIOS_REGION_LOCK].Base   = Top2 + 1;
        LockTemp[MAX_BIOS_REGION_LOCK].Length = Top1 - Top2;
      }
    }
  }

#ifdef EFI_DEBUG
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "\nArray Status (After Remove)....\n"));
  for (Index = 0; Index <= MAX_BIOS_REGION_LOCK; ++Index) {
    Top1 = LockTemp[Index].Base + LockTemp[Index].Length - 1;
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "%d => Base:%8x  Length:%8x Top:%8x\n", Index + 1,  LockTemp[Index].Base, LockTemp[Index].Length, Top1) );
  }
#endif

  Sort (LockTemp, MAX_BIOS_REGION_LOCK);

#ifdef EFI_DEBUG
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "\nArray Status (After Sort)....\n"));
  for (Index = 0; Index <= MAX_BIOS_REGION_LOCK; ++Index) {
    Top1 = LockTemp[Index].Base + LockTemp[Index].Length - 1;
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "%d => Base:%8x  Length:%8x Top:%8x\n", Index + 1,  LockTemp[Index].Base, LockTemp[Index].Length, Top1) );
  }
#endif

  if (LockTemp[MAX_BIOS_REGION_LOCK].Base != 0) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem ((VOID *)BiosLock, (VOID *)LockTemp, sizeof(BIOS_REGION_LOCK_BASE) * MAX_BIOS_REGION_LOCK);

  return EFI_SUCCESS;
}

EFI_STATUS
ProgramRegister (
  IN BIOS_REGION_LOCK_BASE *BiosLock
  )
/*++

Routine Description:

  This function programs the BIOS regions described in BiosLock record to the BIOS protect region registers.

Arguments:

  BiosLock    - Private protected BIOS region record.

Returns:

  EFI_SUCCESS

--*/
{
  UINTN      Index;
  UINT32     PRRegister;
  UINTN      RangeBase;
  UINTN      RangeLimit;
  UINTN      Offset;
  UINTN      BiosRegionStart;
  EFI_STATUS Status;
  
  BiosRegionStart = (MmSpi32 (R_SPI_BFPR) & B_SPI_PRB_MASK) << 12;
  Offset          = FixedPcdGet32(PcdFlashAreaBaseAddress) - BiosRegionStart;
  
#ifdef EFI_DEBUG
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "\nBIOS Region Lock Base ........\n" ) );
#endif

  //
  // Try to merge content in range registers.
  // 
  for (Index = 0; Index < MAX_BIOS_REGION_LOCK; ++Index) {  
    PRRegister = MmSpi32 (R_SPI_PR0 + (Index << 2));
    if (PRRegister != 0) {
      RangeBase  = ((PRRegister & B_SPI_PR0_PRB_MASK) << 12) + Offset;
      RangeLimit = ((PRRegister & B_SPI_PR0_PRL_MASK) >> 4) + Offset;
      Status = MergeToBase(
                  BiosLock,
                  RangeBase,
                  RangeLimit - RangeBase + 0x1000 
                  );
      if (EFI_ERROR(Status)) {
        return Status;
      }
    }
  }

  for (Index = 0; Index < MAX_BIOS_REGION_LOCK; ++Index) {
    PRRegister = 0;
    
    if (BiosLock[Index].Base != 0) {
      RangeBase  = BiosLock[Index].Base - Offset;
      RangeLimit = RangeBase + BiosLock[Index].Length - 1;
      DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "%d => Base:%8x  Length:%8x BiosBase:%8x Limit:%0x\n", Index,  BiosLock[Index].Base, BiosLock[Index].Length, RangeBase, RangeLimit) );
      
      RangeBase  = (RangeBase >> 12) & B_SPI_PR0_PRB_MASK;
      RangeLimit = (RangeLimit << 4) & B_SPI_PR0_PRL_MASK;
      PRRegister = (UINT32)(RangeBase | RangeLimit | B_SPI_PR0_WPE);
    }

    MmSpi32 (R_SPI_PR0 + (Index << 2)) = PRRegister;
    
#ifdef EFI_DEBUG
    PRRegister = MmSpi32 (R_SPI_PR0 + (Index << 2));
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "     Register:%8x\n", PRRegister));
#endif
  }

  return EFI_SUCCESS;
}
