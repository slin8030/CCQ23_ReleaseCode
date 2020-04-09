## @file
#
#  SIO Package Description file
#
#******************************************************************************
#* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************


################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = SioNct5104dPkg
  PLATFORM_GUID                  = CBC2E97A-E20C-4387-8357-3BD2766BF92A
  PLATFORM_VERSION               = 0.2
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SioNct5104dPkg
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
  #//[-start-131009-IB13150002-add]//
  DxeChipsetSvcLib|InsydeModulePkg/Library/DxeChipsetSvcLib/DxeChipsetSvcLib.inf
  #//[-end-131009-IB13150002-add]//

[LibraryClasses.common.PEIM]

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  gSioGuid.PcdSioNct5104dSetup|FALSE
  #
  # If you initialize UpdateAsl variable to FALSE, remembering checked
  # mailbox struct in SioAsl\$(SioName).asl.
  #
  gSioGuid.PcdSioNct5104dUpdateAsl|TRUE

[PcdsFixedAtBuild]
  #
  # Device Number: Com:0x01, WDT:0x12
  #
  # TYPEH: SIO ID High Byte
  # TYPEL: SIO ID Low Byte
  # SI:    SIO Instance
  # D:     SIO Device
  # DI:    SIO Device Instance
  # DE:    SIO Device Enable
  # DBASE: SIO Device Base Address
  # SiIZE: SIO Device Size
  # LDN:   SIO Device LDN
  # DIRQ:  SIO Device IRQ
  # DDMA:  SIO Device DMA
  #
  gSioGuid.PcdSioNct5104dCfg|{ \
    #SIO TYPE | SI  | D   | DI  | DE  | DBASE         | SIZE| LDN | DIRQ| DDMA
    #-----------------------------------------------------------------------------
    0xC4, 0x52, 0x01, 0x01, 0x00, 0x01, UINT16(0x03F8), 0x00, 0x00, 0x04, 0x00,    \ # Com1
    0xC4, 0x52, 0x01, 0x01, 0x01, 0x00, UINT16(0x03E8), 0x00, 0x00, 0x03, 0x00,    \ # Com2
    0xC4, 0x52, 0x01, 0x01, 0x02, 0x00, UINT16(0x02F8), 0x00, 0x00, 0x05, 0x00,    \ # Com3
    0xC4, 0x52, 0x01, 0x01, 0x03, 0x00, UINT16(0x02E8), 0x00, 0x00, 0x07, 0x00,    \ # Com4
    0xC4, 0x52, 0x00, 0x12, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00,    \ # WDT - Size fill counter, DIRQ choose minute/second
    0xC4, 0x52, 0x01, 0x7F, 0x00, 0x00, UINT16(0x002E), 0x00, 0x00, 0x00, 0x00,    \ # CFG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00     \ # End Entry
  }

  gSioGuid.PcdSioNct5104dSetupStr|L"SioNct5104dSetup00"

[Components.IA32]
  SioNct5104dPkg/SioNct5104dPei/SioNct5104dPei.inf {
    <LibraryClasses>
      IoLib|MdePkg/Library/PeiIoLibCpuIo/PeiIoLibCpuIo.inf
      PeiOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLibDefault.inf
      PeiOemSvcKernelLib|InsydeOemServicesPkg/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLib.inf
  }

[Components.X64]
  SioNct5104dPkg/SioNct5104dDxe/SioNct5104dDxe.inf {
    <LibraryClasses>
      DxeOemSvcKernelLibDefault|InsydeOemServicesPkg/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLibDefault.inf
      DxeOemSvcKernelLib|InsydeOemServicesPkg/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf
   <BuildOptions>
      *_*_X64_ASLPP_FLAGS  = /DNPCE388_COM_SUPPORT /DNPCE388_COM_SUPPORT
      *_*_X64_ASLCC_FLAGS  = /DNPCE388_COM_SUPPORT /DNPCE388_COM_SUPPORT
  }

