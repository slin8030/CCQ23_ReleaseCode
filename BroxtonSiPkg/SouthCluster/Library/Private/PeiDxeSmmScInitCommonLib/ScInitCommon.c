/** @file
  SC common library for PCH INIT PEI/DXE/SMM modules

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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

#include <Uefi/UefiBaseType.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/MmPciLib.h>
#include <Library/ScPlatformLib.h>
#include <ScAccess.h>
#include <ScPolicyCommon.h>
#include <Library/ScPlatformLib.h>
#include <Library/S3BootScriptLib.h>

extern CONST PCIE_CONTROLLER_INFO mPcieControllerInfo[];
extern CONST UINT32 mPcieControllerInfoSize;

/**
  This function returns PID according to Root Port Number

  @param[in] RpPort      Root Port Number

  @retval SC_SBI_PID    Returns PID for SBI Access
**/
SC_SBI_PID
GetRpSbiPid (
  IN  UINTN  RpPort
)
{
  if (RpPort < 2) {
    return PID_PCIE0;
  } else {
    return PID_PCIE1;
  }
}

/**
  Calculate root port device number based on physical port index.

  @param[in]  RpIndex              Root port index (0-based).

  @retval     Root port device number.
**/
UINT32
PchGetPcieRpDevice (
  IN  UINT32   RpIndex
  )
{
  UINTN ControllerIndex;
  ControllerIndex = RpIndex / SC_PCIE_CONTROLLER_PORTS;
  ASSERT (ControllerIndex < mPcieControllerInfoSize);
  return mPcieControllerInfo[ControllerIndex].DevNum;
}

/**
  This function reads Pci Config register via SBI Access

  @param[in]  RpDevice            Root Port Device Number
  @param[in]  RpPort              Root Port Number
  @param[in]  Offset              Offset of Config register
  @param[out] *Data32             Value of Config register

  @retval EFI_SUCCESS             SBI Read successful.
**/
EFI_STATUS
PchSbiRpPciRead32 (
  IN    UINTN   RpDevice,
  IN    UINTN   RpPort,
  IN    UINTN   Offset,
  OUT   UINT32  *Data32
)
{
  EFI_STATUS    Status;
  UINT8         Response;
  UINT16        Fid;

  if (RpPort < 2) {
    Fid = (UINT16) ((RpDevice << 3) | RpPort);
  } else {
    Fid = (UINT16) ((RpDevice << 3) | (RpPort - 2 ));
  }
  Status = PchSbiExecutionEx (
             GetRpSbiPid (RpPort),
             Offset,
             PciConfigRead,
             FALSE,
             0xF,
             0,
             Fid,
             Data32,
             &Response
             );
  if (Status != EFI_SUCCESS) {
    DEBUG((DEBUG_ERROR,"Side band Read Failed\n"));
  }
  return Status;
}

/**
  This function And then Or Pci Config register via SBI Access

  @param[in]  RpDevice            Root Port Device Number
  @param[in]  RpPort              Root Port Number
  @param[in]  Offset              Offset of Config register
  @param[in]  Data32And           Value of Config register to be And-ed
  @param[in]  Data32AOr           Value of Config register to be Or-ed

  @retval EFI_SUCCESS             SBI Read and Write successful.
**/
EFI_STATUS
PchSbiRpPciAndThenOr32 (
  IN  UINTN   RpDevice,
  IN  UINTN   RpPort,
  IN  UINTN   Offset,
  IN  UINT32  Data32And,
  IN  UINT32  Data32Or
)
{
  EFI_STATUS  Status;
  UINT32      Data32;
  UINT8       Response;
  UINT16      Fid;

  Status = PchSbiRpPciRead32 (RpDevice, RpPort, Offset, &Data32);
  if (Status == EFI_SUCCESS) {
    Data32 &= Data32And;
    Data32 |= Data32Or;
    if (RpPort < 2) {
      Fid = (UINT16) ((RpDevice << 3) | RpPort);
    } else {
      Fid = (UINT16) ((RpDevice << 3) | (RpPort - 2 ));
    }
    Status = PchSbiExecutionEx (
               GetRpSbiPid (RpPort),
               Offset,
               PciConfigWrite,
               FALSE,
               0xF,
               0,
               Fid,
               &Data32,
               &Response
               );
    if (Status != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR,"Side band Write Failed\n"));
    }
  }
  return Status;
}

