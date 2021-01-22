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
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  EDK_GLOBAL SIO_ENABLE_GPIO_PEI = YES

################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]

  SioLib|SioNct5104dPkg/Library/SioLib/SioLib.inf
# IoDecodeLib|SioNct5104dPkg/Library/SioIoDecodeLibNull/SioIoDecodeLibNull.inf
  IoDecodeLib|BroxtonChipsetPkg\Library\IoDecodeLib\IoDecodeLib.inf
[LibraryClasses.common.PEIM]

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]
  gSioGuid.PcdSioNct5104dSetup|TRUE
  #
  # If you initialize UpdateAsl variable to FALSE, remembering checked
  # mailbox struct in SioAsl\$(SioName).asl.
  #
  gSioGuid.PcdSioNct5104dUpdateAsl|TRUE

  #
  # If Supported PCD is TRUE, the $(SIO_PKG) will be built into FV,
  # otherwise the $(SIO_PKG) would not be built.
  #
  gSioGuid.PcdSioNct5104dSupported|TRUE

  #
  # If EnableGpioPei is TRUE, Gpio would be programmed in Pei & Dxe,
  # otherwise Gpio would only programmed in Dxe
  #
  gSioGuid.PcdSioNct5104dEnableGpioPei|$(SIO_ENABLE_GPIO_PEI)

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
  # SIZE:  SIO Device Size
  # LDN:   SIO Device LDN
  # DIRQ:  SIO Device IRQ
  # DDMA:  SIO Device DMA
  #
  gSioGuid.PcdSioNct5104dCfg|{ \
    #SIO TYPE | SI  | D   | DI  | DE  | DBASE         | SIZE| LDN | DIRQ| DDMA
    #-----------------------------------------------------------------------------
    0xC4, 0x52, 0x00, 0x01, 0x00, 0x01, UINT16(0x03F8), 0x08, 0x00, 0x04, 0x00,    \ # Com1
    0xC4, 0x52, 0x00, 0x01, 0x01, 0x01, UINT16(0x02F8), 0x08, 0x00, 0x03, 0x00,    \ # Com2
    0xC4, 0x52, 0x00, 0x01, 0x02, 0x01, UINT16(0x03E8), 0x08, 0x00, 0x07, 0x00,    \ # Com3
    0xC4, 0x52, 0x00, 0x01, 0x03, 0x00, UINT16(0x02E8), 0x08, 0x00, 0x03, 0x00,    \ # Com4                                          
    0xC4, 0x52, 0x00, 0x12, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00,    \ # WDT - Size fill counter, DIRQ choose minute/second
    0xC4, 0x52, 0x00, 0x13, 0x00, 0x00, UINT16(0x0700), 0xff, 0x00, 0x00, 0x00,    \ # GPIO
    0xC4, 0x52, 0x00, 0x15, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00,    \ # Port80
    0xC4, 0x52, 0x00, 0x7F, 0x00, 0x01, UINT16(0x004E), 0x01, 0x00, 0x00, 0x00,    \ # CFG
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, UINT16(0x0000), 0x00, 0x00, 0x00, 0x00     \ # End Entry
  }

  gSioGuid.PcdSioNct5104dSetupStr|L"SioNct5104dSetup00"

  gSioGuid.PcdSioNct5104dPort80toUart|{ \
    # Type 15 | Length | Enable | Source Supported:Bit[7] I2C to 80 Port, Bit[6] LPC to 80 port, Bit[5:0] Port80 to UART Supported List | Source Selection | BaudRate Setting
    0x0F, 0x09, 0x01, 0xC4, 0x44, UINT32(0x0001C200) \                               # Port80 to UART
  }

  gSioGuid.PcdSioNct5104dPeiCustomizeSettingTable|{ \
    0x1C ,0x1C, \
    0x26 ,0x50, \ # Enable access permitionn 
    0x07 ,0x02, \ # SELECT COM A 
    0xF8 ,0x01, \ # Enable 128 BYTE FFIO 
    0x07 ,0x03, \ # SELECT COM B
    0xF8 ,0x01, \
    0x07 ,0x10, \ # SELECT COM C 
    0xF8 ,0x01, \
    0x07 ,0x11, \ # SELECT COM D
    0xF8 ,0x01, \
    0x14 ,0x00  \ # set COM A, B to Edge trigger
  }

