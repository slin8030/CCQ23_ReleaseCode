/** @file
Pci Host Bridge driver:
Provides the basic interfaces to abstract a PCI Host Bridge Resource Allocation

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

#include "PciRootBridge.h"
#include "PciHostBridge.h"
#include "SaAccess.h"
#include "PlatformBaseAddresses.h"
#include <Library/SideBandLib.h>
#include <ScRegs/RegsUsb.h>
#include <Scregs/RegsPsf.h>
#include <Library/SteppingLib.h>
#include <Npkt.h>
//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
#include <SaRegs.h>
#endif
//[-end-170119-IB07401072-add]//

///
/// Support 64 K IO space
///
#define RES_IO_BASE   0x1000
#define RES_IO_LIMIT  0xFFFF
///
/// Support 4G address space
///
#define RES_MEM_BASE_1  0x80000000
#define RES_MEM_LIMIT_1 (ACPI_MMIO_BASE_ADDRESS - 1)  // Reserve (ACPI_MMIO_BASE) ~ (ACPI_MMIO_BASE_ADDRESS + ACPI_MMIO_SIZE -1) as ACPI MMIO address

//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
#define MEM_EQU_4GB     0x100000000ULL
#endif
//[-end-170119-IB07401072-add]//

///
/// Hard code: Root Bridge Number within the host bridge
///            Root Bridge's attribute
///            Root Bridge's device path
///            Root Bridge's resource appeture
///
static UINTN                              RootBridgeNumber[1]       = { 1 };

static UINT64                             RootBridgeAttribute[1][1] = { EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM };

static EFI_PCI_ROOT_BRIDGE_DEVICE_PATH    mEfiPciRootBridgeDevicePath[1][1] = {
  {
    ACPI_DEVICE_PATH,
    ACPI_DP,
    (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
    (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8),
    EISA_PNP_ID(0x0A03),
    0,
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    END_DEVICE_PATH_LENGTH,
    0
  }
};

static PCI_ROOT_BRIDGE_RESOURCE_APPETURE  mResAppeture[1][1] = { { 0, 255, 0, 0xffffffff, 0, 1 << 16 } };

static EFI_HANDLE                         mDriverImageHandle;

///
/// Implementation
///
/**
  Entry point of PCI Host Bridge driver

  @param[in]  ImageHandle
  @param[in]  SystemTable

  @retval  EFI_DEVICE_ERROR  Start Failed
  @retval  EFI_SUCCESS       Driver Start OK
  @retval  EFI_BUFFER_TOO_SMALL - Fail to allocate required resource
**/
EFI_STATUS
EFIAPI
PciHostBridgeEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS                Status;
  UINTN                     Loop1;
  UINTN                     Loop2;
  PCI_HOST_BRIDGE_INSTANCE  *HostBridge;
  PCI_ROOT_BRIDGE_INSTANCE  *PrivateData;
  EFI_PHYSICAL_ADDRESS      dBMBOUND =0;
  UINT64                    Length;
  UINT32                    buffer32;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR PciMemorySpaceDescriptor;
  UINT64                    BaseAddress;
  UINT64                    Attributes;
//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
  EFI_PHYSICAL_ADDRESS      dBMBOUNDHI =0;
  EFI_PHYSICAL_ADDRESS      Touud;
