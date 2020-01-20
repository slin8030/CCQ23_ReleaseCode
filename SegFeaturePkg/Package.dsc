## @file
# Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2012 - 2018, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************
#

[Defines]
  !include SegFeaturePkg/Package.env

[PcdsFeatureFlag]

[LibraryClasses]
#[-start-180309-IB09330437-add]#
  IpmiSyncBmcLib|SegFeaturePkg/Library/IpmiSyncBmcLibNull/IpmiSyncBmcLibNull.inf
#[-end-180309-IB09330437-add]#
#[-start-180612-IB09330462-add]#
  SegPlatformSupportLib|SegFeaturePkg/Library/SegPlatformSupportLib/SegPlatformSupportLib.inf
#[-end-180612-IB09330462-add]#
[LibraryClasses.common.SEC]

[LibraryClasses.common.PEI_CORE]

[LibraryClasses.common.PEIM]
#[-start-150526-IB08400297-add]#
  EventLogPeiLib|SegFeaturePkg/Library/EventLogPeiLibNull/EventLogPeiLibNull.inf
#[-end-150526-IB08400297-add]#
#[-start-150729-IB10130224-add]#
  PeiBtsLib|SegFeaturePkg/Library/PeiBtsLibNull/PeiBtsLibNull.inf
  PeiBtsLogSaveLib|SegFeaturePkg/Library/PeiBtsLogSaveLibNull/PeiBtsLogSaveLibNull.inf
#[-end-150729-IB10130224-add]#
#[-start-161121-IB09330323-add]#
  PeiIpmiBmcFeatureLib|SegFeaturePkg/Library/PeiIpmiBmcFeatureLibNull/PeiIpmiBmcFeatureLibNull.inf
#[-end-161121-IB09330323-add]#
#[-start-170704-IB05820440-add]#
  GraphicVgaLib|SegFeaturePkg/Library/PeiGraphicVgaLibNull/PeiGraphicVgaLibNull.inf
#[-end-170704-IB05820440-add]#
#[-start-171207-IB09330405-add]#
  IpmiInterfaceLib|SegFeaturePkg/Library/PeiIpmiInterfaceLibNull/PeiIpmiInterfaceLibNull.inf
  IpmiInterfaceLib2|SegFeaturePkg/Library/PeiIpmiInterfaceLibNull2/PeiIpmiInterfaceLibNull2.inf
#[-end-171207-IB09330405-add]#
#[-start-171228-IB09330412-add]#
  IpmiOemFeatureLib|SegFeaturePkg/Library/PeiIpmiOemFeatureLibNull/PeiIpmiOemFeatureLibNull.inf
#[-end-171228-IB09330412-add]#
[LibraryClasses.common.DXE_CORE]
#[-start-180326-IB15760032-add]#
  MemoryDataAnalysisLib|SegFeaturePkg/Library/MemoryDataAnalysisLibNull/MemoryDataAnalysisLibNull.inf
#[-end-180326-IB15760032-add]#

[LibraryClasses.common.DXE_RUNTIME_DRIVER]

[LibraryClasses.common.UEFI_DRIVER]

[LibraryClasses.common.DXE_DRIVER]
#[-start-150526-IB08400297-add]#
  EventLogDxeLib|SegFeaturePkg/Library/EventLogDxeLibNull/EventLogDxeLibNull.inf
#[-end-150526-IB08400297-add]#
  IpmiInterfaceLib|SegFeaturePkg/Library/DxeIpmiInterfaceLibNull/DxeIpmiInterfaceLibNull.inf
  IpmiSdrLib|SegFeaturePkg/Library/DxeIpmiSdrLibNull/DxeIpmiSdrLibNull.inf
  IpmiFruLib|SegFeaturePkg/Library/DxeIpmiFruLibNull/DxeIpmiFruLibNull.inf
  IpmiSelDataLib|SegFeaturePkg/Library/DxeIpmiSelDataLibNull/DxeIpmiSelDataLibNull.inf
  IpmiSelInfoLib|SegFeaturePkg/Library/DxeIpmiSelInfoLibNull/DxeIpmiSelInfoLibNull.inf
#[-start-150729-IB10130224-add]#
#[-start-161121-IB09330323-add]#
  IpmiSyncBmcLib|SegFeaturePkg/Library/IpmiSyncBmcLibNull/IpmiSyncBmcLibNull.inf
#[-end-161121-IB09330323-add]#
  DxeBtsLib|SegFeaturePkg/Library/DxeBtsLibNull/DxeBtsLibNull.inf
  DxeBtsLogSaveLib|SegFeaturePkg/Library/DxeBtsLogSaveLibNull/DxeBtsLogSaveLibNull.inf
#[-end-150729-IB10130224-add]#
#[-start-161121-IB09330323-add]#
  IpmiBmcFeatureOemLib|SegFeaturePkg/Library/IpmiBmcFeatureOemLibNull/IpmiBmcFeatureOemLibNull.inf
#[-end-161121-IB09330323-add]#
#[-start-171225-IB08400545-add]#
  EventLogDebugMsgLib|SegFeaturePkg/Library/EventLogDebugMsgLibNull/EventLogDebugMsgLibNull.inf
