## @file
#
#  SIO Package Description file
#
#******************************************************************************
#* Copyright (c) 2014-2015, Insyde Software Corp. All Rights Reserved.
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

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]

  SioLib|SioIt8987ePkg/Library/SioLib/SioLib.inf
  IoDecodeLib|SioIt8987ePkg/Library/SioIoDecodeLibNull/SioIoDecodeLibNull.inf

[LibraryClasses.common.PEIM]

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  gSioGuid.PcdSioIt8987eSetup|TRUE
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
  #                CIR:0x07
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
    0x89, 0x87, 0x00, 0x01, 0x00, 0x01, UINT16(0x03F8), 0x08, 0x00, 0x04, 0x00,    \ # Com1
    0x89, 0x87, 0x00, 0x01, 0x01, 0x00, UINT16(0x02F8), 0x08, 0x00, 0x03, 0x00,    \ # Com2
    0x89, 0x87, 0x00, 0x04, 0x00, 0x01, UINT16(0x0060), 0x01, 0x00, 0x01, 0x00,    \ # KYBD
    0x89, 0x87, 0x00, 0x05, 0x01, 0x01, UINT16(0x0060), 0x01, 0x00, 0x0C, 0x00,    \ # MOUSE
    0x89, 0x87, 0x00, 0x07, 0x00, 0x00, UINT16(0x0320), 0x08, 0x00, 0x0B, 0x00,    \ # CIR
    0x89, 0x87, 0x00, 0x7F, 0x00, 0x01, UINT16(0x002E), 0x01, 0x00, 0x00, 0x00,    \ # CFG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00     \ # End Entry
  }

  gSioGuid.PcdSioIt8987eSetupStr|L"SioIt8987eSetup00"

[PcdsDynamicDefault]
  gSioGuid.PcdSioIt8987eExtensiveCfg|{ \
    # Extensive Table Entry
      # Entry Point | Length | SIO Instance | Reserve
      #----------------------------------------------
      0xF0, 0x04, 0x00, 0x00, \

    # Com device
      # Type 0 | Length | Device | Device Instance
      # Type 1 | Length | Owned Mode | Enable | Mode Setting
      # Type 2 | Length | Enable | DDN (10 byte)
      # Type 3 | Length | Enable | ACPI IRQ Information
      # Type 4 | Length | Enable | Setup Default | Level Supported | Level Setting
      #-----------------------------------------------------------------------------
      0x00, 0x04, 0x01, 0x00, \                                                       # Identify Device Extensive
      0x01, 0x05, 0x01, 0x00, 0x01, \                                                 # Device Mode Selection
      0x02, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # ASL DDN Setting
      0x03, 0x04, 0x00, 0x01, \                                                       # ASL IRQ Setting
      0x04, 0x06, 0x00, 0x00, 0x01, 0x00, \                                           # FIFO Setting

      0x00, 0x04, 0x01, 0x01, \                                                       # Identify Device Extensive
      0x01, 0x05, 0x01, 0x00, 0x01, \                                                 # Device Mode Selection
      0x02, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # ASL DDN Setting
      0x03, 0x04, 0x00, 0x01, \                                                       # ASL IRQ Setting
      0x04, 0x06, 0x00, 0x00, 0x01, 0x00, \                                           # FIFO Setting

      #-----------------------------------------------------------------------------
      '$', 'S', 'I', 'O' \ # Extensive Table Ending
  }

[PcdsDynamicExDefault]
  !if gSioGuid.PcdSioIt8987eSetup
    gLinkAdvancedTokenSpaceGuid.SioIt8987eSetup00FormSet|{GUID("40f107fc-6aee-40f4-a1c9-9f53633674a9")}
  !endif

[Components.$(PEI_ARCH)]
  !if gSioGuid.PcdSioIt8987eSupported
    SioIt8987ePkg/SioIt8987ePei/SioIt8987ePei.inf
  !endif

[Components.$(DXE_ARCH)]
  !if gSioGuid.PcdSioIt8987eSupported
    SioIt8987ePkg/SioIt8987eDxe/SioIt8987eDxe.inf
  !endif