#endif  
//[-end-170119-IB07401072-add]//

  mDriverImageHandle = ImageHandle;
  //
  // This system has one Host Bridge (one Root Bridge in this Host Bridge)
  //
  //
  // Create Host Bridge Device Handle
  //
  for (Loop1 = 0; Loop1 < HOST_BRIDGE_NUMBER; Loop1++) {

    HostBridge = AllocatePool(sizeof (PCI_HOST_BRIDGE_INSTANCE));
      if (HostBridge == NULL) {
        ASSERT (FALSE);
        return EFI_BUFFER_TOO_SMALL;
      }

    HostBridge->Signature         = PCI_HOST_BRIDGE_SIGNATURE;
    HostBridge->RootBridgeNumber  = RootBridgeNumber[Loop1];
    HostBridge->ResourceSubmited  = FALSE;
    HostBridge->CanRestarted      = TRUE;
    //
    // InitializeListHead (&HostBridge->Head);
    //
    HostBridge->ResAlloc.NotifyPhase          = NotifyPhase;
    HostBridge->ResAlloc.GetNextRootBridge    = GetNextRootBridge;
    HostBridge->ResAlloc.GetAllocAttributes   = GetAttributes;
    HostBridge->ResAlloc.StartBusEnumeration  = StartBusEnumeration;
    HostBridge->ResAlloc.SetBusNumbers        = SetBusNumbers;
    HostBridge->ResAlloc.SubmitResources      = SubmitResources;
    HostBridge->ResAlloc.GetProposedResources = GetProposedResources;
    HostBridge->ResAlloc.PreprocessController = PreprocessController;

    HostBridge->HostBridgeHandle              = NULL;
    Status = gBS->InstallProtocolInterface (
                    &HostBridge->HostBridgeHandle,
                    &gEfiPciHostBridgeResourceAllocationProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &HostBridge->ResAlloc
                    );

    if (EFI_ERROR (Status)) {
      FreePool(HostBridge);
      return EFI_DEVICE_ERROR;
    }
    //
    // Create Root Bridge Device Handle in this Host Bridge
    //
    InitializeListHead (&HostBridge->Head);

    for (Loop2 = 0; Loop2 < HostBridge->RootBridgeNumber; Loop2++) {

      PrivateData = AllocatePool(sizeof (PCI_ROOT_BRIDGE_INSTANCE));
      if (PrivateData == NULL) {
        ASSERT (FALSE);
        return EFI_BUFFER_TOO_SMALL;
      }
      PrivateData->Signature  = PCI_ROOT_BRIDGE_SIGNATURE;

      PrivateData->DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) &mEfiPciRootBridgeDevicePath[Loop1][Loop2];
      RootBridgeConstructor (
        &PrivateData->Io,
        HostBridge->HostBridgeHandle,
        RootBridgeAttribute[Loop1][Loop2],
        &mResAppeture[Loop1][Loop2]
        );

      PrivateData->Handle = NULL;
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &PrivateData->Handle,
                      &gEfiDevicePathProtocolGuid,
                      PrivateData->DevicePath,
                      &gEfiPciRootBridgeIoProtocolGuid,
                      &PrivateData->Io,
                      NULL
                      );
      if (EFI_ERROR (Status)) {
        FreePool(PrivateData);
        return EFI_DEVICE_ERROR;
      }

      InsertTailList (&HostBridge->Head, &PrivateData->Link);
    }
  }

  Status = gDS->AddIoSpace (
                  EfiGcdIoTypeIo,
                  RES_IO_BASE,
                  RES_IO_LIMIT - RES_IO_BASE + 1
                  );
  ///
  /// PCI memory space from top of usable DRAM to TOP of supported memory 3.5GBytes.
  ///
  /// Read TOLUD for now.
  ///
  buffer32 = (MmioRead32 ((UINTN)PcdGet64 (PcdPciExpressBaseAddress) + 0xBC) & 0xFFF00000);
  dBMBOUND  =  (EFI_PHYSICAL_ADDRESS)(buffer32);
  Length      = RES_MEM_LIMIT_1 - dBMBOUND + 1;
  if (Length) {
    DEBUG ((EFI_D_INFO, "Allocating PCI space from 0x%X to 0x%X\n", (UINT32) dBMBOUND, (UINT32) (dBMBOUND + Length - 1)));
    Status = gDS->AddMemorySpace (
                    EfiGcdMemoryTypeMemoryMappedIo,
                    dBMBOUND,
                    Length,
                    0
                    );
    ///
    /// Reserve 0xD0000000 ~ 0xE0000000-1 as ACPI MMIO range to avoid confliction with PCI bridge MMIO range
    ///
    DEBUG ((EFI_D_INFO, " Allocating ACPI MMIO space from 0x%X to 0x%X\n", ACPI_MMIO_BASE_ADDRESS, ACPI_MMIO_BASE_ADDRESS + ACPI_MMIO_SIZE - 1));
    Status = gDS->AddMemorySpace (
          EfiGcdMemoryTypeMemoryMappedIo,
          ACPI_MMIO_BASE_ADDRESS,
          ACPI_MMIO_SIZE,
          0
          );
  }

    ///
    /// Reserve 0xE0000000 ~ 0xE4000000 as Pcie MMIO range
    ///
    DEBUG ((EFI_D_INFO, "Allocating PCIe MMIO space from 0x%X to 0x%X\n", PCIE_MMIO_BASE_ADDRESS, PCIE_MMIO_BASE_ADDRESS + PCIE_MMIO_SIZE - 1));
    Status = gDS->AddMemorySpace (
          EfiGcdMemoryTypeMemoryMappedIo,
          PCIE_MMIO_BASE_ADDRESS,
          PCIE_MMIO_SIZE,
          0
          );

  //
  //  Change PCI memory space attribute to Runtime Memory
  //
  BaseAddress = (EFI_PHYSICAL_ADDRESS)PCIE_MMIO_BASE_ADDRESS;
  Length      = PCIE_MMIO_SIZE;
  Status      = gDS->GetMemorySpaceDescriptor (BaseAddress, &PciMemorySpaceDescriptor);
  ASSERT_EFI_ERROR (Status);

  Attributes  = PciMemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

  Status      = gDS->SetMemorySpaceAttributes (
                         BaseAddress,
                         Length,
                         Attributes
                         );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "Successfully changed memory attribute for PCIe\n"));

//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
  //
  // Provide available above 4GB MMIO resource if PCI 64bit MMIO is enabled
  //
  if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
    Touud = (EFI_PHYSICAL_ADDRESS)LShiftU64 ((MmioRead32 (MmPciAddress (0,SA_MC_BUS, 0, 0, TOUUD_HI_0_0_0_PCI_CUNIT_REG)) & 0xFFFFFFFF),32);
    Touud = (EFI_PHYSICAL_ADDRESS) (Touud + (MmioRead32 (MmPciAddress (0,SA_MC_BUS, 0, 0, TOUUD_LO_0_0_0_PCI_CUNIT_REG)) & 0xFFF00000));
    if (Touud > MEM_EQU_4GB) {
      dBMBOUNDHI = Touud;
    } else {
      dBMBOUNDHI = MEM_EQU_4GB;
    }
    Length = PcdGet64 (PcdMmioMem64Length);
    DEBUG ((EFI_D_ERROR, " Allocating PCI space from 0x%lX to 0x%lX\n", (UINT64)dBMBOUNDHI, (UINT64)(dBMBOUNDHI + Length - 1)));
    Status = gDS->AddMemorySpace (
                    EfiGcdMemoryTypeMemoryMappedIo,
                    dBMBOUNDHI,
                    Length,
                    0
                    );
    ASSERT_EFI_ERROR (Status);
  }
#endif  
//[-end-170119-IB07401072-add]//

  return EFI_SUCCESS;
}

