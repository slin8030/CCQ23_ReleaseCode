## @file
# H2O Console Redirection package project build description file.
#
#******************************************************************************
#* Copyright (c) 2013 - 2016, Insyde Software Corporation. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
#

[Defines]

[LibraryClasses]
  CrConfigDefaultLib|InsydeCrPkg/Library/CrConfigDefaultLib/CrConfigDefaultLib.inf
  CrDeviceVariableLib|InsydeCrPkg/Library/CrDeviceVariableLib/CrDeviceVariableLib.inf
  SelfDebugLib|InsydeCrPkg/Library/SelfDebugLib/SelfDebugLib.inf

[LibraryClasses.common.PEIM]

[LibraryClasses.common.DXE_DRIVER]

[LibraryClasses.common.DXE_SMM_DRIVER]

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.UEFI_APPLICATION]

[PcdsFeatureFlag]
  #
  # Console Redirection Feature Flag
  #
  gInsydeCrTokenSpaceGuid.PcdH2OConsoleRedirectionSupported|TRUE
  gInsydeCrTokenSpaceGuid.PcdH2OCRPciSerialSupported|TRUE
  gInsydeCrTokenSpaceGuid.PcdH2OCRUsbSerialSupported|FALSE
  gInsydeCrTokenSpaceGuid.PcdH2OCRHsuSerialSupported|TRUE

  #
  # Advance Console Redirection Feature Flag
  # (Here's PCDs work on wired-or)
  #
  gInsydeCrTokenSpaceGuid.PcdH2OAdvanceConsoleRedirectionSupported|FALSE
  gInsydeCrTokenSpaceGuid.PcdH2OCRSrvManagerSupported|FALSE
  gInsydeCrTokenSpaceGuid.PcdH2OCRTelnetSupported|FALSE