[PcdsDynamicDefault]
  gSioGuid.PcdSioNct5104dExtensiveCfg|{ \
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
      0x01, 0x05, 0x03, 0x01, 0x01, \                                                 # Device Mode Selection
      0x02, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # ASL DDN Setting
      0x03, 0x04, 0x00, 0x01, \                                                       # ASL IRQ Setting
      0x04, 0x06, 0x00, 0x00, 0x03, 0x00, \                                           # FIFO Setting

      0x00, 0x04, 0x01, 0x01, \                                                       # Identify Device Extensive
      0x01, 0x05, 0x03, 0x01, 0x01, \                                                 # Device Mode Selection
      0x02, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # ASL DDN Setting
      0x03, 0x04, 0x00, 0x01, \                                                       # ASL IRQ Setting
      0x04, 0x06, 0x00, 0x00, 0x03, 0x00, \                                           # FIFO Setting

      0x00, 0x04, 0x01, 0x02, \                                                       # Identify Device Extensive
      0x01, 0x05, 0x03, 0x01, 0x01, \                                                 # Device Mode Selection
      0x02, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x33, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # ASL DDN Setting
      0x03, 0x04, 0x00, 0x01, \                                                       # ASL IRQ Setting
      0x04, 0x06, 0x00, 0x00, 0x03, 0x00, \                                           # FIFO Setting

      0x00, 0x04, 0x01, 0x03, \                                                       # Identify Device Extensive
      0x01, 0x05, 0x03, 0x00, 0x01, \                                                 # Device Mode Selection
      0x02, 0x0D, 0x00, 0x43, 0x6F, 0x6D, 0x34, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \ # ASL DDN Setting
      0x03, 0x04, 0x00, 0x01, \                                                       # ASL IRQ Setting
      0x04, 0x06, 0x00, 0x00, 0x03, 0x00, \                                           # FIFO Setting

    # WDT device
      # Type 0  | Length | Device | Device Instance
      # Type 16 | Length | Unit Supported Mode | Unit Selection | Time-out | Maximam Time-out
      #-----------------------------------------------------------------------------
      0x00, 0x04, 0x12, 0x00, \                                                       # Device
      0x10, 0x08, 0x0F, 0x00, UINT16(0x0000), UINT16(0x00FF), \                       # WDT Time-out Setting

    # GPIO device
      # Type 0  | Length | Device | Device Instatnce
      # Type 17 | Length | Enable | GpNum [4:Group ; 4:GPNum] | [Capability] | [0 : Pull ; 1 : TriState ; 2 : Invert ; 3 : InOut ; 4: Pei OutVal 5 : Dxe OutVal 6 : Reserved 7 : Reserved]
      #---------------------------------Group 0-------------------------------------
      0x00, 0x04, 0x13, 0x00, \
      0x11, 0x06, 0x01, 0x00, 0x7C, 0x02,                                     \ #GPIO00
      0x00, 0x04, 0x13, 0x01, \
      0x11, 0x06, 0x01, 0x01, 0x7C, 0x02,                                     \ #GPIO01
      0x00, 0x04, 0x13, 0x02, \
      0x11, 0x06, 0x01, 0x02, 0x7C, 0x02,                                     \ #GPIO02
      0x00, 0x04, 0x13, 0x03, \
      0x11, 0x06, 0x01, 0x03, 0x7C, 0x02,                                     \ #GPIO03
      0x00, 0x04, 0x13, 0x04, \
      0x11, 0x06, 0x01, 0x04, 0x7C, 0x02,                                     \ #GPIO04
      0x00, 0x04, 0x13, 0x05, \
      0x11, 0x06, 0x01, 0x05, 0x7C, 0x02,                                     \ #GPIO05
      0x00, 0x04, 0x13, 0x06, \
      0x11, 0x06, 0x01, 0x06, 0x7C, 0x02,                                     \ #GPIO06
      0x00, 0x04, 0x13, 0x07, \
      0x11, 0x06, 0x01, 0x07, 0x7C, 0x02,                                     \ #GPIO07
      #---------------------------------Group 1-------------------------------------
      0x00, 0x04, 0x13, 0x08, \
      0x11, 0x06, 0x01, 0x10, 0x7C, 0x02,                                     \ #GPIO10
      0x00, 0x04, 0x13, 0x09, \
      0x11, 0x06, 0x01, 0x11, 0x7C, 0x02,                                     \ #GPIO11
      0x00, 0x04, 0x13, 0x0A, \
      0x11, 0x06, 0x01, 0x12, 0x7C, 0x02,                                     \ #GPIO12
      0x00, 0x04, 0x13, 0x0B, \
      0x11, 0x06, 0x01, 0x13, 0x7C, 0x02,                                     \ #GPIO13
      0x00, 0x04, 0x13, 0x0C, \
      0x11, 0x06, 0x01, 0x14, 0x7C, 0x02,                                     \ #GPIO14
      0x00, 0x04, 0x13, 0x0D, \
      0x11, 0x06, 0x01, 0x15, 0x7C, 0x02,                                     \ #GPIO15
      0x00, 0x04, 0x13, 0x0E, \
      0x11, 0x06, 0x01, 0x16, 0x7C, 0x02,                                     \ #GPIO16
      0x00, 0x04, 0x13, 0x0F, \
      0x11, 0x06, 0x01, 0x17, 0x7C, 0x02,                                     \ #GPIO17
      #---------------------------------Group 6-------------------------------------
      0x00, 0x04, 0x13, 0x10, \
      0x11, 0x06, 0x01, 0x60, 0x7C, 0x02,                                     \ #GPIO60
      0x00, 0x04, 0x13, 0x11, \
      0x11, 0x06, 0x01, 0x61, 0x7C, 0x02,                                     \ #GPIO61
      0x00, 0x04, 0x13, 0x12, \
      0x11, 0x06, 0x01, 0x62, 0x7C, 0x02,                                     \ #GPIO62
      0x00, 0x04, 0x13, 0x13, \
      0x11, 0x06, 0x01, 0x63, 0x7C, 0x02,                                     \ #GPIO63
      0x00, 0x04, 0x13, 0x14, \
      0x11, 0x06, 0x01, 0x64, 0x7C, 0x02,                                     \ #GPIO64
      0x00, 0x04, 0x13, 0x15, \
      0x11, 0x06, 0x01, 0x65, 0x7C, 0x02,                                     \ #GPIO65
      0x00, 0x04, 0x13, 0x16, \
      0x11, 0x06, 0x01, 0x66, 0x7C, 0x02,                                     \ #GPIO66
      0x00, 0x04, 0x13, 0x17, \
      0x11, 0x06, 0x01, 0x67, 0x7C, 0x02,                                     \ #GPIO67

      #-----------------------------------------------------------------------------
      '$', 'S', 'I', 'O' \ # Extensive Table Ending
  }

[PcdsDynamicExDefault]
  !if gSioGuid.PcdSioNct5104dSetup
    gLinkAdvancedTokenSpaceGuid.SioNct5104dSetup00FormSet|{GUID("f6c40058-7c9c-4984-b6ec-ba785e64e870")}  
  !endif

[Components.$(PEI_ARCH)]
  !if gSioGuid.PcdSioNct5104dSupported
    SioNct5104dPkg/SioNct5104dPei/SioNct5104dPei.inf {
      <LibraryClasses>
        !if $(SIO_ENABLE_GPIO_PEI) == NO
          SioGpioLib|SioNct5104dPkg/Library/SioGpioLibNull/SioGpioLibNull.inf
        !else
          SioGpioLib|SioNct5104dPkg/Library/SioGpioLib/SioGpioLib.inf
        !endif
    }
  !endif
  
[Components.$(DXE_ARCH)]
  !if gSioGuid.PcdSioNct5104dSupported
    SioNct5104dPkg/SioNct5104dDxe/SioNct5104dDxe.inf {
      <LibraryClasses>
        SioGpioLib|SioNct5104dPkg/Library/SioGpioLib/SioGpioLib.inf
    }
  !endif