/**
  Enter a certain phase of the PCI enumeration process

  @param[in] This  - The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL instance
  @param[in] Phase - The phase during enumeration

  @retval EFI_SUCCESS            -  Succeed.
  @retval EFI_INVALID_PARAMETER  -  Wrong phase parameter passed in.
  @retval EFI_NOT_READY          -  Resources have not been submitted yet.
**/
EFI_STATUS
EFIAPI
NotifyPhase (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE    Phase
  )
{
  PCI_HOST_BRIDGE_INSTANCE  *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE  *RootBridgeInstance;
  PCI_RESOURCE_TYPE         Index;
  LIST_ENTRY            *List;
  EFI_PHYSICAL_ADDRESS      BaseAddress;
  UINT64                    AddrLen;
  UINTN                     BitsOfAlignment;
  UINT64                    Alignment;
  EFI_STATUS                Status;
  EFI_STATUS                ReturnStatus;
  UINTN                     NpkBaseAddress;
  UINT32                    NpkCsrMtbLbar;
  UINT32                    NpkCsrMtbUbar;

  HostBridgeInstance = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);

  switch (Phase) {
  case EfiPciHostBridgeBeginEnumeration:
    if (HostBridgeInstance->CanRestarted) {
      //
      // Reset the Each Root Bridge
      //
      List = HostBridgeInstance->Head.ForwardLink;

      while (List != &HostBridgeInstance->Head) {
        RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
        for (Index = TypeIo; Index < TypeMax; Index++) {
          RootBridgeInstance->ResAllocNode[Index].Type    = Index;
          RootBridgeInstance->ResAllocNode[Index].Base    = 0;
          RootBridgeInstance->ResAllocNode[Index].Length  = 0;
          RootBridgeInstance->ResAllocNode[Index].Status  = ResNone;
        }

        List = List->ForwardLink;
      }

      HostBridgeInstance->ResourceSubmited  = FALSE;
      HostBridgeInstance->CanRestarted      = TRUE;
    } else {
      //
      // Can not restart
      //
      return EFI_NOT_READY;
    }
    break;

  case EfiPciHostBridgeEndEnumeration:
    break;

  case EfiPciHostBridgeBeginBusAllocation:
    //
    // No specific action is required here, can perform any chipset specific programing
    //
    HostBridgeInstance->CanRestarted = FALSE;
    return EFI_SUCCESS;
    break;

  case EfiPciHostBridgeEndBusAllocation:
    //
    // No specific action is required here, can perform any chipset specific programing
    //
    return EFI_SUCCESS;
    break;

  case EfiPciHostBridgeBeginResourceAllocation:
    ///
    /// No specific action is required here, can perform any chipset specific programing
    ///

    return EFI_SUCCESS;
    break;

  case EfiPciHostBridgeAllocateResources:
    ReturnStatus = EFI_SUCCESS;
    if (HostBridgeInstance->ResourceSubmited) {
      ///
      /// Take care of the resource dependencies between the root bridges
      ///
      List = HostBridgeInstance->Head.ForwardLink;
      while (List != &HostBridgeInstance->Head) {
        RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
        for (Index = TypeIo; Index < TypeBus; Index++) {
          if (RootBridgeInstance->ResAllocNode[Index].Status != ResNone) {
            AddrLen   = RootBridgeInstance->ResAllocNode[Index].Length;
            Alignment = RootBridgeInstance->ResAllocNode[Index].Alignment;

            ///
            /// Get the number of '1' in Alignment.
            ///
            for (BitsOfAlignment = 0; Alignment != 0; BitsOfAlignment++) {
              Alignment = RShiftU64 (Alignment, 1);
            }

            switch (Index) {
            case TypeIo:
              ///
              /// It is impossible for this chipset to align 0xFFFF for IO16
              /// So clear it
              ///
              if (BitsOfAlignment >= 16) {
                BitsOfAlignment = 0;
              }

              Status = gDS->AllocateIoSpace (
                              EfiGcdAllocateAnySearchBottomUp,
                              EfiGcdIoTypeIo,
                              BitsOfAlignment,
                              AddrLen,
                              &BaseAddress,
                              mDriverImageHandle,
                              NULL
                              );
              if (!EFI_ERROR (Status)) {
                RootBridgeInstance->ResAllocNode[Index].Base    = (UINTN) BaseAddress;
                RootBridgeInstance->ResAllocNode[Index].Status  = ResAllocated;
              } else {
                ReturnStatus = Status;
                if (Status != EFI_OUT_OF_RESOURCES) {
                  RootBridgeInstance->ResAllocNode[Index].Length = 0;
                }
              }

              break;

            case TypeMem32:
              //
              // It is impossible for this chipset to align 0xFFFFFFFF for Mem32
              // So clear it
              //
              if (BitsOfAlignment >= 32) {
                BitsOfAlignment = 0;
              }

              //CpuDeadLoop();

              Status = gDS->AllocateMemorySpace (
                              EfiGcdAllocateAnySearchBottomUp,
                              EfiGcdMemoryTypeMemoryMappedIo,
                              BitsOfAlignment,
                              AddrLen,
                              &BaseAddress,
                              mDriverImageHandle,
                              NULL
                              );

              if (!EFI_ERROR (Status)) {
                ///
                /// We were able to allocate the PCI memory
                ///
                RootBridgeInstance->ResAllocNode[Index].Base    = (UINTN) BaseAddress;
                RootBridgeInstance->ResAllocNode[Index].Status  = ResAllocated;
              } else {
                ///
                /// Not able to allocate enough PCI memory
                ///
                ReturnStatus = Status;

                if (Status != EFI_OUT_OF_RESOURCES) {
                  RootBridgeInstance->ResAllocNode[Index].Length = 0;
                }
                ///
                /// Reset
                ///
                gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

              }
              break;

            case TypePMem32:
//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
              if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
	              //
	              // It is impossible for this chipset to align 0xFFFFFFFF for Mem32
	              // So clear it
	              //
	              if (BitsOfAlignment >= 32) {
	                BitsOfAlignment = 0;
	              }

	              //CpuDeadLoop();

	              Status = gDS->AllocateMemorySpace (
	                              EfiGcdAllocateAnySearchBottomUp,
	                              EfiGcdMemoryTypeMemoryMappedIo,
	                              BitsOfAlignment,
	                              AddrLen,
	                              &BaseAddress,
	                              mDriverImageHandle,
	                              NULL
	                              );

	              if (!EFI_ERROR (Status)) {
	                ///
	                /// We were able to allocate the PCI memory
	                ///
	                RootBridgeInstance->ResAllocNode[Index].Base    = (UINTN) BaseAddress;
	                RootBridgeInstance->ResAllocNode[Index].Status  = ResAllocated;
	              } else {
	                ///
	                /// Not able to allocate enough PCI memory
	                ///
	                ReturnStatus = Status;

	                if (Status != EFI_OUT_OF_RESOURCES) {
	                  RootBridgeInstance->ResAllocNode[Index].Length = 0;
	                }
	                ///
	                /// Reset
	                ///
	                gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);

	              }
              } else {
                ReturnStatus = EFI_ABORTED;
              }
              break;
#endif        
//[-end-170119-IB07401072-add]//
            case TypeMem64:
            case TypePMem64:
//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
              if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
	              //
	              // It is impossible for this chipset to align 0xFFFFFFFFFFFFFFFF for Mem64
	              // So clear it
	              //
	              if (BitsOfAlignment >= 64) {
	                BitsOfAlignment = 0;
	              }
	              //
	              // Enable BIOS above 4GB MMIO assignment only when policy enabled.
	              //
	              if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
                    
	                DEBUG ((EFI_D_ERROR, " 64Bit MMIO BIOS assignment enabled\n"));
	                BaseAddress = MAX_ADDRESS;
	                Status = gDS->AllocateMemorySpace (
	                                EfiGcdAllocateAnySearchTopDown,
	                                EfiGcdMemoryTypeMemoryMappedIo,
	                                BitsOfAlignment,
	                                AddrLen,
	                                &BaseAddress,
	                                mDriverImageHandle,
	                                NULL
	                                );
	              } else {
	                Status = gDS->AllocateMemorySpace (
	                                EfiGcdAllocateAnySearchBottomUp,
	                                EfiGcdMemoryTypeMemoryMappedIo,
	                                BitsOfAlignment,
	                                AddrLen,
	                                &BaseAddress,
	                                mDriverImageHandle,
	                                NULL
	                                );
	              }
	              if (!EFI_ERROR (Status)) {
	                ///
	                /// We were able to allocate the PCI memory
	                ///
	                RootBridgeInstance->ResAllocNode[Index].Base    = (UINTN) BaseAddress;
	                RootBridgeInstance->ResAllocNode[Index].Status  = ResAllocated;
	              } else {
	                ///
	                /// Not able to allocate enough PCI memory - critical stop
	                ///
	                DEBUG ((EFI_D_ERROR, "Out of PCI memory! AllocateMemorySpace() returned %r\n", Status));
	                DEBUG ((EFI_D_ERROR, "Size requested: 0x%lX bytes\n", AddrLen));
	                ReturnStatus = Status;
	              }
              } else {
                ReturnStatus = EFI_ABORTED;
              }
#else              
              ReturnStatus = EFI_ABORTED;
#endif
//[-end-170119-IB07401072-add]//
              break;
            default:
              break;
            }
            //
            // end switch
            //
          }
        }

        List = List->ForwardLink;
      }

      return ReturnStatus;
    } else {
      return EFI_NOT_READY;
    }
    //
    // HostBridgeInstance->CanRestarted = FALSE;
    //
    break;

  case EfiPciHostBridgeSetResources:
    //
    // HostBridgeInstance->CanRestarted = FALSE;
    //
    break;

  case EfiPciHostBridgeFreeResources:
    //
    // HostBridgeInstance->CanRestarted = FALSE;
    //
    ReturnStatus  = EFI_SUCCESS;
    List          = HostBridgeInstance->Head.ForwardLink;
    while (List != &HostBridgeInstance->Head) {
      RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
      for (Index = TypeIo; Index < TypeBus; Index++) {
        if (RootBridgeInstance->ResAllocNode[Index].Status == ResAllocated) {
          AddrLen     = RootBridgeInstance->ResAllocNode[Index].Length;
          BaseAddress = RootBridgeInstance->ResAllocNode[Index].Base;
          switch (Index) {
          case TypeIo:
            Status = gDS->FreeIoSpace (BaseAddress, AddrLen);
            if (EFI_ERROR (Status)) {
              ReturnStatus = Status;
            }
            break;

          case TypeMem32:
            Status = gDS->FreeMemorySpace (BaseAddress, AddrLen);
            if (EFI_ERROR (Status)) {
              ReturnStatus = Status;
            }
            break;

          case TypePMem32:
            break;

          case TypeMem64:
            break;

          case TypePMem64:
            break;
          default:
            break;
          }
          //
          // end switch
          //
          RootBridgeInstance->ResAllocNode[Index].Type    = Index;
          RootBridgeInstance->ResAllocNode[Index].Base    = 0;
          RootBridgeInstance->ResAllocNode[Index].Length  = 0;
          RootBridgeInstance->ResAllocNode[Index].Status  = ResNone;
        }
      }

      List = List->ForwardLink;
    }

    HostBridgeInstance->ResourceSubmited  = FALSE;
    HostBridgeInstance->CanRestarted      = TRUE;
    return ReturnStatus;
    break;

  case EfiPciHostBridgeEndResourceAllocation:
    HostBridgeInstance->CanRestarted = FALSE;

    NpkBaseAddress = MmPciAddress (0, 0, PCI_DEVICE_NUMBER_NPK, PCI_FUNCTION_NUMBER_NPK, 0);
    NpkCsrMtbLbar = MmioRead32 (NpkBaseAddress + R_PCH_NPK_CSR_MTB_LBAR) & B_PCH_NPK_CSR_MTB_RBAL;
    NpkCsrMtbUbar = MmioRead32 (NpkBaseAddress + R_PCH_NPK_CSR_MTB_UBAR) & B_PCH_NPK_CSR_MTB_RBAU;

    SideBandWriteReserved32(SB_USB_HOST_PORT, DBC_TRACE_IN_PAYLOAD_BP_LOW, V_OPCODE_MWR, V_RID_BAR_DBC, NpkCsrMtbLbar + DBC_TRACE_IN_PAYLOAD_BP_VALUE_OFFSET);
    SideBandWriteReserved32(SB_USB_HOST_PORT, DBC_TRACE_IN_PAYLOAD_BP_HI,  V_OPCODE_MWR, V_RID_BAR_DBC, NpkCsrMtbUbar);

    SideBandWriteReserved32(SB_USB_HOST_PORT, DBC_TRACE_IN_PAYLOAD_QUALIFIERS, V_OPCODE_MWR, V_RID_BAR_DBC, DBC_TRACE_IN_QUALIFIERS_VALUE);
    SideBandWriteReserved32(SB_USB_HOST_PORT, DBC_TRACE_IN_STATUS_QUALIFIERS,  V_OPCODE_MWR, V_RID_BAR_DBC, DBC_TRACE_IN_QUALIFIERS_VALUE);

    SideBandWriteReserved32(SB_USB_HOST_PORT, DBC_TRACE_IN_STATUS_BP_LOW, V_OPCODE_MWR, V_RID_BAR_DBC, NpkCsrMtbLbar + DBC_TRACE_IN_STATUS_BP_VALUE_OFFSET);
    SideBandWriteReserved32(SB_USB_HOST_PORT, DBC_TRACE_IN_STATUS_BP_HI,  V_OPCODE_MWR, V_RID_BAR_DBC, NpkCsrMtbUbar);
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }
  //
  // end switch
  //
  return EFI_SUCCESS;
}

