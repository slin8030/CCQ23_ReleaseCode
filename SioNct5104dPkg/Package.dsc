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

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]

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
    0xC4, 0x52, 0x01, 0x01, 0x00, 0x01, UINT16(0x03F8), 0x00, 0x00, 0x07, 0x00,    \ # Com1
    0xC4, 0x52, 0x01, 0x01, 0x01, 0x00, UINT16(0x03E8), 0x00, 0x00, 0x03, 0x00,    \ # Com2
    0xC4, 0x52, 0x01, 0x01, 0x02, 0x00, UINT16(0x02F8), 0x00, 0x00, 0x05, 0x00,    \ # Com3
    0xC4, 0x52, 0x01, 0x01, 0x03, 0x00, UINT16(0x02E8), 0x00, 0x00, 0x04, 0x00,    \ # Com4
    0xC4, 0x52, 0x00, 0x12, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00,    \ # WDT - Size fill counter, DIRQ choose minute/second
    0xC4, 0x52, 0x01, 0x7F, 0x00, 0x00, UINT16(0x002E), 0x00, 0x00, 0x00, 0x00,    \ # CFG
    0xC4, 0x52, 0x01, 0x10, 0x00, 0x01, UINT16(0x9002), 0x00, 0x00, 0x00, 0x00,    \ # HWM
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00     \ # End Entry
  }

  gSioGuid.PcdSioNct5104dSetupStr|L"SioNct5104dSetup00"

[PcdsDynamicDefault]
  gSioGuid.PcdSioNct5104dDmi|{ \
    # Title
      # Titile | Length | SIO Instance | Reserve
      0xF0, 0x04, 0x00, 0x00, \

    # Com device
      # MainType | Length | Device | Device Instance
      # SubType2 | Length | Enable | 0=RS232, 1=RS485
      # SubType3 | Length | Enable | DDN (10 byte)
      #-----------------------------------------------------------------------------
      0x00, 0x04, 0x01, 0x00, \                                                       # Device
      0x02, 0x04, 0x00, 0x00, \                                                       # SubType 2 Setting type
      0x03, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # SubType 3 Setting DDN

      0x00, 0x04, 0x01, 0x01, \                                                       # Device
      0x02, 0x04, 0x00, 0x00, \                                                       # SubType 2 Setting type
      0x03, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # SubType 3 Setting DDN

      0x00, 0x04, 0x01, 0x02, \                                                       # Device
      0x02, 0x04, 0x00, 0x00, \                                                       # SubType 2 Setting type
      0x03, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x33, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # SubType 3 Setting DDN

      0x00, 0x04, 0x01, 0x03, \                                                       # Device
      0x02, 0x04, 0x00, 0x00, \                                                       # SubType 2 Setting type
      0x03, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x34, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # SubType 3 Setting DDN

      #-----------------------------------------------------------------------------
      0xFF, 0xFF, 0xFF, 0xFF \ # End Entry
  }

[PcdsDynamicExDefault]
  !if gSioGuid.PcdSioNct5104dSetup
    gLinkAdvancedTokenSpaceGuid.SioNct5104dSetup00FormSet|{GUID("f6c40058-7c9c-4984-b6ec-ba785e64e870")}
  !endif

[Components.$(PEI_ARCH)]
  SioNct5104dPkg/SioNct5104dPei/SioNct5104dPei.inf

[Components.$(DXE_ARCH)]
  SioNct5104dPkg/SioNct5104dDxe/SioNct5104dDxe.inf