#[-end-171225-IB08400545-add]#

#[-start-170609-IB09330361-add]#
  IpmiInterfaceLib2|SegFeaturePkg/Library/DxeIpmiInterfaceLibNull2/DxeIpmiInterfaceLibNull2.inf
  IpmiSdrLib2|SegFeaturePkg/Library/DxeIpmiSdrLibNull2/DxeIpmiSdrLibNull2.inf
  IpmiFruLib2|SegFeaturePkg/Library/DxeIpmiFruLibNull2/DxeIpmiFruLibNull2.inf
  IpmiSelDataLib2|SegFeaturePkg/Library/DxeIpmiSelDataLibNull2/DxeIpmiSelDataLibNull2.inf
  IpmiSelInfoLib2|SegFeaturePkg/Library/DxeIpmiSelInfoLibNull2/DxeIpmiSelInfoLibNull2.inf
#[-end-170609-IB09330361-add]#
#[-start-170615-IB09330362-add]#
  IpmiBmcFeatureGetDataLib|SegFeaturePkg/Library/IpmiBmcFeatureGetDataLibNull/IpmiBmcFeatureGetDataLibNull.inf
#[-end-170615-IB09330362-add]#
#[-start-170704-IB05820440-add]#
  GraphicVgaLib|SegFeaturePkg/Library/DxeGraphicVgaLibNull/DxeGraphicVgaLibNull.inf
#[-end-170704-IB05820440-add]#
#[-start-171228-IB09330412-add]#
  IpmiOemFeatureLib|SegFeaturePkg/Library/DxeIpmiOemFeatureLibNull/DxeIpmiOemFeatureLibNull.inf
#[-end-171228-IB09330412-add]#
#[-start-180326-IB15760032-add]#
  MemoryDataAnalysisLib|SegFeaturePkg/Library/MemoryDataAnalysisLibNull/MemoryDataAnalysisLibNull.inf
#[-end-180326-IB15760032-add]#

[LibraryClasses.common.DXE_SMM_DRIVER]
  EventLogDebugMsgLib|SegFeaturePkg/Library/EventLogDebugMsgLibNull/EventLogDebugMsgLibNull.inf

#[-start-150526-IB08400297-add]#
  EventLogSmmLib|SegFeaturePkg/Library/EventLogSmmLibNull/EventLogSmmLibNull.inf
#[-end-150526-IB08400297-add]#
#[-start-150527-IB04930579-add]#
  SmmWheaPlatformLib|SegFeaturePkg/Library/SmmWheaPlatformLibNull/SmmWheaPlatformLibNull.inf
#[-end-150527-IB04930579-add]#
  MediaPlayerDxeLib|SegFeaturePkg/Library/MediaPlayerDxeLibNull/MediaPlayerDxeLibNull.inf
#[-start-170704-IB05820440-add]#
  GraphicVgaLib|SegFeaturePkg/Library/DxeGraphicVgaLibNull/DxeGraphicVgaLibNull.inf
#[-end-170704-IB05820440-add]#
#[-start-170718-IB09330369-add]#
  IpmiInterfaceLib|SegFeaturePkg/Library/SmmIpmiInterfaceLibNull/SmmIpmiInterfaceLibNull.inf
  IpmiSdrLib|SegFeaturePkg/Library/SmmIpmiSdrLibNull/SmmIpmiSdrLibNull.inf
  IpmiFruLib|SegFeaturePkg/Library/SmmIpmiFruLibNull/SmmIpmiFruLibNull.inf
  IpmiSelDataLib|SegFeaturePkg/Library/SmmIpmiSelDataLibNull/SmmIpmiSelDataLibNull.inf
  IpmiSelInfoLib|SegFeaturePkg/Library/SmmIpmiSelInfoLibNull/SmmIpmiSelInfoLibNull.inf
  IpmiInterfaceLib2|SegFeaturePkg/Library/SmmIpmiInterfaceLib2Null/SmmIpmiInterfaceLib2Null.inf
  IpmiSdrLib2|SegFeaturePkg/Library/SmmIpmiSdrLib2Null/SmmIpmiSdrLib2Null.inf
  IpmiFruLib2|SegFeaturePkg/Library/SmmIpmiFruLib2Null/SmmIpmiFruLib2Null.inf
  IpmiSelDataLib2|SegFeaturePkg/Library/SmmIpmiSelDataLib2Null/SmmIpmiSelDataLib2Null.inf
  IpmiSelInfoLib2|SegFeaturePkg/Library/SmmIpmiSelInfoLib2Null/SmmIpmiSelInfoLib2Null.inf
#[-end-170718-IB09330369-add]#
#[-start-171228-IB09330412-add]#
  IpmiOemFeatureLib|SegFeaturePkg/Library/SmmIpmiOemFeatureLibNull/SmmIpmiOemFeatureLibNull.inf
#[-end-171228-IB09330412-add]#
#[-start-180326-IB15760032-add]#
  MemoryDataAnalysisLib|SegFeaturePkg/Library/MemoryDataAnalysisLibNull/MemoryDataAnalysisLibNull.inf