/**
  Return the device handle of the next PCI root bridge that is associated with
  this Host Bridge

  @param[in]     This              - The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance
  @param[in,out] RootBridgeHandle  - Returns the device handle of the next PCI Root Bridge.
                                     On input, it holds the RootBridgeHandle returned by the most
                                     recent call to GetNextRootBridge().The handle for the first
                                     PCI Root Bridge is returned if RootBridgeHandle is NULL on input

  @retval EFI_SUCCESS            -  Succeed.
  @retval EFI_NOT_FOUND          -  Next PCI root bridge not found.
  @retval EFI_INVALID_PARAMETER  -  Wrong parameter passed in.
**/
EFI_STATUS
EFIAPI
GetNextRootBridge (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN OUT EFI_HANDLE                                   *RootBridgeHandle
  )
{
  BOOLEAN                   NoRootBridge;
  LIST_ENTRY                *List;
  PCI_HOST_BRIDGE_INSTANCE  *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE  *RootBridgeInstance;

  NoRootBridge        = TRUE;
  HostBridgeInstance  = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  List                = HostBridgeInstance->Head.ForwardLink;

  while (List != &HostBridgeInstance->Head) {
    NoRootBridge        = FALSE;
    RootBridgeInstance  = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
    if (*RootBridgeHandle == NULL) {
      ///
      /// Return the first Root Bridge Handle of the Host Bridge
      ///
      *RootBridgeHandle = RootBridgeInstance->Handle;
      return EFI_SUCCESS;
    } else {
      if (*RootBridgeHandle == RootBridgeInstance->Handle) {
        ///
        /// Get next if have
        ///
        List = List->ForwardLink;
        if (List != &HostBridgeInstance->Head) {
          RootBridgeInstance  = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
          *RootBridgeHandle   = RootBridgeInstance->Handle;
          return EFI_SUCCESS;
        } else {
          return EFI_NOT_FOUND;
        }
      }
    }

    List = List->ForwardLink;
  }
  ///
  /// end while
  ///
  if (NoRootBridge) {
    return EFI_NOT_FOUND;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

/**
  Returns the attributes of a PCI Root Bridge.

  @param[in] This              - The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param[in] RootBridgeHandle  - The device handle of the PCI Root Bridge
                                 that the caller is interested in
  @param[out] Attributes       - The pointer to attributes of the PCI Root Bridge

  @retval EFI_SUCCESS            -  Succeed.
  @retval EFI_INVALID_PARAMETER  -  Attributes parameter passed in is NULL or
                            RootBridgeHandle is not an EFI_HANDLE
                            that was returned on a previous call to
                            GetNextRootBridge().
**/
EFI_STATUS
EFIAPI
GetAttributes (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT UINT64                                          *Attributes
  )
{
  LIST_ENTRY                *List;
  PCI_HOST_BRIDGE_INSTANCE  *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE  *RootBridgeInstance;

  if (Attributes == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HostBridgeInstance  = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  List                = HostBridgeInstance->Head.ForwardLink;
  while (List != &HostBridgeInstance->Head) {
    RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
    if (RootBridgeHandle == RootBridgeInstance->Handle) {
//[-start-170120-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
      if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
        RootBridgeInstance->RootBridgeAttrib |= EFI_PCI_HOST_BRIDGE_MEM64_DECODE;
      }
#endif
//[-end-170120-IB07401072-add]//
      *Attributes = RootBridgeInstance->RootBridgeAttrib;
      return EFI_SUCCESS;
    }

    List = List->ForwardLink;
  }
  ///
  /// RootBridgeHandle is not an EFI_HANDLE
  /// that was returned on a previous call to GetNextRootBridge()
  ///
  return EFI_INVALID_PARAMETER;
}

/**
  This is the request from the PCI enumerator to set up
  the specified PCI Root Bridge for bus enumeration process.

  @param[in] This              - The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param[in] RootBridgeHandle  - The PCI Root Bridge to be set up.
  @param[out] Configuration    - Pointer to the pointer to the PCI bus resource descriptor.

  @retval EFI_SUCCESS            -  Succeed.
  @retval EFI_OUT_OF_RESOURCES   -  Not enough pool to be allocated.
  @retval EFI_INVALID_PARAMETER  -  RootBridgeHandle is not a valid handle.
**/
EFI_STATUS
EFIAPI
StartBusEnumeration (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT VOID                                            **Configuration
  )
{
  LIST_ENTRY                *List;
  PCI_HOST_BRIDGE_INSTANCE  *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE  *RootBridgeInstance;
  VOID                      *Buffer;
  UINT8                     *Temp;
  UINT64                    BusStart;
  UINT64                    BusEnd;

  HostBridgeInstance  = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  List                = HostBridgeInstance->Head.ForwardLink;

  while (List != &HostBridgeInstance->Head) {
    RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
    if (RootBridgeHandle == RootBridgeInstance->Handle) {
      ///
      /// Set up the Root Bridge for Bus Enumeration
      ///
      BusStart  = RootBridgeInstance->BusBase;
      BusEnd    = RootBridgeInstance->BusLimit;

      ///
      /// Program the Hardware(if needed) if error return EFI_DEVICE_ERROR
      ///
      Buffer = AllocatePool(sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) + sizeof (EFI_ACPI_END_TAG_DESCRIPTOR));
      if (!Buffer) {
        return EFI_OUT_OF_RESOURCES;
      }

      Temp  = (UINT8 *) Buffer;

      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->Desc                  = 0x8A;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->Len                   = 0x2B;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->ResType               = 2;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->GenFlag               = 0;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->SpecificFlag          = 0;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->AddrSpaceGranularity  = 0;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->AddrRangeMin          = BusStart;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->AddrRangeMax          = 0;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->AddrTranslationOffset = 0;
      ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp)->AddrLen               = BusEnd - BusStart + 1;

      Temp = Temp + sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR);
      ((EFI_ACPI_END_TAG_DESCRIPTOR *) Temp)->Desc = 0x79;
      ((EFI_ACPI_END_TAG_DESCRIPTOR *) Temp)->Checksum = 0x0;

      *Configuration = Buffer;
      return EFI_SUCCESS;
    }

    List = List->ForwardLink;
  }

  return EFI_INVALID_PARAMETER;
}

