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

#[-start-170427-IB07400864-remove]#
#  SioLib|SioNpce285Pkg/Library/SioLib/SioLib.inf
#[-end-170427-IB07400864-remove]#
  IoDecodeLib|SioNpce285Pkg/Library/SioIoDecodeLibNull/SioIoDecodeLibNull.inf

[LibraryClasses.common.PEIM]

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  gSioGuid.PcdSioNpce285Setup|TRUE
  #
  # If you initialize UpdateAsl variable to FALSE, remembering checked
  # mailbox struct in SioAsl\$(SioName).asl.
  #
  gSioGuid.PcdSioNpce285UpdateAsl|TRUE  

  #
  # If Supported PCD is TRUE, the $(SIO_PKG) will be built into FV,
  # otherwise the $(SIO_PKG) would not be built.
  #
#[-start-170427-IB07400864-modify]#
!if $(KSC_SIO_SUPPORT) == YES
  gSioGuid.PcdSioNpce285Supported|TRUE
!else
  gSioGuid.PcdSioNpce285Supported|FALSE
!endif
#[-end-170427-IB07400864-modify]#

[PcdsFixedAtBuild]
  #
  # Device Number: Com:0x01, Floppy:0x02, LPT:0x3, KYBD:0x04, MOUSE:0x05
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
#[-start-170428-IB07400864-modify]#
  gSioGuid.PcdSioNpce285Cfg|{ \
    #SIO TYPE | SI  | D   | DI  | DE  | DBASE         | SIZE| LDN | DIRQ| DDMA
    #-----------------------------------------------------------------------------
    0xFC, 0x11, 0x00, 0x04, 0x00, 0x01, UINT16(0x0060), 0x01, 0x00, 0x01, 0x00,    \ # KYBD
    0xFC, 0x11, 0x00, 0x05, 0x01, 0x01, UINT16(0x0060), 0x01, 0x00, 0x0C, 0x00,    \ # MOUSE
    0xFC, 0x11, 0x00, 0x7F, 0x00, 0x01, UINT16(0x004E), 0x01, 0x00, 0x00, 0x00,    \ # CFG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00     \ # End Entry
  }
#[-end-170428-IB07400864-modify]#

  gSioGuid.PcdSioNpce285SetupStr|L"SioNpce285Setup00"

[PcdsDynamicDefault]
  gSioGuid.PcdSioNpce285ExtensiveCfg|{ \
    # Extensive Table Entry
      # Entry Point | Length | SIO Instance | Reserve
      #----------------------------------------------
      0xF0, 0x04, 0x00, 0x00, \

      #-----------------------------------------------------------------------------      
      '$', 'S', 'I', 'O' \ # Extensive Table Ending
  }

[PcdsDynamicExDefault]
  !if gSioGuid.PcdSioNpce285Setup
    gLinkAdvancedTokenSpaceGuid.SioNpce285Setup00FormSet|{GUID("1b0a12a0-703c-40f7-8aab-5d7b9464d908")}  
  !endif

[Components.$(PEI_ARCH)]
  !if gSioGuid.PcdSioNpce285Supported
#[-start-170427-IB07400864-modify]#
    SioNpce285Pkg/SioNpce285Pei/SioNpce285Pei.inf {
      <LibraryClasses>
        SioLib|SioNpce285Pkg/Library/SioLib/SioLib.inf
    }
#[-end-170427-IB07400864-modify]#
  !endif

[Components.$(DXE_ARCH)]
  !if gSioGuid.PcdSioNpce285Supported
#[-start-170427-IB07400864-modify]#
    SioNpce285Pkg/SioNpce285Dxe/SioNpce285Dxe.inf {
      <LibraryClasses>
        SioLib|SioNpce285Pkg/Library/SioLib/SioLib.inf
    }
#[-end-170427-IB07400864-modify]#
  !endif

