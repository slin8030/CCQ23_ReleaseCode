## @file
#
#  SIO Package Description file
#
#******************************************************************************
#* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = SioIt8987ePkg
  PLATFORM_GUID                  = 770EEFFD-E2D6-4391-815B-DBCA1C0D1D2D
  PLATFORM_VERSION               = 0.2
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SioIt8987ePkg
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]
  SioLib|InsydeModulePkg/Library/SioLib/SioLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  PostCodeLib|MdePkg/Library/BasePostCodeLibPort80/BasePostCodeLibPort80.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  AcpiPlatformLib|InsydeModulePkg/Library/CommonPlatformLib/AcpiPlatformLib/AcpiPlatformLib.inf
  SetupUtilityLib|InsydeModulePkg/Library/SetupUtilityLib/SetupUtilityLib.inf
  BvdtLib|InsydeModulePkg/Library/BvdtLib/BvdtLib.inf
  OemGraphicsLib|InsydeModulePkg/Library/OemGraphicsLib/OemGraphicsLib.inf
  HiiLib|InsydeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeChipsetSvcLib|InsydeModulePkg/Library/DxeChipsetSvcLib/DxeChipsetSvcLib.inf

[LibraryClasses.common.PEIM]

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  gSioGuid.PcdSioIt8987eSetup|FALSE
  #
  # If you initialize UpdateAsl variable to FALSE, remembering checked
  # mailbox struct in SioAsl\$(SioName).asl.
  #
  gSioGuid.PcdSioIt8987eUpdateAsl|TRUE

  #
  # If Supported PCD is TRUE, the $(SIO_PKG) will be built into FV,
  # otherwise the $(SIO_PKG) would not be built.
  #
  gSioGuid.PcdSioIt8987eSupported|TRUE

[PcdsFixedAtBuild]
  #
  # Device Number: Com:0x01, Floppy:0x02, LPT:0x3, KYBD:0x04, MOUSE:0x05, HardWare Monitor:0x10
  #                CIR:0x07, GPIO:0x13
  #
  # TYPEH: SIO ID High Byte
  # TYPEL: SIO ID Low Byte
  # SI:    SIO Instance
  # D:     SIO Device
  # DI:    SIO Device Instance
  # DE:    SIO Device Enable
  # DBASE: SIO Device Base Address
  # SIZE:  SIO Device Size
  # LDN:   SIO Device LDN
  # DIRQ:  SIO Device IRQ
  # DDMA:  SIO Device DMA
  #
  gSioGuid.PcdSioIt8987eCfg|{ \
    #SIO TYPE | SI  | D   | DI  | DE  | DBASE         | SIZE| LDN | DIRQ| DDMA
    #-----------------------------------------------------------------------------
    0x87, 0x86, 0x00, 0x01, 0x00, 0x01, UINT16(0x03F8), 0x00, 0x00, 0x04, 0x00,    \ # Com1
    0x87, 0x86, 0x00, 0x01, 0x01, 0x00, UINT16(0x02F8), 0x00, 0x00, 0x03, 0x00,    \ # Com2
    0x87, 0x86, 0x00, 0x01, 0x02, 0x00, UINT16(0x03E8), 0x00, 0x00, 0x04, 0x00,    \ # Com3
    0x87, 0x86, 0x00, 0x01, 0x03, 0x00, UINT16(0x02E8), 0x00, 0x00, 0x03, 0x00,    \ # Com4
    0x87, 0x86, 0x00, 0x01, 0x04, 0x00, UINT16(0x0338), 0x00, 0x00, 0x04, 0x00,    \ # Com5
    0x87, 0x86, 0x00, 0x01, 0x05, 0x00, UINT16(0x0228), 0x00, 0x00, 0x03, 0x00,    \ # Com6
    0x87, 0x86, 0x00, 0x03, 0x00, 0x00, UINT16(0x0378), 0x00, 0x00, 0x07, 0x00,    \ # Lpt
    0x87, 0x86, 0x00, 0x04, 0x00, 0x01, UINT16(0x0060), 0x00, 0x00, 0x01, 0x00,    \ # KYBD
    0x87, 0x86, 0x00, 0x05, 0x01, 0x01, UINT16(0x0060), 0x00, 0x00, 0x0C, 0x00,    \ # MOUSE
    0x87, 0x86, 0x00, 0x07, 0x00, 0x00, UINT16(0x0320), 0x01, 0x00, 0x0B, 0x00,    \ # CIR
    0x87, 0x86, 0x00, 0x10, 0x00, 0x00, UINT16(0x0290), 0x00, 0x00, 0x00, 0x00,    \ # HWM
    0x87, 0x86, 0x00, 0x12, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00,    \ # WDT
    0x87, 0x86, 0x00, 0x13, 0x00, 0x00, UINT16(0x0700), 0x00, 0x00, 0x00, 0x00,    \ # GPIO
    0x87, 0x86, 0x00, 0x20, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00,    \ # LED
    0x87, 0x86, 0x00, 0x7F, 0x00, 0x00, UINT16(0x002E), 0x00, 0x00, 0x00, 0x00,    \ # CFG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00     \ # End Entry
  }

  gSioGuid.PcdSioIt8987eSetupStr|L"SioIt8987eSetup00"

[Components.IA32]
  SioIt8987ePkg/SioIt8987ePei/SioIt8987ePei.inf {
    <LibraryClasses>
      IoLib|MdePkg/Library/PeiIoLibCpuIo/PeiIoLibCpuIo.inf
      PeiOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLibDefault.inf
      PeiOemSvcKernelLib|InsydeOemServicesPkg/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLib.inf
  }

[Components.X64]
  SioIt8987ePkg/SioIt8987eDxe/SioIt8987eDxe.inf {
    <LibraryClasses>
      DxeOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLibDefault.inf
      DxeOemSvcKernelLib|InsydeOemServicesPkg/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf
  }