/**
  This function programs the PCI Root Bridge hardware so that
  it decodes the specified PCI bus range

  @param[in] This              - The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance
  @param[in] RootBridgeHandle  - The PCI Root Bridge whose bus range is to be programmed
  @param[in] Configuration     - The pointer to the PCI bus resource descriptor

  @retval EFI_SUCCESS            -  Succeed.
  @retval EFI_INVALID_PARAMETER  -  Wrong parameters passed in.
**/
EFI_STATUS
EFIAPI
SetBusNumbers (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  IN VOID                                             *Configuration
  )
{
  LIST_ENTRY                *List;
  PCI_HOST_BRIDGE_INSTANCE  *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE  *RootBridgeInstance;
  UINT8                     *Ptr;
  UINTN                     BusStart;
  UINTN                     BusEnd;
  UINTN                     BusLen;

  if (Configuration == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Ptr = Configuration;

  ///
  /// Check the Configuration is valid
  ///
  if (*Ptr != ACPI_ADDRESS_SPACE_DESCRIPTOR) {
    return EFI_INVALID_PARAMETER;
  }

  if (((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Ptr)->ResType != 2) {
    return EFI_INVALID_PARAMETER;
  }

  Ptr += sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR);
  if (*Ptr != ACPI_END_TAG_DESCRIPTOR) {
    return EFI_INVALID_PARAMETER;
  }

  HostBridgeInstance  = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  List                = HostBridgeInstance->Head.ForwardLink;
  Ptr                 = Configuration;
  while (List != &HostBridgeInstance->Head) {
    RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
    if (RootBridgeHandle == RootBridgeInstance->Handle) {
      BusStart  = (UINTN) ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Ptr)->AddrRangeMin;
      BusLen    = (UINTN) ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Ptr)->AddrLen;
      BusEnd    = BusStart + BusLen - 1;
      if (BusStart > BusEnd) {
        return EFI_INVALID_PARAMETER;
      }

      if ((BusStart < RootBridgeInstance->BusBase) || (BusEnd > RootBridgeInstance->BusLimit)) {
        return EFI_INVALID_PARAMETER;
      }
      ///
      /// Update the Bus Range
      ///
      RootBridgeInstance->ResAllocNode[TypeBus].Base    = BusStart;
      RootBridgeInstance->ResAllocNode[TypeBus].Length  = BusLen;
      RootBridgeInstance->ResAllocNode[TypeBus].Status  = ResAllocated;

      ///
      /// Program the Root Bridge Hardware
      ///
      return EFI_SUCCESS;
    }

    List = List->ForwardLink;
  }

  return EFI_INVALID_PARAMETER;
}