#[-end-180326-IB15760032-add]#

[LibraryClasses.common.COMBINED_SMM_DXE]

[LibraryClasses.common.SMM_CORE]
#[-start-180326-IB15760032-add]#
  MemoryDataAnalysisLib|SegFeaturePkg/Library/MemoryDataAnalysisLibNull/MemoryDataAnalysisLibNull.inf
#[-end-180326-IB15760032-add]#

[LibraryClasses.common.UEFI_APPLICATION]

#
# LibraryClasses for OS Tool Modules
#
[LibraryClasses.common.USER_DEFINED]

[PcdsFeatureFlag]

[PcdsFixedAtBuild]
#[-start-170509-IB15760003-add]#
  ## Serial Port Configuration Array
  #
  #  Each element in the array is represented by a specific serial port settings.
  #  The fields of element are described as follows.
  #
  #######################################
  #  RegBase : Serial Port Regiser Base
  #######################################
  #
  #  Base address of 16550 serial port registers.
  #
  #  COM 1 default is 0x03F8.
  #  COM 2 default is 0x02F8.
  #
  #######################################
  #  UIOS : Use I/O Space    
  #######################################
  #
  #  Indicates the 16550 serial port registers are in MMIO space, or in I/O space.
  #    1 - 16550 serial port registers are in I/O space.
  #    0 - 16550 serial port registers are in MMIO space.
  #
  #  Default is I/O space.
  #
  #######################################
  #  BaudRate : Serial Port Baud Rate
  #######################################
  #
  #  Baud rate for the 16550 serial port.
  #
  #  Default is 115200.
  #
  #######################################
  #  LC : Line Control      
  #######################################
  #
  #  Line Control Register (LCR) for the 16550 serial port. This encodes data bits, parity, and stop bits.
  #    BIT1..BIT0 - Data bits.  00b = 5 bits, 01b = 6 bits, 10b = 7 bits, 11b = 8 bits.
  #    BIT2       - Stop Bits.  0 = 1 stop bit.  1 = 1.5 stop bits if 5 data bits selected, otherwise 2 stop bits.
  #    BIT5..BIT3 - Parity.  xx0b = No Parity, 001b = Odd Parity, 011b = Even Parity, 101b = Mark Parity, 111b=Stick Parity.
  #    BIT7..BIT6 - Reserved.  Must be 0.
  #
  #  Default is No Parity, 8 Data Bits, 1 Stop Bit
  #
  #######################################
  #  FC : FIFO Control    
  #######################################
  #
  #  FIFO Control Register (FCR) for the 16550 serial port.
  #    BIT0       - FIFO Enable.  0 = Disable FIFOs.  1 = Enable FIFOs.
  #    BIT1       - Clear receive FIFO.  1 = Clear FIFO.
  #    BIT2       - Clear transmit FIFO.  1 = Clear FIFO.
  #    BIT4..BIT3 - Reserved.  Must be 0.
  #    BIT5       - Enable 64-byte FIFO.  0 = Disable 64-byte FIFO.  1 = Enable 64-byte FIFO
  #    BIT7..BIT6 - Reserved.  Must be 0.
  #
  #  Default is to enable and clear all FIFOs.
  #
  #######################################
  #  DC : Detect Cable  
  #######################################
  #
  #  Indicates if the 16550 serial Tx operations will be blocked if DSR is not asserted (no cable).
  #  This field is ignored if UHFC is 0.
  #    1 - 16550 serial Tx operations will be blocked if DSR is not asserted.
  #    0 - 16550 serial Tx operations will not be blocked if DSR is not asserted.
  #
  #  Default is 0.
  #
  #######################################
  #  UHFC : Use Hardware Flow Control    
  #######################################
  #
  #  Indicates if the 16550 serial port hardware flow control will be enabled.
  #    1 - 16550 serial port hardware flow control will be enabled.
  #    0 - 16550 serial port hardware flow control will be disabled.
  #
  #  Default is 0.
  #
  gSegFeatureTokenSpaceGuid.PcdH2OSerialPortConfig|{ \
    # RegBase     | UIOS | BaudRate      | LC  | FC  | DC | UHFC 
    #-----------------------------------------------------------------------------
    UINT64(0x03F8),     1, UINT32(115200), 0x03, 0x07,   0,     0,   \ # Com 1
    UINT64(0x02F8),     1, UINT32(115200), 0x03, 0x07,   0,     0,   \ # Com 2
    UINT64(0)     ,     0, UINT32(0)     , 0x00, 0x00,   0,     0    \ # End Entry
  }
#[-end-170509-IB15760003-add]#
#[-start-180612-IB09330462-add]#
  gSegFeatureTokenSpaceGuid.PcdH2OSegPlatformSupportBmcUsbDevicePath|"PciRoot(0x0)/Pci(0x14,0x0)/USB(0x1,0x0)"
#[-end-180612-IB09330462-add]#

[PcdsDynamicDefault]

[PcdsDynamicExDefault]

[Components.$(PEI_ARCH)]


[Components.$(DXE_ARCH)]