[PcdsFixedAtBuild]

  gInsydeCrTokenSpaceGuid.PcdH2OCrSoftwareSmi|0xDC

  #
  # Console Redirection support Devices List
  #   List can be extended and 0xFF indicates end of list.
  #
  # Enable = 0 : Disable this device in SCU default.
  # Enable = 1 : If the Device path exist, then enable this device in SCU default.
  # DevName    : If device exist then the device name is dispalyed on console Redirection page in SCU.
  # Device Path String:
  #   Device path need to description until the previous node of the UART node.
  #   If the CR device need used itself default setting, the device can description device path until terminal device node.
  #
  ## Example 1: (Using COMA to redirect console default, Uart setting come from VFR globol setting)
  #    UINT8(0x01), L"COM_A"    , L"PciRoot(0x0)/Pci(0x1F,0x0)/Serial(0x0)"
  #
  ## Example 2: (After boot search all uart16550 compatible device under PciRoot(0x0) and list in SCU.
  #              If device path string have "*", the Enalbe flag is invalid.)
  #    UINT8(0x00), L"OTHER"    , L"PciRoot(0x0)/*"
  #
  ## Example 3: (Using COMB to redirect console default, Uart and terminal setting are the same with device path nodes)
  #    UINT8(0x01), L"COM_B"    , L"PciRoot(0x0)/Pci(0x1F,0x0)/Serial(0x1)/Uart(57600,8,N,1)/VenVt100()"
  #
  #
  gInsydeCrTokenSpaceGuid.PcdH2OCrDevice|{  \
  #-------------------------------------------------------------------------------------------------------------
  #|  Enable   || DevName        || Device Path String
  #-------------------------------------------------------------------------------------------------------------
    UINT8(0x01), L"COM_A"         , L"PciRoot(0x0)/Pci(0x1F,0x0)/Serial(0x0)",  \
    UINT8(0x00), L"COM_B"         , L"PciRoot(0x0)/Pci(0x1F,0x0)/Serial(0x1)",  \
    UINT8(0x00), L"OTHER"         , L"PciRoot(0x0)/*",  \
    UINT8(0xFF), UINT8(0xFF)      , UINT8(0xFF) \
    }

  gInsydeCrTokenSpaceGuid.PcdH2OCrPolicy| { \
                        #    Flag Name          |  FALSE(0x00) |   TRUE(0x01)
                        #----------------------------------------------------
    UINT8(0x00),     \  # CRVideoType           |  Color       |  Mono
    UINT8(0x00),     \  # CRTerminalKey         |  Senseless   |  Sensitive
    UINT8(0x01),     \  # CRComboKey            |  Disable     |  Enable
    UINT8(0x00),     \  # Reserve (compatiable) |              |
    UINT8(0x00),     \  # Reserve (compatiable) |              |
    UINT8(0x01),     \  # CRUnKbcSupport        |  Disable     |  Enable
    UINT8(0x00),     \  # Reserve (compatiable) |              |
    UINT8(0x01),     \  # CRManualRefresh       |  Disable     |  Enable
    UINT8(0x01),     \  # CRTerminalCharSet     |  ASCII       |  Graphic
    UINT8(0x01),     \  # CRAsyncTerm           |  Auto        |  Force Async
    UINT8(0x00),     \  # CRScreenOffCheck      |  Disable     |  Enable
    UINT8(0x01),     \  # CRShowHelp            |  Disable     |  Enable
    UINT8(0x00),     \  # CRHeadlessVBuffer     |  Vbuffer     |  EBDA
    UINT8(0x00),     \  # Reserve (compatiable) |              |
    UINT8(0x00),     \  # Reserve (compatiable) |              |
    UINT8(0x00),     \  # Reserve (compatiable) |              |
    UINT8(0x00),     \  # CROpROMLoadOnESegment |  Disable     |  Enable
    UINT8(0xFF),     \  # End of feature flag (0xFF) add new feature flag need before this column.
                        #----------------------------------------------------
    UINT8(0x10),     \  # CRFifoLength          |  [Value]
    UINT8(0x00)}        # CRWriteCharInterval   |  [Value]

  ##
  ## ============ High speed uart support PCDs ==================
  ##

  #
  # High speed uart devices have other SerialClock and SampleRate
  # Please change this Pcd value
  #
  gInsydeCrTokenSpaceGuid.PcdH2OHsUartSerialClock|1843200
  gInsydeCrTokenSpaceGuid.PcdH2OHsUartSampleRate|16

  #
  # Device information for PciHsUart driver to support.
  # List can be extended and 0xFF indicates end of list.
  # Either Bus/Device/Function or Vendor ID/Device ID are necessary.
  # Zero value means "don't care" when either Bus/Device/Function or Vendor ID/Device ID are non-zero.
  # When both Bus/Device/Function and Vendor ID/Device ID are non-zero, device will be supported only when all of them are matched.
  #
  ## Example 1: (Connect PCI device (0/1E/00) to PciHsUart driver, ignore the VID and DID)
  #    UINT32(0x00), UINT32(0x1E), UINT32(0x00), UINT32(0x0000), UINT32(0x0000)
  #
  ## Example 2: (Connect PCI device (VID:8086 and DID 1234) to PciHsUart driver)
  #    UINT32(0x00), UINT32(0x00), UINT32(0x00), UINT32(0x8086), UINT32(0x1234)
  #
  ## Example 3: (Connect PCI device (0/19/0, VID:8086 and DID 4321)
  #    UINT32(0x00), UINT32(0x19), UINT32(0x00), UINT32(0x8086), UINT32(0x4321)
  #
  #                                                      Bus          Device      Function       Vendor ID       Device ID
  #                                                  ===========   ===========   ===========   =============   =============
  gInsydeCrTokenSpaceGuid.PcdH2OPciHsUartDeviceList|{UINT32(0xFF), UINT32(0xFF), UINT32(0xFF), UINT32(0xFFFF), UINT32(0xFFFF)} # All 0xFF indicates end of list.

  #
  # PCH MMIO Address list for PchHsUart driver to support.
  # List the Uart 0, Uart 1, ... MMIO address
  # List can be extended and 0xFF indicates end of list.
  # Enable = 0 : Disable this device
  # Enable = 1 : Enable this device
  # IRQ        : The device IRQ value for legacy boot. 0xff mean no IRQ value.
  #
  ## Example  : (Enable the Uart0 with baseAddress 0xFE030000, IRQ=4h.
  #              Enable the Uart1 with baseAddress 0xFE032000, No IRQ.
  #              Unsupport the third uart )
  #    UINT8(0x01), UINT32(0xFE030000), UINT8(0x04), \
  #    UINT8(0x01), UINT32(0xFE032000), UINT8(0xFF), \
  #    UINT8(0x00), UINT32(0xFE034000), UINT8(0xFF)
  #                                                      Enable          Address           IRQ
  #                                                    =========    =================   ==========
  gInsydeCrTokenSpaceGuid.PcdH2OPchHsUartMmioAddress|{ UINT8(0xFF), UINT32(0xFFFFFFFF), UINT8(0xFF) } # All 0xFF indicates end of list.

  #
  # PchHsUart driver Byte Address Control (PCICFGCTRL)
  # If platform didn't support this flag to control AccessMode.
  # Set PCD = 1  (Access8bit)
  # Set PCD = 0  (Access32bit)
  #
  gInsydeCrTokenSpaceGuid.PcdH2OSerialIoPchPcrAddress|0xFDCB0618


  ##
  ## ============ SOL support PCDs ==================
  ##

  #
  # SOL Len Configuration
  #
  gInsydeCrTokenSpaceGuid.PcdH2OSolDefaultSetting|{ UINT16(0),                                       \ # Reserve (This column config in Vfr)
                                                    UINT16(23),                                      \ # PortNumber;
                                                    UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0),  \ # LocalIp[4];
                                                    UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0),  \ # SubnetMask[4];
                                                    UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0),  \ # Gateway[4];
                                                    UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0),  \ # AdminIp[4];
                                                    UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT8(0x0), UINT64(0x0), UINT64(0x0), UINT64(0x0), UINT16(0x0), \ #AdminMac[32];
                                                    L"Insyde" ,   \  # UserName[13];
                                                    L"123" }         # Password[13];

  ##
  ## ============ Terminal support PCDs ==================
  ##

  #
  # Terminal want to send more data at onec, that can set this PCD to higher values.
  # That can give more bandwidth but should cause inconvenience in some instances (eg. SCU).
  #
  gInsydeCrTokenSpaceGuid.PcdH2OCrTerminalFifoSize|128