/**
  Submits the I/O and memory resource requirements for the specified PCI Root Bridge

  @param[in] This              - The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance
  @param[in] RootBridgeHandle  - The PCI Root Bridge whose I/O and memory resource requirements
                                 are being submitted
  @param[in] Configuration     - The pointer to the PCI I/O and PCI memory resource descriptor

  @retval EFI_SUCCESS            -  Succeed.
  @retval EFI_INVALID_PARAMETER  -  Wrong parameters passed in.
**/
EFI_STATUS
EFIAPI
SubmitResources (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  IN VOID                                             *Configuration
  )
{
  LIST_ENTRY                        *List;
  PCI_HOST_BRIDGE_INSTANCE          *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE          *RootBridgeInstance;
  UINT8                             *Temp;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *ptr;
  UINT64                            AddrLen;
  UINT64                            Alignment;

  ///
  /// Check the input parameter: Configuration
  ///
  if (Configuration == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HostBridgeInstance  = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  List                = HostBridgeInstance->Head.ForwardLink;
  Temp                = (UINT8 *) Configuration;
  while (*Temp == 0x8A) {
    Temp += sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR);
  }

  if (*Temp != 0x79) {
    return EFI_INVALID_PARAMETER;
  }

  Temp = (UINT8 *) Configuration;
  while (List != &HostBridgeInstance->Head) {
    RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
    if (RootBridgeHandle == RootBridgeInstance->Handle) {
      while (*Temp == 0x8A) {
        ptr = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp;

        //
        // Check Address Length
        //
//[-start-170120-IB07401072-modify]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
        if ((ptr->AddrLen > 0xffffffff) && (PcdGet8 (PcdMmioMem64Enable) == 0x00)) {
          return EFI_INVALID_PARAMETER;
        }
#else     
        if ((ptr->AddrLen > 0xffffffff)) {
          return EFI_INVALID_PARAMETER;
        }   
#endif        
//[-end-170120-IB07401072-modify]//
        //
        // Check address range alignment
        //
        if (ptr->AddrRangeMax >= 0xffffffff || ptr->AddrRangeMax != (Power2MaxMemory (ptr->AddrRangeMax + 1) - 1)) {
          return EFI_INVALID_PARAMETER;
        }

        switch (ptr->ResType) {
        case 0:
          ///
          /// Check invalid Address Sapce Granularity
          ///
//[-start-170120-IB07401072-modify]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
          if ((ptr->AddrSpaceGranularity != 32) && (PcdGet8 (PcdMmioMem64Enable) == 0x00)) {
            return EFI_INVALID_PARAMETER;
          }
#else
          if ((ptr->AddrSpaceGranularity != 32)) {
            return EFI_INVALID_PARAMETER;
          }
#endif
//[-end-170120-IB07401072-modify]//
          ///
          /// check the memory resource request is supported by PCI root bridge
          ///
          if (RootBridgeInstance->RootBridgeAttrib == EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM && ptr->SpecificFlag == 0x06) {
            return EFI_INVALID_PARAMETER;
          }

//[-start-170120-IB07401072-modify]//
          AddrLen   = (UINT64) ptr->AddrLen;
          Alignment = (UINT64) ptr->AddrRangeMax;
//[-end-170120-IB07401072-modify]//
          if (ptr->AddrSpaceGranularity == 32) {
            if (ptr->SpecificFlag == 0x06) {
              ///
              /// Apply from GCD
              ///
              RootBridgeInstance->ResAllocNode[TypePMem32].Status = ResSubmitted;
            } else {
              RootBridgeInstance->ResAllocNode[TypeMem32].Length    = AddrLen;
              RootBridgeInstance->ResAllocNode[TypeMem32].Alignment = Alignment;
              RootBridgeInstance->ResAllocNode[TypeMem32].Status    = ResRequested;
              HostBridgeInstance->ResourceSubmited                  = TRUE;
            }
          }

          if (ptr->AddrSpaceGranularity == 64) {
            if (ptr->SpecificFlag == 0x06) {
              RootBridgeInstance->ResAllocNode[TypePMem64].Status = ResSubmitted;
            } else {
              RootBridgeInstance->ResAllocNode[TypeMem64].Status    = ResSubmitted;
//[-start-170120-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
              if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
                RootBridgeInstance->ResAllocNode[TypeMem64].Length    = AddrLen;
                RootBridgeInstance->ResAllocNode[TypeMem64].Alignment = Alignment;
                HostBridgeInstance->ResourceSubmited                  = TRUE;
              }
#endif              
//[-end-170120-IB07401072-add]//
            }
          }
          break;

        case 1:
//[-start-170120-IB07401072-modify]//
          AddrLen   = (UINT64) ptr->AddrLen;
          Alignment = (UINT64) ptr->AddrRangeMax;
//[-end-170120-IB07401072-modify]//
          RootBridgeInstance->ResAllocNode[TypeIo].Length     = AddrLen;
          RootBridgeInstance->ResAllocNode[TypeIo].Alignment  = Alignment;
          RootBridgeInstance->ResAllocNode[TypeIo].Status     = ResRequested;
          HostBridgeInstance->ResourceSubmited                = TRUE;
          break;

        default:
          break;
        }

        Temp += sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR);
      }

      return EFI_SUCCESS;
    }

    List = List->ForwardLink;
  }

  return EFI_INVALID_PARAMETER;
}

