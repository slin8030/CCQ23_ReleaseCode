/** @file
This is the driver that initializes the System Agent.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

@par Specification Reference:
**/

#include "SaInit.h"
#include "SaRegs.h"
#include "Npkt.h"
#include <Protocol/SaPolicy.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/FirmwareVolume2.h>
#include <Library/MemoryAllocationLib.h>

SA_POLICY_PROTOCOL  *SaPolicy;

extern EFI_GUID gSaSsdtAcpiTableStorageGuid;

/**
  Initialize System Agent SSDT ACPI tables

  @retval EFI_SUCCESS    ACPI tables are initialized successfully
  @retval EFI_NOT_FOUND  ACPI tables not found
**/
EFI_STATUS
InitializeSaSsdtAcpiTables (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  EFI_FV_FILETYPE               FileType;
  UINT32                        FvStatus;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINTN                         Size;
  UINTN                         i;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FwVol;
  INTN                          Instance;
  EFI_ACPI_COMMON_HEADER        *CurrentTable;
  UINTN                         AcpiTableKey;
  EFI_ACPI_DESCRIPTION_HEADER   *SaAcpiTable;
  EFI_ACPI_TABLE_PROTOCOL       *AcpiTable;

  FwVol       = NULL;
  SaAcpiTable = NULL;

  ///
  /// Locate ACPI Table protocol
  ///
  DEBUG ((DEBUG_INFO, "Init SA SSDT table\n"));
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to locate EfiAcpiTableProtocol.\n"));
    return EFI_NOT_FOUND;
  }

  ///
  /// Locate protocol.
  /// There is little chance we can't find an FV protocol
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);
  ///
  /// Looking for FV with ACPI storage file
  ///
  for (i = 0; i < NumberOfHandles; i++) {
    ///
    /// Get the protocol on this handle
    /// This should not fail because of LocateHandleBuffer
    ///
    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (VOID **) &FwVol
                    );
    ASSERT_EFI_ERROR (Status);

    ///
    /// See if it has the ACPI storage file
    ///
    Size      = 0;
    FvStatus  = 0;
    Status = FwVol->ReadFile (
                      FwVol,
                      &gSaSsdtAcpiTableStorageGuid,
                      NULL,
                      &Size,
                      &FileType,
                      &Attributes,
                      &FvStatus
                      );

    ///
    /// If we found it, then we are done
    ///
    if (Status == EFI_SUCCESS) {
      break;
    }
  }
  ///
  /// Free any allocated buffers
  ///
  FreePool (HandleBuffer);

  ///
  /// Sanity check that we found our data file
  ///
  ASSERT (FwVol != NULL);
  if (FwVol == NULL) {
    DEBUG ((DEBUG_INFO, "SA SSDT table not found\n"));
    return EFI_NOT_FOUND;
  }
  ///
  /// Our exit status is determined by the success of the previous operations
  /// If the protocol was found, Instance already points to it.
  /// Read tables from the storage file.
  ///
  Instance      = 0;
  CurrentTable  = NULL;
  while (Status == EFI_SUCCESS) {
    Status = FwVol->ReadSection (
                      FwVol,
                      &gSaSsdtAcpiTableStorageGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      (VOID **) &CurrentTable,
                      &Size,
                      &FvStatus
                      );

    if (!EFI_ERROR (Status)) {
      ///
      /// Check the table ID to modify the table
      ///
      if (((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->OemTableId == SIGNATURE_64 ('S', 'a', 'S', 's', 'd', 't', ' ', 0)) {
        SaAcpiTable = (EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable;
        AcpiTableKey = 0;
        Status = AcpiTable->InstallAcpiTable (
                              AcpiTable,
                              SaAcpiTable,
                              SaAcpiTable->Length,
                              &AcpiTableKey
                              );
        ASSERT_EFI_ERROR (Status);
        return EFI_SUCCESS;
      }
      ///
      /// Increment the instance
      ///
      Instance++;
      CurrentTable = NULL;
    }
  }

  return Status;

}

/**
  This is the standard EFI driver point that detects
  whether there is an ICH southbridge in the system
  and if so, initializes the chip.


  @param[in]  ImageHandle             Handle for the image of this driver
  @param[in]  SystemTable             Pointer to the EFI System Table

  @retval     EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
EFIAPI
SaInitEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{

  EFI_STATUS                        Status;

  DEBUG ((DEBUG_INFO, "SaInitDxe Start\n"));

  PSMI_Mem_init();

  ///
  /// LegacyRegion Driver
  ///
  DEBUG ((EFI_D_INFO, "Initializing Legacy Region\n"));
  LegacyRegionInstall (ImageHandle, SystemTable);

  ///
  /// IgdOpRegion Install Initialization
  ///
  DEBUG ((EFI_D_INFO, "Initializing IGD OpRegion\n"));
  IgdOpRegionInit ();

  ///
  /// Install System Agent Ssdt ACPI table
  ///
  DEBUG ((EFI_D_INFO, "Installing System Agent Ssdt ACPI table\n"));
  Status = InitializeSaSsdtAcpiTables ();
  
//[-start-160628-IB07400748-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  ///
  /// Hybrid Graphics (DXE) Initialization
  ///
  DEBUG((DEBUG_INFO, "Initializing Hybrid Graphics (Dxe)\n"));
  HybridGraphicsInit();
#endif
#endif
//[-end-160628-IB07400748-modify]//

  DEBUG ((EFI_D_INFO, "SaInitDxe End\n"));

  return EFI_SUCCESS;
}


/**
   Memory Allocation for PSMI

  @param[in]  None

  @retval  EFI_STATUS
**/
EFI_STATUS
PSMI_Mem_init (
  VOID
  )
{
  UINTN Status=0, MchBarBase;
  UINT32  memReq_MOT=0, memReq_PSMI_handler=0, memReq_PSMI_Expose=0, memReq_Aunit_Mirror=0, memReq_NPK=0;
  UINTN    tempBuff=0;
  EFI_PHYSICAL_ADDRESS  memBase_MOT_Alin=0,memBase_MOT=0, memBase_PSMI_handler=0, memBase_PSMI_Expose=0, memBase_Aunit_Mirror=0, memBase_NPK=0, totalMem=0;
  EFI_PHYSICAL_ADDRESS tempAddr=0;

  ///
  /// PSMI Variable
  ///
  EFI_PHYSICAL_ADDRESS     BaseAddr;
  MIRROR_RANGE_0_0_0_MCHBAR_1 Aunit_mirror_1;
  MIRROR_RANGE_0_0_0_MCHBAR_2 Aunit_mirror_2;
  MOT_OUT_MASK_0_0_0_MCHBAR mot_out_mask_Bunit;
  MOT_OUT_BASE_0_0_0_MCHBAR mot_out_base_Bunit;
  A_CR_SPARE_BIOS_MCHBAR spare_bios_mchbar;
  _MSC0DESTSZ  msc0destsz;
  _MSC0BAR     msc0bar;

  MchBarBase  = MmioRead32 (MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + 0x48) &~BIT0;

  ///
  /// PSMI Handler
  ///
  DEBUG ((EFI_D_INFO, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));
  DEBUG ((EFI_D_INFO, "+++++++++++++++++++++++++++++++++++++++++++++++  PSMI Memory Initialization start +++++++++++++++++++++++++++++++++++++++\n"));
  DEBUG ((EFI_D_INFO, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"));

    Aunit_mirror_1.ul = MCHMmioRead32(MIRROR_RANGE_MCHBAR);
    Aunit_mirror_2.ul = MCHMmioRead32(MIRROR_RANGE_MCHBAR + 4);
    mot_out_mask_Bunit.ul = MCHMmioRead32(MOT_OUT_MASK_Bunit);

  DEBUG ((EFI_D_INFO, " [Aunit Mirror] Aunit_mirror_1.r.MIRROR_BASE                     0x%.12lx\n",Aunit_mirror_1.r.MIRROR_BASE));
  DEBUG ((EFI_D_INFO, " [Aunit Mirror] Aunit_mirror_1                                   0x%.12lx\n",Aunit_mirror_1.ul));
  DEBUG ((EFI_D_INFO, " [Aunit Mirror] Aunit_mirror_2                                   0x%.12lx\n",Aunit_mirror_2.ul));

  switch((Aunit_mirror_1.r.MIRROR_BASE & 0x00000003))
  {
  case 0:
    memReq_PSMI_handler=0;
    break;
  case 1: // 256 KB //256*1024;
    memReq_PSMI_handler = 0x40000;
    break;
  case 2: // 512 KB //512*1024;
    memReq_PSMI_handler = 0x80000;
    break;
  case 3: // 1024KB //1024*1024;
    memReq_PSMI_handler=0x100000;
    break;
  default:
     DEBUG ((EFI_D_INFO, " [PSMI_handler]  memory out of range\n"));
  }

  if(memReq_PSMI_handler !=0)
  {
    ///
    /// Memory Allocation
    ///
    memBase_PSMI_handler = 0xFFFFFFFF;  // must be under 4G
    Status = gBS->AllocatePages (
          AllocateMaxAddress,
          EfiReservedMemoryType,
          ((memReq_PSMI_handler*2)/4096),
          &memBase_PSMI_handler
          );
    ASSERT_EFI_ERROR (Status);
    DEBUG ((EFI_D_INFO, "\n [PSMI_handler] address                                        0x%.12lx",memBase_PSMI_handler));
    memBase_PSMI_handler = NaturalAlignemnt(memBase_PSMI_handler, memReq_PSMI_handler);
    DEBUG ((EFI_D_INFO, "\n [PSMI_handler] Align address                                  0x%.12lx",memBase_PSMI_handler));
    spare_bios_mchbar.r.PSMI_Handler_base = (UINT32)(memBase_PSMI_handler >> 18);
    DEBUG ((EFI_D_INFO, "\n [PSMI_handler] Spare_bios_mchbar.r.PSMI_Handler_base address  0x%.12lx",spare_bios_mchbar.r.PSMI_Handler_base));
    spare_bios_mchbar.r.PSMI_Handler_size = Get_Aunit_mirror_size (Aunit_mirror_1.r.MIRROR_BASE);
    DEBUG ((EFI_D_INFO, "\n [PSMI_handler] Spare_bios_mchbar.r.PSMI_Handler_size size     0x%.12lx\n",spare_bios_mchbar.r.PSMI_Handler_size));
    MCHMmioWrite32(SPARE_BIOS_MCHBAR, spare_bios_mchbar.ul);

  }

  ///
  /// MOT MemInit
  ///
  DEBUG ((EFI_D_INFO, " [MOT]  mot_out_mask_Bunit.r.MOT_OUT_MASK                         0x%.12lx\n",mot_out_mask_Bunit.r.MOT_OUT_MASK));
  DEBUG ((EFI_D_INFO, " [MOT]  mot_out_mask_Bunit.ul                                     0x%.12lx\n",mot_out_mask_Bunit.ul));
  //
  // 0:0, 16:24, 32:25, 64:26, 128:27, 256:28, 512:29, 1024:30
  //
  switch(mot_out_mask_Bunit.r.MOT_OUT_MASK)
  {
  case 0:   // 0 memory required
    memReq_MOT=0;
    DEBUG ((EFI_D_INFO, " [MOT] No memory requested for Mot\n"));
    break;
  case 24:  // 16 MB
    memReq_MOT = _MB(16);
    break;
  case 25:  // 32MB
    memReq_MOT = _MB(32);
    break;
  case 26:  // 64MB
    memReq_MOT = _MB(64);
    break;
  case 27:  // 128MB
    memReq_MOT = _MB(128);
    break;
  case 28:  // 256MB
    memReq_MOT = _MB(256);
    break;
  case 29:  // 512MB
    memReq_MOT = _MB(512);
    break;
  case 30:  // 1024MB
    memReq_MOT = _MB(1024);
    break;
  default:
    DEBUG ((EFI_D_INFO, " [MOT] Mot memory out of range\n"));
  }

  if(mot_out_mask_Bunit.r.MOT_OUT_MASK != 0)
  {
    Status = gBS->AllocatePages (
              AllocateAnyPages,
              EfiReservedMemoryType,
              ((memReq_MOT*2)/4096),
              &memBase_MOT
              );
    ASSERT_EFI_ERROR (Status);
    DEBUG ((EFI_D_INFO, "\n [MOT] memBase_MOT memory address                              0x%.12lx Bytes",memBase_MOT));
    DEBUG ((EFI_D_INFO, "\n [MOT] memBase_MOT memory size                                 0x%.12lx Bytes",memReq_MOT));
    memBase_MOT_Alin = (UINTN) NaturalAlignemnt((UINTN) memBase_MOT, (UINTN) memReq_MOT);
    DEBUG ((EFI_D_INFO, "\n [MOT] N.Aligned memBase_MOT memory address                    0x%.12lx",memBase_MOT_Alin));
    mot_out_base_Bunit.r.MOT_OUT_BASE = (UINT32)  ((UINTN)(memBase_MOT_Alin >> 24));
    mot_out_mask_Bunit.r.MOT_OUT_MASK = (~((memReq_MOT>>24)-1));
    MCHMmioWrite32 (MOT_OUT_BASE_Bunit, mot_out_base_Bunit.ul);
    MCHMmioWrite32 (MOT_OUT_MASK_Bunit, mot_out_mask_Bunit.ul);
    MCHMmioWrite32 (MOT_OUT_BASE_Aunit, mot_out_base_Bunit.ul);
    MCHMmioWrite32 (MOT_OUT_MASK_Aunit, mot_out_mask_Bunit.ul);
    DEBUG ((EFI_D_INFO, "\n [MOT] mot_out_base_Bunit.r.MOT_OUT_BASE                       0x%.12lx",mot_out_base_Bunit.r.MOT_OUT_BASE));
    DEBUG ((EFI_D_INFO, "\n [MOT] mot_out_mask_Bunit.r.MOT_OUT_MASK                       0x%.12lx\n\n",mot_out_mask_Bunit.r.MOT_OUT_MASK));

    BaseAddr = memBase_MOT_Alin + memReq_MOT;
    DEBUG ((EFI_D_INFO, "\n [MOT] N.Aligned End Base memory address                       0x%.12lx\n",BaseAddr));
    // Free unused memory
    tempAddr = memBase_MOT_Alin -  memBase_MOT;

   // CpuDeadLoop();

  //  MmioWrite32(MchBarBase+MOT_OUT_BASE_Bunit, mot_out_base_Bunit.ul);
  //  mot_out_mask_Bunit.r.MOT_OUT_MASK = (~(memReq_MOT>>24)-1);

  //  MmioWrite32(MchBarBase+MOT_OUT_MASK_Bunit, mot_out_mask_Bunit.ul);
  }

  ///
  ///  PSMI Expose + Aunit Mirror + NPK        (naturally aligned)
  ///

  ///
  /// PSMI Expose Buffer MemInit
  ///
  DEBUG ((EFI_D_INFO, " [Aunit Mirror] Aunit_mirror_1.r.MIRROR_BASE                      0x%.12lx\n",Aunit_mirror_1.r.MIRROR_BASE));

  switch(((Aunit_mirror_1.r.MIRROR_BASE & 0x0000000C)>>2))
  {
  case 0:
    memReq_PSMI_Expose=0;
    break;
  case 1: //8 MB
     memReq_PSMI_Expose = 8;
     break;
  case 2: //16MB
     memReq_PSMI_Expose = 16;;
     break;
  case 3:
     memReq_PSMI_Expose = 32;
     break;
  default:
      DEBUG ((EFI_D_INFO, " [PSMI Expose] Buffer memory out of range\n"));
  }


  ///
  /// Aunit Mirroring MemInit
  ///
  DEBUG ((EFI_D_INFO, " [Aunit Mirror] Aunit_mirror_2                                   0x%.12lx\n",Aunit_mirror_2.ul));

  tempBuff = Aunit_mirror_2.r.MIRROR_LIMIT;
  if(tempBuff != 0)
  {
    memReq_Aunit_Mirror= (UINT32) (tempBuff * 8); // multiple of 8MB
  }else {
    DEBUG ((EFI_D_INFO, " [Aunit Mirror] memory out of range\n"));
  }

  ///
  /// NPK MemInit
  ///
  DEBUG ((EFI_D_INFO, " [Aunit Mirror] Aunit_mirror_1.r.MIRROR_BASE                      0x%.12lx\n",Aunit_mirror_1.r.MIRROR_BASE));

  tempBuff = ((Aunit_mirror_1.r.MIRROR_BASE & 0x000FFF0)>>4);
  if(tempBuff != 0)
  {
    memReq_NPK= (UINT32) (tempBuff *8); // multiple of 8MB
  }else
  {
     DEBUG ((EFI_D_INFO, " [NPK] Trace Buffer memory out of range\n"));
  }

  totalMem = memReq_PSMI_Expose + memReq_Aunit_Mirror + memReq_NPK;
  DEBUG ((EFI_D_INFO, "\n ==================================== SIZE OF DIFFERENT MODULES ====================================================="));
  DEBUG ((EFI_D_INFO, "\n || PSMI Expose  %10ld MB    || Aunit mirror %10ld MB    || NPK  %10d MB    || Total %10ld MB  ||", memReq_PSMI_Expose,  memReq_Aunit_Mirror, memReq_NPK, totalMem));
  DEBUG ((EFI_D_INFO, "\n ====================================================================================================================\n"));
    if(totalMem != 0)
  {
      tempBuff =1;

      while(totalMem >= tempBuff )
      {
        tempBuff= tempBuff <<1;
      }
  //    DEBUG ((EFI_D_INFO, "Power of 2 size %ld\n",tempBuff));
        tempBuff = tempBuff *_1MB;
        totalMem= (((tempBuff +  _MB(1024))/4096));
        DEBUG ((EFI_D_INFO, " Totalmem after power of 2                                       %ld Bytes\n", totalMem));

        ///
        /// Memory Allocation
        ///
        Status = gBS->AllocatePages (
          AllocateAnyPages,
          EfiReservedMemoryType,
          (UINTN) totalMem ,
          &BaseAddr
          );
        ASSERT_EFI_ERROR (Status);
        totalMem = (((totalMem / 2))*(4096));
        DEBUG ((EFI_D_INFO, "\n ++++++++++++++++++++++++++++PSMI MEMORY ALLOCATION ++++++++++++++++++++++++++++++++++"));
        DEBUG ((EFI_D_INFO, "\n Module     \t\tBase Address\t\tSize        \t\tEnd Address"));
       // DEBUG ((EFI_D_INFO, "\n Base memory address %lx",BaseAddr));
        memBase_PSMI_Expose = NaturalAlignemnt(BaseAddr, tempBuff);
        DEBUG ((EFI_D_INFO, "\n PSMI Expose\t\t0x%.12lx\t\t0x%.12lx\t\t0x%.12lx",memBase_PSMI_Expose,(memReq_PSMI_Expose *_1MB),memBase_PSMI_Expose+(memReq_PSMI_Expose *_1MB)));
       // DEBUG ((EFI_D_INFO, "\n Naturally Aligned with power of 2 memBase_PSMI_Expose memory address %lx",memBase_PSMI_Expose));
       // DEBUG ((EFI_D_INFO, "\n Aunit_mirror_1.r.MIRROR_BASE = %lx Aunit_mirror_2.r.MIRROR_LIMIT = %lx\n",Aunit_mirror_1.r.MIRROR_BASE,Aunit_mirror_2.r.MIRROR_LIMIT));
        memBase_Aunit_Mirror = memBase_PSMI_Expose + (memReq_PSMI_Expose *_1MB);
        spare_bios_mchbar.r.PSMI_Expose_size = (UINT32) ((UINTN)(Aunit_mirror_1.r.MIRROR_BASE & 0x0000000C)>>2);;
        DEBUG ((EFI_D_INFO, "\n Aunit Mirror\t\t0x%.12lx\t\t0x%.12lx\t\t0x%.12lx",memBase_Aunit_Mirror,(UINTN) (memReq_Aunit_Mirror *_1MB),(memBase_Aunit_Mirror,(UINTN) (memReq_Aunit_Mirror *_1MB))));
        Aunit_mirror_1.r.MIRROR_BASE = (UINT32) ((UINTN) memBase_Aunit_Mirror >>23);
        Aunit_mirror_2.r.MIRROR_LIMIT = (UINT32) ((UINTN)(memBase_Aunit_Mirror + (((memReq_Aunit_Mirror  *1024*1024))-1)) >>23);
        MCHMmioWrite32(MIRROR_RANGE_MCHBAR, Aunit_mirror_1.ul);
        MCHMmioWrite32(MIRROR_RANGE_MCHBAR+4, Aunit_mirror_2.ul);
        MCHMmioWrite32(SPARE_BIOS_MCHBAR, spare_bios_mchbar.ul);
        memBase_NPK = memBase_Aunit_Mirror + ((UINTN) (memReq_Aunit_Mirror *_1MB));
        DEBUG ((EFI_D_INFO, "\n NPK         \t\t0x%.12lx\t\t0x%.12lx\t\t0x%.12lx",memBase_NPK,(UINT32) (memReq_NPK *_1MB),(memBase_NPK+(memReq_NPK *_1MB))));
        DEBUG ((EFI_D_INFO, "\n ===================================================================================="));


        msc0bar.r.MSCBASE = (UINT32) ((UINTN) memBase_NPK >> 12);
        msc0destsz.r.MSCSIZE = (UINT32) (memReq_NPK *_1MB) >> 12;

        DEBUG ((EFI_D_INFO, "\n ++++++++++++++++++++++++++++ FINAL REGISTER VALUES ++++++++++++++++++++++++++++++++++"));
        DEBUG ((EFI_D_INFO, "\n [NPK] _MSC0BAR.r.MSCBASE address                              0x%.12lx",msc0bar.r.MSCBASE));
        DEBUG ((EFI_D_INFO, "\n [NPK] _MSC0DESTSZ.r.MSCSIZE size                              0x%.12lx",msc0destsz.r.MSCSIZE));

        ///
        /// Program the MTB Base Address Register
        ///
        NPKMmioWrite32 (0x54, (MmioRead32 (MmPciAddress (0, 0, 0, 0, 0) + 0x54)| BIT2));
        NPKMmioWrite32 (R_PCH_NPK_CSR_MTB_LBAR, MTB_BASE_ADDRESS);
        NPKMmioWrite32 (R_PCH_NPK_CSR_MTB_UBAR, 0x0);

        tempBuff = NPKMmioRead32 (R_PCH_NPK_CSR_MTB_LBAR);
        DEBUG ((EFI_D_ERROR, "\n [NPK] NpkBaseAddress + R_PCH_NPK_CSR_MTB_LBAR                 0x%.12lx",tempBuff));
        tempBuff = NPKMmioRead32 (0x4);
        tempBuff = tempBuff | 0x02;
        NPKMmioWrite32 (0x4,(UINT32) tempBuff);
        MTBWrite32(((UINTN) MSC0BAR), msc0bar.ul);
        MTBWrite32(((UINTN) MSC0DESTSZ), msc0destsz.ul);
        DEBUG ((EFI_D_INFO, "\n [Aunit mirror] Aunit_mirror_1.r.MIRROR_BASE                   0x%.12lx",Aunit_mirror_1.r.MIRROR_BASE));
        DEBUG ((EFI_D_INFO,  "\n [Aunit mirror] Aunit_mirror_2.r.MIRROR_LIMIT                  0x%.12lx", Aunit_mirror_2.r.MIRROR_LIMIT));
        DEBUG ((EFI_D_INFO, "\n ===================================================================================="));
     }
    DEBUG ((EFI_D_INFO, " \n =======================================  PSMI Memory Initialization Done =========================================\n"));

  return Status;
}