/**
  Print registers value

  @param[in] PrintMmioBase       Mmio base address
  @param[in] PrintSize           Number of registers
  @param[in] OffsetFromBase      Offset from mmio base address

  @retval None
**/
VOID
PrintRegisters (
  IN  UINTN        PrintMmioBase,
  IN  UINT32       PrintSize,
  IN  UINT32       OffsetFromBase
  )
{
  UINT32  Offset;
  DEBUG ((DEBUG_VERBOSE, "       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
  for (Offset = 0; Offset < PrintSize; Offset++) {
    if ((Offset % 16) == 0) {
      DEBUG ((DEBUG_VERBOSE, "\n %04X: ", (Offset + OffsetFromBase) & 0xFFF0));
    }
    DEBUG ((DEBUG_VERBOSE, "%02X ", MmioRead8 (PrintMmioBase + Offset)));
  }
  DEBUG ((DEBUG_VERBOSE, "\n"));
}

/**
  PrintPchPciConfigSpace

  @param[in] None

  @retval None
**/
VOID
PrintPchPciConfigSpace (
  VOID
  )
{
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 00 - 0 ] Host Bridge\n"));
  PrintRegisters (MmPciBase ( 0,0,0 ), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 01 - 0 ] PCI Bridge (0-1) x16@1 (x16)\n"));
  PrintRegisters (MmPciBase ( 0,1,0 ), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 02 - 0 ] IGD\n"));
  PrintRegisters (MmPciBase ( 0,2,0 ), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 03 - 0 ] High Def Audio\n"));
  PrintRegisters (MmPciBase (00, 03, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 20(0x14) - 0 ] XHCI Controller\n"));
  PrintRegisters (MmPciBase (00, 20, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 22(0x16) - 0 ] ME\n"));
  PrintRegisters (MmPciBase (00, 22, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 22(0x16) - 2 ] ME IDER\n"));
  PrintRegisters (MmPciBase (00, 22, 02), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 22(0x16) - 3 ] ME SOL\n"));
  PrintRegisters (MmPciBase (00, 22, 03), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 25(0x19) - 0 ] LAN\n"));
  PrintRegisters (MmPciBase (00, 25, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 26(0x1A) - 0 ] EHCI Controller\n"));
  PrintRegisters (MmPciBase (00, 26, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 27(0x1B) - 0 ] High Def Audio\n"));
  PrintRegisters (MmPciBase (00, 27, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 28(0x1C) - 0 ] PCI Bridge (0-2) x0@1 (x1)\n"));
  PrintRegisters (MmPciBase (00, 28, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "PCI Bridge: AECH\n"));
  PrintRegisters (MmPciBase (00, 28, 00) + 0x0100, 0x040,0x0100);
  DEBUG ((DEBUG_VERBOSE, "PCI Bridge: DFT\n"));
  PrintRegisters (MmPciBase (00, 28, 00) + 0x0310, 0x030,0x0310);
  DEBUG ((DEBUG_VERBOSE, "PCI Bridge: VC0\n"));
  PrintRegisters (MmPciBase (00, 28, 00) + 0x0340, 0x020,0x0340);
  DEBUG ((DEBUG_VERBOSE, "PCI Bridge: Port Configuration Extension\n"));
  PrintRegisters (MmPciBase (00, 28, 00) + 0x0400, 0x020,0x0400);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 28(0x1C) - 6 ] PCI Bridge (0-3:10) x0@1 (x1)\n"));
  PrintRegisters (MmPciBase (00, 28, 06), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 29(0x1D) - 0 ] EHCI Controller\n"));
  PrintRegisters (MmPciBase (00, 29, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 31(0x1F) - 0 ] ISA Bridge\n"));
  PrintRegisters (MmPciBase (00, 31, 00), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 31(0x1F) - 2 ] SATA\n"));
  PrintRegisters (MmPciBase (00, 31, 02), 256, 0);
  DEBUG ((DEBUG_VERBOSE, "[ 0 - 31(0x1F) - 4 ] SMBus Controller\n"));
  PrintRegisters (MmPciBase (00, 31, 04), 256, 0);

  DEBUG ((DEBUG_VERBOSE, "\n\n"));
}