/**
  This function returns the proposed resource settings for the specified
  PCI Root Bridge

  @param[in] This              - The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance
  @param[in] RootBridgeHandle  - The PCI Root Bridge handle
  @param[out] Configuration    - The pointer to the pointer to the PCI I/O
                                 and memory resource descriptor

  @retval EFI_SUCCESS            -  Succeed.
  @retval EFI_OUT_OF_RESOURCES   -  Not enough pool to be allocated.
  @retval EFI_INVALID_PARAMETER  -  RootBridgeHandle is not a valid handle.
**/
EFI_STATUS
EFIAPI
GetProposedResources (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT VOID                                            **Configuration
  )
{
  LIST_ENTRY                        *List;
  PCI_HOST_BRIDGE_INSTANCE          *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE          *RootBridgeInstance;
  UINTN                             Index;
  UINTN                             Number;
  VOID                              *Buffer;
  UINT8                             *Temp;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *ptr;
  UINT64                            ResStatus;

  Buffer  = NULL;
  Number  = 0;

  ///
  /// Get the Host Bridge Instance from the resource allocation protocol
  ///
  HostBridgeInstance  = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  List                = HostBridgeInstance->Head.ForwardLink;

  ///
  /// Enumerate the root bridges in this host bridge
  ///
  while (List != &HostBridgeInstance->Head) {
    RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
    if (RootBridgeHandle == RootBridgeInstance->Handle) {
      for (Index = 0; Index < TypeBus; Index++) {
        if (RootBridgeInstance->ResAllocNode[Index].Status != ResNone) {
          Number++;
        }
      }

      if (Number > 0) {

        Buffer = AllocateZeroPool(Number * sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) + sizeof (EFI_ACPI_END_TAG_DESCRIPTOR));

        if (!Buffer) {
          return EFI_OUT_OF_RESOURCES;
        }
      }
      else {
        return EFI_NOT_FOUND;
      }

      Temp = Buffer;
      for (Index = 0; Index < TypeBus; Index++) {
        if (RootBridgeInstance->ResAllocNode[Index].Status != ResNone) {
          ptr       = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Temp;
          ResStatus = RootBridgeInstance->ResAllocNode[Index].Status;

          switch (Index) {
          case TypeIo:
            ///
            /// Io
            ///
            ptr->Desc                   = 0x8A;
            ptr->Len                    = 0x2B;
            ptr->ResType                = 1;
            ptr->GenFlag                = 0;
            ptr->SpecificFlag           = 0;
            ptr->AddrRangeMin           = RootBridgeInstance->ResAllocNode[Index].Base;
            ptr->AddrRangeMax           = 0;
            ptr->AddrTranslationOffset  = (ResStatus == ResAllocated) ? EFI_RESOURCE_SATISFIED : EFI_RESOURCE_LESS;
            ptr->AddrLen                = RootBridgeInstance->ResAllocNode[Index].Length;
            break;

          case TypeMem32:
            ///
            /// Memory 32
            ///
            ptr->Desc                   = 0x8A;
            ptr->Len                    = 0x2B;
            ptr->ResType                = 0;
            ptr->GenFlag                = 0;
            ptr->SpecificFlag           = 0;
            ptr->AddrSpaceGranularity   = 32;
            ptr->AddrRangeMin           = RootBridgeInstance->ResAllocNode[Index].Base;
            ptr->AddrRangeMax           = 0;
            ptr->AddrTranslationOffset  = (ResStatus == ResAllocated) ? EFI_RESOURCE_SATISFIED : EFI_RESOURCE_LESS;
            ptr->AddrLen                = RootBridgeInstance->ResAllocNode[Index].Length;
            break;

          case TypePMem32:
            ///
            /// Prefetch memory 32
            ///
            ptr->Desc                   = 0x8A;
            ptr->Len                    = 0x2B;
            ptr->ResType                = 0;
            ptr->GenFlag                = 0;
            ptr->SpecificFlag           = 6;
            ptr->AddrSpaceGranularity   = 32;
            ptr->AddrRangeMin           = 0;
            ptr->AddrRangeMax           = 0;
            ptr->AddrTranslationOffset  = EFI_RESOURCE_NONEXISTENT;
            ptr->AddrLen                = 0;
//[-start-170120-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
            if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
              ptr->AddrTranslationOffset  = (ResStatus == ResAllocated) ? EFI_RESOURCE_SATISFIED : EFI_RESOURCE_LESS;
              ptr->AddrLen                = RootBridgeInstance->ResAllocNode[Index].Length;
            }  
#endif            
//[-end-170120-IB07401072-add]//
            break;

          case TypeMem64:
            ///
            /// Memory 64
            ///
            ptr->Desc                   = 0x8A;
            ptr->Len                    = 0x2B;
            ptr->ResType                = 0;
            ptr->GenFlag                = 0;
            ptr->SpecificFlag           = 0;
            ptr->AddrSpaceGranularity   = 64;
            ptr->AddrRangeMin           = 0;
            ptr->AddrRangeMax           = 0;
            ptr->AddrTranslationOffset  = EFI_RESOURCE_NONEXISTENT;
            ptr->AddrLen                = 0;
//[-start-170120-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
            if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
              ptr->AddrRangeMin           = RootBridgeInstance->ResAllocNode[Index].Base;
              ptr->AddrTranslationOffset  = (ResStatus == ResAllocated) ? EFI_RESOURCE_SATISFIED : EFI_RESOURCE_LESS;
              ptr->AddrLen                = RootBridgeInstance->ResAllocNode[Index].Length;
            }  
#endif            
//[-end-170120-IB07401072-add]//
            break;

          case TypePMem64:
            ///
            /// Prefetch memory 64
            ///
            ptr->Desc                   = 0x8A;
            ptr->Len                    = 0x2B;
            ptr->ResType                = 0;
            ptr->GenFlag                = 0;
            ptr->SpecificFlag           = 6;
            ptr->AddrSpaceGranularity   = 64;
            ptr->AddrRangeMin           = 0;
            ptr->AddrRangeMax           = 0;
            ptr->AddrTranslationOffset  = EFI_RESOURCE_NONEXISTENT;
            ptr->AddrLen                = 0;
//[-start-170120-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
            if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
              ptr->AddrTranslationOffset  = (ResStatus == ResAllocated) ? EFI_RESOURCE_SATISFIED : EFI_RESOURCE_LESS;
              ptr->AddrLen                = RootBridgeInstance->ResAllocNode[Index].Length;
            }
#endif            
//[-end-170120-IB07401072-add]//
            break;
          }

          Temp += sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR);
        }
      }

      ((EFI_ACPI_END_TAG_DESCRIPTOR *) Temp)->Desc      = 0x79;
      ((EFI_ACPI_END_TAG_DESCRIPTOR *) Temp)->Checksum  = 0x0;

      *Configuration = Buffer;

      return EFI_SUCCESS;
    }

    List = List->ForwardLink;
  }

  return EFI_INVALID_PARAMETER;
}

