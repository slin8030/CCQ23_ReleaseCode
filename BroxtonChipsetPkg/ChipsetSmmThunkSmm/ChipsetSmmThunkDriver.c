/** @file
  Chipset SMM Thunk Driver
  Revision History

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <ChipsetSmmThunkDriver.h>

BOOLEAN                       SkipSoftEnDisAPIC = FALSE;

SMM_THUNK_PRIVATE_DATA        mPrivateData = {
  EFI_SMM_THUNK_SIGNATURE,
  NULL,
  {
    SmmFarCall86,
    NULL,
    SmmInt86
  },
  0,
  0,
  0
};

EFI_STATUS
RestoreSMRAMCAttribute (
  );

EFI_STATUS
DisableSMRAMCAttribute (
  );

#ifndef EFI_APIC_GLOBAL_ENABLE
#define EFI_APIC_GLOBAL_ENABLE                0x800
#endif

#ifndef APIC_REGISTER_TPR_OFFSET
#define APIC_REGISTER_TPR_OFFSET              0x000000080
#endif
#ifndef APIC_REGISTER_SPURIOUS_VECTOR_OFFSET
#define APIC_REGISTER_SPURIOUS_VECTOR_OFFSET  0x0000000F0
#endif
#ifndef APIC_REGISTER_TIMER_VECTOR_OFFSET
#define APIC_REGISTER_TIMER_VECTOR_OFFSET     0x000000320
#endif

#define EXTENDED_APIC_BASE_MSR                0x800
#ifndef MSR_EXT_XAPIC_TPR
#define MSR_EXT_XAPIC_TPR                     0x808
#endif
#ifndef MSR_EXT_XAPIC_LVT_TIMER
#define MSR_EXT_XAPIC_LVT_TIMER               0x00000832
#endif

UINT32 ApicTPR;
UINT32 ApicLVT[6];
UINT8 SavedSmramReg;
BOOLEAN x2ApicEnabled = FALSE;
UINT32 x2ApicTPR;
UINT32 x2ApicLVT[6];


STATIC
VOID
DisableAPIC (
  );

STATIC
VOID
EnableAPIC (
  );
EFI_PHYSICAL_ADDRESS
GetBufferfromEBDA(
  IN UINTN      LegacyRegionSize
  );

VOID
ConvertRegisterBack (
  OUT EFI_IA32_REGISTER_SET           *Regs,
  IN  IA32_REGISTER_SET               *Register
  );



/**
  Waits for at least the given number of microseconds.

 @param[in]         Microseconds  Desired length of time to wait

 @retval EFI_SUCCESS   If the desired amount of time passed.

--*/
VOID
Stall(
  IN UINTN              Microseconds
  )
{
  UINTN                 Ticks;
  UINTN                 Counts;
  UINT32                CurrentTick;
  UINT32                OriginalTick;
  UINT64                RemainingTick;
  UINT16                mAcpiBaseAddr;

  mAcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

//[-start-180813-IB07400997-modify]//
  OriginalTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//

  CurrentTick = OriginalTick;

  //
  //The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;
  Counts = Ticks / V_ACPI_PM1_TMR_MAX_VAL;  //The loops needed by timer overflow
  RemainingTick = Ticks % V_ACPI_PM1_TMR_MAX_VAL;  //remaining clocks within one loop

  //
  //not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  //one I/O operation, and maybe generate SMI
  //

  while (Counts != 0) {
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
    if (CurrentTick < OriginalTick) {
//[-end-180813-IB07400997-modify]//
      Counts --;
    }
    OriginalTick = CurrentTick;
  }

  while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick) ) {
    OriginalTick = CurrentTick;
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//
  }
}