[PcdsDynamicExDefault]

#[-start-160315-IB08400335-modify]#
  #
  # Use Advanced formset GUID as default so that the Event Log Manager could be put under Advanced menu.
  #
  gInsydeCrTokenSpaceGuid.PcdH2OConsoleRedirectionFormsetClassGuid|{GUID("C6D4769E-7F48-4D2A-98E9-87ADCCF35CCC")}
#[-end-160315-IB08400335-modify]#

[Components.$(PEI_ARCH)]

[Components.$(DXE_ARCH)]

!if gInsydeCrTokenSpaceGuid.PcdH2OConsoleRedirectionSupported == 1 || gInsydeCrTokenSpaceGuid.PcdH2OAdvanceConsoleRedirectionSupported == 1
  #
  # Console Redirection Feature Flag
  #
  InsydeCrPkg/CrServiceDxe/CrServiceDxe.inf
  InsydeCrPkg/CrServiceSmm/CrServiceSmm.inf

  InsydeCrPkg/ConfigUtility/CrConfigUtilDxe/CrConfigUtil.inf
  InsydeCrPkg/Uart16550SerialDxe/Uart16550SerialDxe.inf
#[-start-160826-IB07400773-modify]#
!if $(INSYDE_DEBUGGER) == YES AND $(H2O_DDT_DEBUG_IO) == Com
!else
  InsydeCrPkg/Uart16550Devices/IsaUartDxe/IsaUartDxe.inf
!endif  
#[-end-160826-IB07400773-modify]#

  !if gInsydeCrTokenSpaceGuid.PcdH2OCRHsuSerialSupported == 1
    InsydeCrPkg/Uart16550Devices/PciHsUartDxe/PciHsUartDxe.inf
    InsydeCrPkg/Uart16550Devices/PchHsUartDxe/PchHsUartDxe.inf
  !endif

  !if gInsydeCrTokenSpaceGuid.PcdH2OCRPciSerialSupported == 1
    InsydeCrPkg/Uart16550Devices/PciUartDxe/PciUartDxe.inf
  !endif

  !if gInsydeCrTokenSpaceGuid.PcdH2OCRUsbSerialSupported == 1 || gInsydeCrTokenSpaceGuid.PcdH2OAdvanceConsoleRedirectionSupported == 1
    InsydeCrPkg/UsbSerialDxe/UsbSerialDxe.inf
    InsydeCrPkg/UsbDevices/CrPl2303Dxe/CrPl2303Dxe.inf

    !if gInsydeTokenSpaceGuid.PcdH2ODdtSupported == 0
      InsydeCrPkg/UsbDevices/CrDdtCableDxe/CrDdtCableDxe.inf
    !endif
  !endif

  #
  # Advance Console Redirection Feature Flag
  #
  !if gInsydeCrTokenSpaceGuid.PcdH2OCRTelnetSupported == 1 || gInsydeCrTokenSpaceGuid.PcdH2OAdvanceConsoleRedirectionSupported == 1
    InsydeCrPkg/AdvanceTerminalDxe/AdvanceTerminalDxe.inf
    InsydeCrPkg/SolDxe/SolDxe.inf
    InsydeCrPkg/ConfigUtility/SolConfigUtility/SolConfigUtil.inf
    InsydeCrPkg/TelnetDxe/TelnetDxe.inf

    !if gInsydeTokenSpaceGuid.PcdH2ONetworkIscsiSupported == 0
      InsydeNetworkPkg/Drivers/TcpDxe/TcpDxe.inf
    !endif
  !else
    InsydeCrPkg/TerminalDxe/TerminalDxe.inf
  !endif

  !if gInsydeCrTokenSpaceGuid.PcdH2OCRSrvManagerSupported == 1 || gInsydeCrTokenSpaceGuid.PcdH2OAdvanceConsoleRedirectionSupported == 1
    InsydeCrPkg/CrSrvManagerDxe/CrSrvManagerDxe.inf
    InsydeCrPkg/CrBiosFlashDxe/CrBiosFlashDxe.inf
    InsydeCrPkg/CrFileTransferDxe/CrFileTransferDxe.inf
    InsydeCrPkg/FileSelectUIDxe/FileSelectUIDxe.inf
  !endif

!endif

