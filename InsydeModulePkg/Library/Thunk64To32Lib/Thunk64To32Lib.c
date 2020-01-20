 /** @file
  IA-32, x64 specifc functionality for Thunk 32 To 64 Library

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/PciLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>

#define CPU_STACK_SIZE                            0x2000
#define PEI_SERVICES_ADDR                         0x30000

#define IA32_PG_P                                 BIT0
#define IA32_PG_RW                                BIT1
#define IA32_PG_PS                                BIT7

#pragma pack(1)

typedef struct {
  UINT16  Limit;
  UINT64  Base;
} DESCRIPTOR64;

typedef struct {
  UINT16  Limit;
  UINT32  Base;
} DESCRIPTOR32;

typedef struct {
  DESCRIPTOR64                      x64GdtDesc;          // Protected mode GDT
  DESCRIPTOR64                      x64IdtDesc;          // Protected mode IDT
  UINT64                            x64Ss;
  UINT64                            x64Esp;
  UINT64                            ia32Stack;
  DESCRIPTOR32                      ia32IdtDesc;
  DESCRIPTOR32                      ia32GdtDesc;
  UINT16                            CodeSeg32offset;
} MEMORY_THUNK;

typedef union {
  struct {
    UINT32  LimitLow    : 16;
    UINT32  BaseLow     : 16;
    UINT32  BaseMid     : 8;
    UINT32  Type        : 4;
    UINT32  System      : 1;
    UINT32  Dpl         : 2;
    UINT32  Present     : 1;
    UINT32  LimitHigh   : 4;
    UINT32  Software    : 1;
    UINT32  Reserved    : 1;
    UINT32  DefaultSize : 1;
    UINT32  Granularity : 1;
    UINT32  BaseHigh    : 8;
  } Bits;
  UINT64  Uint64;
} IA32_GDT;
#pragma pack()

extern UINT32                mIA32Cr3;
STATIC MEMORY_THUNK          *mThunkInit           = NULL;
STATIC UINT64                *mStack               = NULL;
STATIC BOOLEAN               mConstructorProcessed = FALSE;

VOID
InternalThunk64To32 (
  IN MEMORY_THUNK  *IntThunk,
  IN UINT32        FunctionPoint,
  IN UINT32        PeiServicesPoint
  );

/**
  Create 4G PageTable in SMRAM.

  @return         PageTable Address

**/
UINT32
Gen4GPageTable (
  VOID
  )
{
  VOID    *PageTable;
  UINTN   Index;
  UINT64  *Pte;


  PageTable = AllocatePages (5);
  ASSERT (PageTable != NULL);
  if (PageTable == NULL) {
    return 0;
  }
  Pte = (UINT64*)PageTable;

  //
  // Zero out all page table entries first
  //
  ZeroMem (Pte, EFI_PAGES_TO_SIZE (1));

  //
  // Set Page Directory Pointers
  //
  for (Index = 0; Index < 4; Index++) {
    Pte[Index] = (UINTN)PageTable + EFI_PAGE_SIZE * (Index + 1) + IA32_PG_P;
  }
  Pte += EFI_PAGE_SIZE / sizeof (*Pte);

  //
  // Fill in Page Directory Entries
  //
  for (Index = 0; Index < EFI_PAGE_SIZE * 4 / sizeof (*Pte); Index++) {
    Pte[Index] = (Index << 21) + IA32_PG_PS + IA32_PG_RW + IA32_PG_P;
  }

  return (UINT32)(UINTN)PageTable;
}

/**
  The constructor function caches the PCI Express Base Address and creates a
  Set Virtual Address Map event to convert physical address to virtual addresses.

  @param[in]  ImageHandle   The firmware allocated handle for the EFI image.
  @param[in]  SystemTable   A pointer to the EFI System Table.

  @retval EFI_SUCCESS   The constructor completed successfully.
  @retval Other value   The constructor did not complete successfully.

**/
EFI_STATUS
EFIAPI
Thunk64To32LibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                Status;

  //
  // Init thunk 64 to 32 info.
  //
  mIA32Cr3 = Gen4GPageTable ();

  Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    CPU_STACK_SIZE,
                    (VOID **)&mStack
                    );
  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  mThunkInit = AllocatePool (sizeof(MEMORY_THUNK));
  ASSERT (mThunkInit != NULL);
  if (mThunkInit == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem(mStack, CPU_STACK_SIZE);
  ZeroMem(mThunkInit, sizeof(MEMORY_THUNK));
  mThunkInit->ia32Stack = (UINT64)(UINT64 *)(mStack + CPU_STACK_SIZE);
  mConstructorProcessed = TRUE;
  return EFI_SUCCESS;
}

/**
  Do thunk 64 to 32 and jmp to run code.

  @param [in]   PeiServicesPoint   Pei Services Point
  @param [in]   FunctionPoint      Function Point

  @retval EFI_SUCCESS     The thunk completed successfully.
  @retval EFI_NOT_READY   The thunk constructor function has not been performed

**/
EFI_STATUS
EFIAPI
Thunk64To32 (
  IN UINT32        PeiServicesPoint,
  IN UINT32        FunctionPoint
  )
{
  IA32_DESCRIPTOR                           Gdtr;
  UINTN                                     Index;
  IA32_GDT                                  *GdtrTable;
  UINT16                                    CodeSeg32offset;
  //
  // Check mThunkInit is already initialized
  //
  if (!mConstructorProcessed) {
    ASSERT (mConstructorProcessed == TRUE);
    return EFI_NOT_READY;
  }

  //
  // Get CodeSeg32 offset from Gdt Table.
  //
  CodeSeg32offset = 0;
  AsmReadGdtr (&Gdtr);
  GdtrTable = (IA32_GDT *)Gdtr.Base;
  for (Index = 0; Index < ((Gdtr.Limit + 1)/sizeof (IA32_GDT)); Index ++) {
    if (
      (GdtrTable [Index].Bits.Type == 0xa) ||
      (GdtrTable [Index].Bits.Type == 0xb) &&
      (GdtrTable [Index].Bits.Software == 0x0) &&
      (GdtrTable [Index].Bits.Reserved == 0x0) &&
      (GdtrTable [Index].Bits.DefaultSize == 0x01) &&
      (GdtrTable [Index].Bits.Granularity == 0x01)
      ) {
      CodeSeg32offset = (UINT16)Index * sizeof (IA32_GDT);
      break;
    }
  }
  mThunkInit->CodeSeg32offset = CodeSeg32offset;

  InternalThunk64To32 (mThunkInit, FunctionPoint, PeiServicesPoint);

  return EFI_SUCCESS;
}