/**
 Initializes the SMM Thunk Driver

 @param [in]   ImageHandle      Handle for the image of this driver
 @param [in]   SystemTable      Pointer to the EFI System Table

 @retval EFI_SUCCESS            SMM thunk driver has been initialized successfully
 @return Other        SMM thunk driver init failed

**/
EFI_STATUS
EFIAPI
InitializeChipsetSmmThunkProtocol (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;
  UINTN                     AddressBelow1M;
  
  if (FeaturePcdGet (PcdSmmInt10Enable)) {
    mPrivateData.SmmThunk.SmmInt10 = SmmInt10;
  }  
  //
  // Allocate buffer for ThunkLib code below 1MB
  //
  //
  //  bugbug: reserved any space in SMM instead of the address below 1M
  //  since no A, B segment available in SMM, this space will stored the data
  //  copyied from space below 1M
  //mPrivateData.RealModeBuffer = 0x100000;
  mPrivateData.RealModeBuffer = (UINT64) (-1);
  Status = gSmst->SmmAllocatePages (
                    AllocateMaxAddress,
                    EfiRuntimeServicesData,
                    EFI_SIZE_TO_PAGES(LOW_STACK_SIZE),
                    &mPrivateData.RealModeBuffer
                    );
  ASSERT_EFI_ERROR (Status);

  //
  // Bugbug: Save the data below 1M
  //  
  AddressBelow1M = (UINTN) ((0x100000 - LOW_STACK_SIZE - 16) &~0xf);
  CopyMem ((VOID *) (UINTN) mPrivateData.RealModeBuffer, (VOID *) AddressBelow1M, LOW_STACK_SIZE);

  //
  //Get the real mode thunk implementation the address and size of the real
  //mode buffer needed.
  //
  mPrivateData.BufferSize = EFI_PAGE_SIZE;
  R8AsmThunk16SetProperties (
    &mPrivateData.ThunkContext,
    (VOID*)(UINTN)AddressBelow1M,
    mPrivateData.BufferSize
    ); 

  //
  // Bugbug: Restore the orignal data below 1M
  //
  CopyMem ((VOID *) AddressBelow1M, (VOID *) (UINTN) mPrivateData.RealModeBuffer, LOW_STACK_SIZE);

//  Status = gBS->InstallProtocolInterface (
//                  &mPrivateData.Handle,
//                  &gEfiSmmThunkProtocolGuid,
//                  EFI_NATIVE_INTERFACE,
//                  &mPrivateData.SmmThunk
//                  );
  Status = gSmst->SmmInstallProtocolInterface (
                  &mPrivateData.Handle,
                  &gEfiSmmThunkProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mPrivateData.SmmThunk
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

/**
 Convert EFI_IA32_REGISTER_SET to IA32_REGISTER_SET

 @param [in]   Regs
 @param [out]  Register         Register context will be used in Thunk16Lib

 @retval None

**/
VOID
ConvertRegister (
  IN  EFI_IA32_REGISTER_SET           *Regs,
  OUT IA32_REGISTER_SET               *Register
  )
{
  Register->E.EDI = Regs->E.EDI;
  Register->E.ESI = Regs->E.ESI;
  Register->E.EBP = Regs->E.EBP;
  Register->E.ESP = Regs->E.ESP;
  Register->E.EBX = Regs->E.EBX;
  Register->E.EDX = Regs->E.EDX;
  Register->E.ECX = Regs->E.ECX;
  Register->E.EAX = Regs->E.EAX;
  Register->E.DS  = Regs->E.DS;
  Register->E.ES  = Regs->E.ES;
  Register->E.FS  = Regs->E.FS;
  Register->E.GS  = Regs->E.GS;
  Register->E.CS  = Regs->E.CS;
  Register->E.SS  = Regs->E.SS;
  CopyMem (&Register->E.EFLAGS, &Regs->E.EFlags, sizeof(UINT32));

  return ;
}

/**
 Thunk to 16-bit real mode and call Segment:Offset. Regs will contain the
 16-bit register context on entry and exit. Arguments can be passed on
 the Stack argument

 @param [in]   This             Protocol instance pointer.
 @param [in]   Segment          Segemnt of 16-bit mode call
 @param [in]   Offset           Offset of 16-bit mdoe call
 @param [in]   Regs
 @param [in]   Stack            Caller allocated stack used to pass arguments
 @param [in]   StackSize        Size of Stack in bytes

 @retval FALSE                  Thunk completed, and there were no BIOS errors in the target code.
                                See Regs for status.
 @retval TRUE                   There was a BIOS erro in the target code.

**/
BOOLEAN
EFIAPI
SmmFarCall86 (
  IN EFI_SMM_THUNK_PROTOCOL           *This,
  IN  UINT16                          Segment,
  IN  UINT16                          Offset,
  IN  EFI_IA32_REGISTER_SET           *Regs OPTIONAL,
  IN  VOID                            *Stack OPTIONAL,
  IN  UINTN                           StackSize
  )
{
  UINT32              ThunkFlag;
  IA32_REGISTER_SET   Register;
  THUNK16_CONTEXT       ThunkContext;
  EFI_STATUS          Status;
  UINTN               *SaveBuffer; 

  if (FeaturePcdGet (PcdSmmInt10Enable)) {
    //
    // Transfer the EFI_IA32_REGISTER_SET register set to
    // IA32_REGISTER_SET register set
    //
    if (Regs != NULL) {
      ConvertRegister (Regs, &Register);
    }

    //
    // Set the flag that talk to thunk code
    // this far call will return by iret instruction
    //
    ThunkFlag = 0;

    //
    // Fill the far call address
    //
    Register.E.CS  = Segment;
    Register.E.Eip = Offset;

    if (Segment == 0xC000) {
      //
      // Allocate buffer for saving (0x80000) data
      //
      Status = gSmst->SmmAllocatePool (
               EfiRuntimeServicesData,
               0x1000,
               (VOID **)&SaveBuffer
               );
      if (EFI_ERROR (Status)) {
        return FALSE;
      }

      //
      // Save Data
      //
      CopyMem (SaveBuffer, (VOID*)((UINTN) 0x80000), 0x1000);

      //
      // Prepare environment
      //
      R8AsmThunk16SetProperties (
        &ThunkContext,
        (VOID*)((UINTN) 0x80000),
        0x1000
        );

      //
      // Disable A0000-BFFFF SMRAM attribute
      //
      DisableSMRAMCAttribute ();

      DisableAPIC ();

      R8AsmThunk16FarCall86 (&ThunkContext, &Register, (UINT32)ThunkFlag);

      EnableAPIC ();
      //
      // Restore A0000-BFFFF SMRAM attribute
      //
      RestoreSMRAMCAttribute ();
      //
      // Destroy environment
      //
      R8AsmThunk16Destroy (&ThunkContext);
      //
      // Restore Data
      //
      CopyMem ((VOID*)((UINTN) 0x80000), SaveBuffer, 0x1000);
      Status = gSmst->SmmFreePool (SaveBuffer);
    } else {

      if (mPrivateData.RealModeBuffer) {
        R8AsmThunk16FarCall86 (&mPrivateData.ThunkContext, &Register, (UINT32)ThunkFlag);
      } else {
        //
        // The SmmThunk functionality force off due to out of A/B segment memory
        //
        return FALSE;
      }
    }

    //
    // Transfer the IA32_REGISTER_SET register set to
    // EFI_IA32_REGISTER_SET register set
    //
    if (Regs != NULL) {
      ConvertRegisterBack (Regs, &Register);
    }

  } else {
    //
    // Transfer the EFI_IA32_REGISTER_SET register set to
    // IA32_REGISTER_SET register set
    //
    if (Regs != NULL) {
      ConvertRegister (Regs, &Register);
    }

    //
    // Set the flag that talk to thunk code
    // this far call will return by iret instruction
    //
    ThunkFlag = THUNK_INTERRUPT;

    //
    // Fill the far call address
    //
    Register.E.CS  = Segment;
    Register.E.Eip = Offset;

    DisableAPIC ();

    R8AsmThunk16FarCall86 (&mPrivateData.ThunkContext, &Register, (UINT32)ThunkFlag);

    EnableAPIC ();
  }

  return TRUE;
}

/**
 Convert IA32_REGISTER_SET to EFI_IA32_REGISTER_SET

 @param [out]  Regs
 @param [in]   Register         Register context will be used in Thunk16Lib

 @retval None

**/
VOID
ConvertRegisterBack (
  OUT EFI_IA32_REGISTER_SET           *Regs,
  IN  IA32_REGISTER_SET               *Register
  )
{
  Regs->E.EDI = Register->E.EDI;
  Regs->E.ESI = Register->E.ESI;
  Regs->E.EBP = Register->E.EBP;
  Regs->E.ESP = Register->E.ESP;
  Regs->E.EBX = Register->E.EBX;
  Regs->E.EDX = Register->E.EDX;
  Regs->E.ECX = Register->E.ECX;
  Regs->E.EAX = Register->E.EAX;
  Regs->E.DS  = Register->E.DS;
  Regs->E.ES  = Register->E.ES;
  Regs->E.FS  = Register->E.FS;
  Regs->E.GS  = Register->E.GS;
  Regs->E.CS  = Register->E.CS;
  Regs->E.SS  = Register->E.SS;
  CopyMem (&Regs->E.EFlags, &Register->E.EFLAGS, sizeof(UINT32));

  return ;
}

/**
 Thunk to 16-bit Int10 real mode. Regs will contain the
 16-bit register context on entry and exit. Arguments can be passed on
 the Stack argument

 @param [in]   This             Protocol instance pointer.
 @param [in, out] Regs          Register contexted passed into (and returned) from thunk to
                                16-bit mode

 @retval FALSE                  Thunk completed, and there were no BIOS errors in the target code.
                                See Regs for status.
 @retval TRUE                   There was a BIOS erro in the target code.

**/
BOOLEAN
EFIAPI
SmmInt10(
   IN EFI_SMM_THUNK_PROTOCOL    *This,
   IN OUT EFI_IA32_REGISTER_SET *Regs
   )
{

  THUNK16_CONTEXT           ThunkContext;
  IA32_REGISTER_SET         Register;
  EFI_STATUS                Status;
  UINTN                     *SaveBuffer;
  UINT32                    ThunkFlag;

  //
  // Allocate buffer for saving (0x80000) data
  //
  Status = gSmst->SmmAllocatePool (
             EfiRuntimeServicesData,
             0x1000,
             (VOID **)&SaveBuffer
             );
  if (EFI_ERROR (Status)) {
    return FALSE;
    }

  //
  // Save Data
  //
  CopyMem (SaveBuffer, (VOID*)((UINTN) 0x80000), 0x1000);

  //
  // Prepare environment
  //
  R8AsmThunk16SetProperties (
    &ThunkContext,
    (VOID*)((UINTN) 0x80000),
    0x1000
    );

  //
  // Transfer the EFI_IA32_REGISTER_SET register set to
  // IA32_REGISTER_SET register set
  //
  if (Regs != NULL) {
    ConvertRegister (Regs, &Register);
  }

  //
  // Set the flag that talk to thunk code
  // this far call will return by iret instruction
  //
  ThunkFlag = THUNK_INTERRUPT;

  //
  // Disable A0000-BFFFF SMRAM attribute
  //
  DisableSMRAMCAttribute ();

  DisableAPIC ();

  R8AsmThunk16Int86 (
    &ThunkContext,
    0x10,
    &Register,
    ThunkFlag
    );

  EnableAPIC ();
  //
  // Restore A0000-BFFFF SMRAM attribute
  //
  RestoreSMRAMCAttribute ();

  //
  // Destroy environment
  //
  R8AsmThunk16Destroy (&ThunkContext);

  //
  // Restore Data
  //
  CopyMem ((VOID*)((UINTN) 0x80000), SaveBuffer, 0x1000);
 
  //
  // Transfer the IA32_REGISTER_SET register set to
  // EFI_IA32_REGISTER_SET register set
  //
  if (Regs != NULL) {
    ConvertRegisterBack (Regs, &Register);
  }
  Status = gSmst->SmmFreePool (SaveBuffer);
  
  return TRUE;
}

/**
 Thunk to 16-bit Int10 real mode. Regs will contain the
 16-bit register context on entry and exit. Arguments can be passed on
 the Stack argument

 @param [in]   This             Protocol instance pointer.
 @param [in]   IntNumber        Int vector number
 @param [in, out] Regs          Register contexted passed into (and returned) from thunk to
                                16-bit mode

 @retval FALSE                  Thunk completed, and there were no BIOS errors in the target code.
                                See Regs for status.
 @retval TRUE                   There was a BIOS erro in the target code.

**/
BOOLEAN
EFIAPI
SmmInt86(
   IN EFI_SMM_THUNK_PROTOCOL    *This,
   IN UINT8                     IntNumber,
   IN OUT EFI_IA32_REGISTER_SET *Regs
   )
{
  THUNK16_CONTEXT           ThunkContext;
  UINT32             ThunkFlag;
  IA32_REGISTER_SET  Register;
  UINTN                     *SaveBuffer;
  EFI_STATUS                Status;

  //
  // Allocate buffer for saving (0x80000) data
  //
  Status = gSmst->SmmAllocatePool (
             EfiRuntimeServicesData,
             0x1000,
             (VOID **)&SaveBuffer
             );
  if (EFI_ERROR (Status)) {
    return FALSE;
    }

  //
  // Save Data
  //
  CopyMem (SaveBuffer, (VOID*)((UINTN) 0x80000), 0x1000);

  //
  // Prepare environment
  //
  R8AsmThunk16SetProperties (
    &ThunkContext,
    (VOID*)((UINTN) 0x80000),
    0x1000
    );

  //
  // Transfer the EFI_IA32_REGISTER_SET register set to
  // IA32_REGISTER_SET register set
  //
  if (Regs != NULL) {
    ConvertRegister (Regs, &Register);
  }

  //
  // Set the flag that talk to thunk code
  // this far call will return by iret instruction
  //
  ThunkFlag = THUNK_INTERRUPT;

  DisableAPIC ();
  //
  // Fill the far call address
  //
  R8AsmThunk16Int86 (
    &ThunkContext,
    IntNumber,
    &Register,
    ThunkFlag
    );

  EnableAPIC ();

  //
  // Destroy environment
  //
  R8AsmThunk16Destroy (&ThunkContext);

  //
  // Restore Data
  //
  CopyMem ((VOID*)((UINTN) 0x80000), SaveBuffer, 0x1000);
 
  //
  // Transfer the IA32_REGISTER_SET register set to
  // EFI_IA32_REGISTER_SET register set
  //
  if (Regs != NULL) {
    ConvertRegisterBack (Regs, &Register);
  }

  Status = gSmst->SmmFreePool (SaveBuffer);
  
  return TRUE;
}

/**



**/
EFI_STATUS
DisableSMRAMCAttribute (
  )
{
//  UINTN                       Address;
//
//  Address = PCI_CF8_LIB_ADDRESS (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_SMRAMC);
//  SavedSmramReg = PciCf8Read8 (Address);
//  PciCf8Or8 (Address, B_SA_SMRAMC_D_CLS_MASK);
//
  return EFI_SUCCESS;
}

/**



**/
EFI_STATUS
RestoreSMRAMCAttribute (
  )
{
//
//  UINTN                    Address;
//  
//  Address = PCI_CF8_LIB_ADDRESS (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_SMRAMC);
//  PciCf8Write8 (Address, SavedSmramReg);
  return EFI_SUCCESS;
}

//
// Set Complier Optimize off or else the MMIO access code will be optimized from dword access to byte access
// and caused some MMIO registers access error
//
#ifdef __GNUC__
#pragma GCC optimize ("O0")
#else
#pragma optimize( "", off )
#endif


/**

  Disable APIC
  
 @param None
 
 @retval None
 
*/
STATIC
VOID
DisableAPIC (
  )
{
  UINT64 ApicBaseReg;
  UINT8  *ApicBase;
  UINTN  Index;
  UINTN  Offset;

  ApicBaseReg = AsmReadMsr64(MSR_IA32_APIC_BASE);
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
    // Stall 10ms to waiting for signal stable
    //
    Stall(10 * 1000);
    //
    // Software disable APIC
    //
    if (!SkipSoftEnDisAPIC){
    *(UINT32*)(UINTN)(ApicBase + APIC_REGISTER_SPURIOUS_VECTOR_OFFSET) &= ~0x100;
    }
  }
}

/**

  Enable APIC
  
 @param None
 
 @retval None
*/
STATIC
VOID
EnableAPIC (
  )
{
  UINT64 ApicBaseReg;
  UINT8  *ApicBase;
  UINTN  Index;
  UINTN  Offset;

  ApicBaseReg = AsmReadMsr64(MSR_IA32_APIC_BASE);
  if (ApicBaseReg & EFI_APIC_GLOBAL_ENABLE) {
    ApicBase = (UINT8*)(UINTN)(ApicBaseReg & 0xffffff000);
    //
    // Software enable APIC
    //
  if (!SkipSoftEnDisAPIC){
     *(UINT32*)(UINTN)(ApicBase + APIC_REGISTER_SPURIOUS_VECTOR_OFFSET) |= 0x100;
    }
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
 Get RealModeBuffer Use SmmFarcall86 ().


 @return Return RealModeBuffer

**/
EFI_PHYSICAL_ADDRESS
GetBufferfromEBDA(
  IN    UINTN                    LegacyRegionSize
  )
 {
      
  UINTN                             EbdaAddr = 0;
  UINTN                             EbdaAddrNew = 0;
  UINTN                             EbdaSize = 0;
  EFI_PHYSICAL_ADDRESS              RealModeBuffer = 0;

  EbdaAddr = BDA(0x0E) << 4;
  EbdaSize = EBDA(0) << 10;
  
  if ((EbdaAddr + EbdaSize) & 0x03ff) {
    //
    // Alignment :LegacyRegionSize = LegacyRegionSize + (Top EBDA address - Top EBDA address(alignment))
    //
    LegacyRegionSize = LegacyRegionSize + ((EbdaAddr + EbdaSize) - ((EbdaAddr + EbdaSize) & 0xfffff800));         
  }
  
  BDA(0x13) = BDA(0x13) - ((UINT16)(LegacyRegionSize >> 10));
  BDA(0x0E) = BDA(0x0E) - ((UINT16)(LegacyRegionSize >> 4));
  
  EbdaAddrNew = BDA(0x0E) << 4;
  
  CopyMem (
    (VOID *) (UINTN) EbdaAddrNew,
    (VOID *) (UINTN) EbdaAddr,
    EbdaSize
    );
  
  // 
  // Get Realmode buffer from EBDA
  //
  RealModeBuffer = (EFI_PHYSICAL_ADDRESS)(EbdaAddrNew + EbdaSize);
 
  return RealModeBuffer;
}