/**
  This function is called for all the PCI controllers that the PCI
  bus driver finds. Can be used to Preprogram the controller.

  @param[in] This              - The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance
  @param[in] RootBridgeHandle  - The PCI Root Bridge handle
  @param[in] PciAddress        - Address of the controller on the PCI bus
  @param[in] Phase             - The Phase during resource allocation

  @retval EFI_SUCCESS            -  Succeed.
  @retval EFI_INVALID_PARAMETER  -  RootBridgeHandle is not a valid handle.
**/
EFI_STATUS
EFIAPI
PreprocessController (
  IN  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL          *This,
  IN  EFI_HANDLE                                                RootBridgeHandle,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS               PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE              Phase
  )
{
  PCI_HOST_BRIDGE_INSTANCE  *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE  *RootBridgeInstance;
  LIST_ENTRY                *List;

  HostBridgeInstance  = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  List                = HostBridgeInstance->Head.ForwardLink;

  ///
  /// Enumerate the root bridges in this host bridge
  ///
  while (List != &HostBridgeInstance->Head) {
    RootBridgeInstance = DRIVER_INSTANCE_FROM_LIST_ENTRY (List);
    if (RootBridgeHandle == RootBridgeInstance->Handle) {
      return EFI_SUCCESS;
    }

    List = List->ForwardLink;
  }

  return EFI_INVALID_PARAMETER;
}

/**
  Calculate max memory of power 2

  @param[in] MemoryLength  -  Input memory length.

  @retval Returned Maximum length.
**/
UINT64
Power2MaxMemory (
  IN UINT64                     MemoryLength
  )
{
  UINT64  Result;

  if (RShiftU64 (MemoryLength, 32)) {
    Result = LShiftU64 ((UINT64) GetPowerOfTwo64 ( RShiftU64 (MemoryLength, 32)), 32);
  } else {
    Result = (UINT64) GetPowerOfTwo64 ( MemoryLength);
  }

  return Result;
}
