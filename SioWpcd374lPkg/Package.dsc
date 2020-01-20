## @file
#
#  SIO Package Description file.
#
#******************************************************************************
#* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
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

[LibraryClasses.common.PEIM]

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
#[-start-160812-IB07400770-add]#
  gSioGuid.PcdSioWpcd374lSetup|$(W8374LF2_SIO_SUPPORT)
#[-end-160812-IB07400770-add]#
  #
  # If you initialize UpdateAsl variable to FALSE, remembering checked 
  # mailbox struct in SioAsl\$(SioName).asl.
  #    
  gSioGuid.PcdSioWpcd374lUpdateAsl|TRUE

  #
  # If Supported PCD is TRUE, the $(SIO_PKG) will be built into FV,
  # otherwise the $(SIO_PKG) would not be built.
  #
#[-start-160813-IB07400770-modify]#
#[-start-161023-IB07400803-modify]#
!if $(UART_DEBUG_SUPPORT) == YES
!if $(UART_DEBUG_IO_PORT) == 0x3F8
  gSioGuid.PcdSioWpcd374lSupported|$(W8374LF2_SIO_SUPPORT)
!else
  gSioGuid.PcdSioWpcd374lSupported|FALSE
!endif
!else
  gSioGuid.PcdSioWpcd374lSupported|$(W8374LF2_SIO_SUPPORT)
!endif
#[-end-161023-IB07400803-modify]#
#[-end-160813-IB07400770-modify]#

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
  # SiIZE: SIO Device Size
  # LDN:   SIO Device LDN
  # DIRQ:  SIO Device IRQ
  # DDMA:  SIO Device DMA
  #
#[-start-160812-IB07400770-modify]#
  gSioGuid.PcdSioWpcd374lCfg|{ \
    #SIO TYPE | SI  | D   | DI  | DE  | DBASE         | SIZE| LDN | DIRQ| DDMA
    #-----------------------------------------------------------------------------
    0xF1, 0x00, 0x00, 0x01, 0x00, 0x01, UINT16(0x03F8), 0x10, 0x00, 0x04, 0x00,    \ # Com1
    0xF1, 0x00, 0x00, 0x01, 0x01, 0x01, UINT16(0x02F8), 0x10, 0x00, 0x03, 0x00,    \ # Com2
    0xF1, 0x00, 0x00, 0x02, 0x00, 0x00, UINT16(0x03F2), 0x10, 0x00, 0x06, 0x00,    \ # Floppy
    0xF1, 0x00, 0x00, 0x03, 0x00, 0x01, UINT16(0x0278), 0x10, 0x00, 0x07, 0x00,    \ # Lpt
    0xF1, 0x00, 0x00, 0x04, 0x00, 0x01, UINT16(0x0060), 0x00, 0x00, 0x01, 0x00,    \ # KYBD
    0xF1, 0x00, 0x00, 0x05, 0x01, 0x01, UINT16(0x0060), 0x00, 0x00, 0x0C, 0x00,    \ # MOUSE
    0xF1, 0x00, 0x00, 0x7F, 0x00, 0x00, UINT16(0x002E), 0x00, 0x00, 0x00, 0x00,    \ # CFG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00     \ # End Entry
  }
#[-end-160812-IB07400770-modify]#

  gSioGuid.PcdSioWpcd374lSetupStr|L"SioWpcd374lSetup00"

[PcdsDynamicDefault]
  gSioGuid.PcdSioWpcd374lDmi|{ \
    # Title  
      # Titile | Length | SIO Instance | Reserve 
      0xF0, 0x04, 0x00, 0x00, \
  
#[-start-160812-IB07400770-modify]#
    # Com device  
      # MainType | Length | Device | Device Instance 
      # SubType2 | Length | Enable | 0=RS232, 1=RS485 
      # SubType3 | Length | Enable | DDN (10 byte)     
      #-----------------------------------------------------------------------------  
      0x00, 0x04, 0x01, 0x00, \                                                       # Device
      0x02, 0x04, 0x00, 0x00, \                                                       # SubType 2 Setting type
      0x03, 0x0D, 0x01, 0x43, 0x6F, 0x6D, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # SubType 3 Setting DDN
    
      0x00, 0x04, 0x01, 0x01, \                                                       # Device
      0x02, 0x04, 0x00, 0x00, \                                                       # SubType 2 Setting type
      0x03, 0x0D, 0x01, 0x43, 0x6F, 0x6D, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # SubType 3 Setting DDN 
#[-end-160812-IB07400770-modify]#

    # LPT device  
      # MainType | Length | Device | Device Instance 
      # SubType2 | Length | Enable | 0=SPP 1=EPP 2=ECP 3=EPP&ECP
      # SubType3 | Length | Enable | DDN (10 byte)     
      #-----------------------------------------------------------------------------      
      0x00, 0x04, 0x03, 0x00, \                                                       # Device 
      0x02, 0x04, 0x00, 0x00, \                                                       # SubType 2 Mode
      0x03, 0x0D, 0x00, 0x4C, 0x70, 0x74, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # SubType 3 Setting DDN  
      
      #-----------------------------------------------------------------------------      
      0xFF, 0xFF, 0xFF, 0xFF \ # End Entry
  }

[PcdsDynamicExDefault]
  !if gSioGuid.PcdSioWpcd374lSetup
    gLinkAdvancedTokenSpaceGuid.SioWpcd374lSetup00FormSet|{GUID("2c221d35-982e-4e78-ab01-4a01dd39d0f8")}  
  !endif

[Components.$(PEI_ARCH)]
  !if gSioGuid.PcdSioWpcd374lSupported
    SioWpcd374lPkg/SioWpcd374lPei/SioWpcd374lPei.inf
  !endif
  
[Components.$(DXE_ARCH)]
  !if gSioGuid.PcdSioWpcd374lSupported
    SioWpcd374lPkg/SioWpcd374lDxe/SioWpcd374lDxe.inf
  !endif

