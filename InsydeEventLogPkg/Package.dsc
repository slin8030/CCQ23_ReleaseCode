## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

[Defines]
  EDK_GLOBAL  INSYDE_EVENTLOG_PKG          = InsydeEventLogPkg

[LibraryClasses]
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2OBaseDebugPrintErrorLevelLib/H2OBaseDebugPrintErrorLevelLib.inf
!endif

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugCfgToErrLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2ODebugCfgToErrLevelLib/H2ODebugCfgToErrLevelLib.inf
!endif

  EventLogGetTimeFromCmosLib|$(INSYDE_EVENTLOG_PKG)/Library/EventLogGetTimeFromCmosLib/EventLogGetTimeFromCmosLib.inf

[LibraryClasses.common.SEC]

[LibraryClasses.common.PEI_CORE]
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2OPeiDebugPrintErrorLevelLib/H2OPeiDebugPrintErrorLevelLib.inf
!endif

[LibraryClasses.common.PEIM]
  EventLogPeiLib|InsydeEventLogPkg/Library/EventLogPeiLib/EventLogPeiLib.inf
  
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2OPeiDebugPrintErrorLevelLib/H2OPeiDebugPrintErrorLevelLib.inf
!endif

[LibraryClasses.common.DXE_CORE]
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2ODxeDebugPrintErrorLevelLib/H2ODxeDebugPrintErrorLevelLib.inf
!endif

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2ODxeDebugPrintErrorLevelLib/H2ODxeDebugPrintErrorLevelLib.inf
!endif

[LibraryClasses.common.UEFI_DRIVER]
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2ODxeDebugPrintErrorLevelLib/H2ODxeDebugPrintErrorLevelLib.inf
!endif

[LibraryClasses.common.DXE_DRIVER]
  EventLogDxeLib|InsydeEventLogPkg/Library/EventLogDxeLib/EventLogDxeLib.inf

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2ODxeDebugPrintErrorLevelLib/H2ODxeDebugPrintErrorLevelLib.inf
#[-start-171225-IB08400545-add]#
  EventLogDebugMsgLib|$(INSYDE_EVENTLOG_PKG)/Library/EventLogDebugMsgLib/EventLogDebugMsgLib.inf
#[-end-171225-IB08400545-add]#
!endif

[LibraryClasses.common.DXE_SMM_DRIVER]
  EventLogSmmLib|InsydeEventLogPkg/Library/EventLogSmmLib/EventLogSmmLib.inf

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  EventLogDebugMsgLib|$(INSYDE_EVENTLOG_PKG)/Library/EventLogDebugMsgLib/EventLogDebugMsgLib.inf
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2OSmmDebugPrintErrorLevelLib/H2OSmmDebugPrintErrorLevelLib.inf
!endif

[LibraryClasses.common.COMBINED_SMM_DXE]
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2OSmmDebugPrintErrorLevelLib/H2OSmmDebugPrintErrorLevelLib.inf
!endif

[LibraryClasses.common.SMM_CORE]
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2OSmmDebugPrintErrorLevelLib/H2OSmmDebugPrintErrorLevelLib.inf
!endif

[LibraryClasses.common.UEFI_APPLICATION]
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  DebugPrintErrorLevelLib|$(INSYDE_EVENTLOG_PKG)/Library/H2ODxeDebugPrintErrorLevelLib/H2ODxeDebugPrintErrorLevelLib.inf
!endif

[PcdsFeatureFlag]
#[-start-171212-IB08400542-modify]#
  # FOR DXE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OStoreDebugMsgSupported|FALSE
  # FOR PEI
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OStoreDebugMsgPeiSupported|FALSE
  # For logging Event to BIOS GPNV Region.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageSupported|TRUE
  # For logging Event to BMC SEL.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBmcSelEventStorageSupported|FALSE
  # Display logged events on VGA/Console during POST.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowEventsOnPostDisplaySupported|FALSE
  # SCU Manager for all of the Event Log related Setup drivers. 
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogConfigManagerSupported|TRUE
  # Enable the Post Message support.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OPostMessageSupported|TRUE
#[-end-171212-IB08400542-modify]#
  # Enable the EFI DEBUG message control driver support.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported|TRUE
  # For using Variable as BIOS Event Storage.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageUseVariableStorageSupported|FALSE

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageUseVariableStorageSupported
  # For using GPNV region as BIOS Event Storage.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageUseGpnvRegionStorageSupported|FALSE
!else
  # For using GPNV region as BIOS Event Storage.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageUseGpnvRegionStorageSupported|TRUE
!endif

[PcdsFixedAtBuild]
  #
  # Display logged events on VGA/Console during POST.
  # PcdH2OShowEventsOnPostDisplaySupported !V When true, displays logged events to the VGA/console.
  # PcdH2OEventsOnPostDisplayFilterList !V Selects which event types to display on the VGA/console.
  #
  # Storage ID Bitmap: (The Storage ID can be ORed.)
  #   BIOS: 0x01, BMC SEL: 0x02, Memory: 0x04
  # Format:
  #  StorageId (Bitmap), EventID (SensorType, SensorNum, EventType)
  #   See Platform Event Message from IPMI spec for definition of Sensor Type, SensorNum, EventType.
  #   For BIOS generated events use SensorNum 0x80.
  #
  # Ex:
  #  0x01, 0x11, 0x22, 0x33         // BIOS storage   : Filter EventID => 0x11, 0x22, 0x33
  #  0x02, 0x0F, 0x00, 0x6F         // BMC SEL storage: Filter EventID => 0x0F, 0x00, 0x6F
  #  0x07, 0x0F, 0x00, 0x6F         // BIOS, BMC SEL, Memory storages: Filter EventID => 0x0F, 0x00, 0x6F
  #
#[-start-171212-IB08400542-modify]#
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventsOnPostDisplayFilterList|{ \
    0x01, 0x0C, 0x80, 0x6F,         \ # BIOS storages : Filter EventID => 0x0C, 0x00, 0x6F (Memory)
    0x01, 0x07, 0x80, 0x6F,         \ # BIOS storages : Filter EventID => 0x07, 0x80, 0x6F (CPU)
    0x01, 0x0F, 0x00, 0x6F,         \ # BIOS storages : Filter EventID => 0x0F, 0x00, 0x6F (POST)
    0x00, 0x00, 0x00, 0x00}           # End Entry.	  
#[-end-171212-IB08400542-modify]#

  # Show Status Code serial message.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowStatusCodeSerialMsgControl|TRUE
  # Determine if need to clear all of the Events in the Variable for the BIOS Event Storage feature.
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageUseVariableClearEventByAllOrders|FALSE

#[-start-180531-IB08400600-add]#
  #
  # Use Advanced formset GUID as default so that the Event Log Manager could be put under Advanced menu.
  #
#[-start-160315-IB08400335-modify]#
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogManagerFormsetClassGuid|{GUID("C6D4769E-7F48-4D2A-98E9-87ADCCF35CCC")}
#[-end-160315-IB08400335-modify]#
#[-end-180531-IB08400600-add]#

[PcdsDynamicDefault]

[PcdsDynamicExDefault]

#[-start-171212-IB08400542-modify]#
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowStatusCodeSerialMsgControl
  #
  # Show Status Code serial message, default is disabled.
  #
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowProgressCodeSerialMsgEnable|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowErrorCodeSerialMsgEnable|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowDebugCodeSerialMsgEnable|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowStatusCodeSerialMsgReady|FALSE
!else
  #
  # Show Status Code serial message, default is enabled as MDE code base.
  #
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowProgressCodeSerialMsgEnable|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowErrorCodeSerialMsgEnable|TRUE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowDebugCodeSerialMsgEnable|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowStatusCodeSerialMsgReady|TRUE
!endif
#[-end-171212-IB08400542-modify]#

[Components.$(PEI_ARCH)]

#[-start-180824-IB07400999-modify]#
#[-start-171212-IB08400542-modify]#
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogSupported
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageSupported
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BiosStorage/BiosStoragePei/BiosStoragePei.inf {
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  }
!endif

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogSupported
  $(INSYDE_EVENTLOG_PKG)/EventLog/EventLogPei/EventLogPei.inf {
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  }
!endif  

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OPostMessageSupported
  $(INSYDE_EVENTLOG_PKG)/PostMessage/PostMessagePei/PostMessagePei.inf {
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  }
!endif

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OBmcSelEventStorageSupported
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BmcSelStorage/BmcSelStoragePei/BmcSelStoragePei.inf {
    <LibraryClasses>
      DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  }
!endif
#[-end-171212-IB08400542-modify]#

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported  
  $(INSYDE_EVENTLOG_PKG)/DebugMessage/DebugMaskPei/DebugMaskPei.inf
!endif  
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OStoreDebugMsgSupported 
  $(INSYDE_EVENTLOG_PKG)/DebugMessage/StoreDebugMessagePei/StoreDebugMessagePei.inf
!endif  

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  $(INSYDE_EVENTLOG_PKG)/StatusCodeHandler/Pei/H2OStatusCodeHandlerPei.inf
!endif
!endif
#[-end-180824-IB07400999-modify]#

[Components.$(DXE_ARCH)]

#[-start-171212-IB08400542-modify]#
#[-start-180823-IB07400999-add]#
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogSupported
#[-end-180823-IB07400999-add]#
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageSupported
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageUseVariableStorageSupported
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BiosStorage/BiosStorageDxe/BiosStorageKernelUseVariableDxe.inf
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BiosStorage/BiosStorageSmm/BiosStorageKernelUseVariableSmm.inf
!else
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BiosStorage/BiosStorageDxe/BiosStorageKernelDxe.inf {
    <LibraryClasses>
      S3BootScriptLib|MdePkg/Library/BaseS3BootScriptLibNull/BaseS3BootScriptLibNull.inf
  }
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BiosStorage/BiosStorageSmm/BiosStorageKernelSmm.inf {
    <LibraryClasses>
      S3BootScriptLib|MdePkg/Library/BaseS3BootScriptLibNull/BaseS3BootScriptLibNull.inf
  }
!endif

  $(INSYDE_EVENTLOG_PKG)/EventStorage/BiosStorage/BiosStorageSmm/BiosStorageChipsetSmm.inf 
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BiosStorage/BiosStorageDxe/BiosStorageChipsetDxe.inf 
!endif

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogSupported
  $(INSYDE_EVENTLOG_PKG)/EventLog/EventLogDxe/EventLogDxe.inf
  $(INSYDE_EVENTLOG_PKG)/EventLog/EventLogSmm/EventLogSmm.inf
  
#[-start-180823-IB07400999-modify]#
!if $(H2O_MEMORY_EVENT_STORAGE_SUPPORTED) == YES
  $(INSYDE_EVENTLOG_PKG)/EventStorage/MemoryStorage/MemoryStorageDxe/MemoryStorageKernelDxe.inf 
  $(INSYDE_EVENTLOG_PKG)/EventStorage/MemoryStorage/MemoryStorageDxe/MemoryStorageChipsetDxe.inf 

  $(INSYDE_EVENTLOG_PKG)/EventStorage/MemoryStorage/MemoryStorageSmm/MemoryStorageKernelSmm.inf 
  $(INSYDE_EVENTLOG_PKG)/EventStorage/MemoryStorage/MemoryStorageSmm/MemoryStorageChipsetSmm.inf 
!endif
#[-end-180823-IB07400999-modify]#
  
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OBmcSelEventStorageSupported  
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BmcSelStorage/BmcSelEventStorageDxe/BmcSelEventStorageDxe.inf
  $(INSYDE_EVENTLOG_PKG)/EventStorage/BmcSelStorage/BmcSelEventStorageSmm/BmcSelEventStorageSmm.inf 
  $(INSYDE_EVENTLOG_PKG)/BmcSelStringTranslationDxe/BmcSelStringTranslationDxe.inf
!endif  

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogConfigManagerSupported
  $(INSYDE_EVENTLOG_PKG)/ConfigUtility/H2OEventLogConfigManagerDxe/H2OEventLogConfigManagerDxe.inf
  $(INSYDE_EVENTLOG_PKG)/ConfigUtility/BiosEventLogConfigUtilDxe/BiosEventLogConfigUtilDxe.inf 
  !if gH2OEventLogPkgTokenSpaceGuid.PcdH2OPostMessageSupported  
    $(INSYDE_EVENTLOG_PKG)/ConfigUtility/PostMessageConfigUtilDxe/PostMessageConfigUtilDxe.inf
  !endif  
  !if gInsydeTokenSpaceGuid.PcdH2OSetupChangeDisplaySupported
    $(INSYDE_EVENTLOG_PKG)/ConfigUtility/SetupChangeConfigUtilDxe/SetupChangeConfigUtilDxe.inf
  !endif  
  !if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported  
    $(INSYDE_EVENTLOG_PKG)/ConfigUtility/DebugMessageConfigUtilDxe/DebugMessageConfigUtilDxe.inf
  !endif
!endif  
  
!endif  

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OPostMessageSupported
  $(INSYDE_EVENTLOG_PKG)/PostMessage/PostMessageDxe/PostMessageDxe.inf
  $(INSYDE_EVENTLOG_PKG)/PostMessage/PostMessageSmm/PostMessageSmm.inf
!endif

  #
  # Store the DEBUG() in memory space, user can dump it by tool(Ex:H2OELV (shell and DOS tool).).
  #
!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OStoreDebugMsgSupported  
  $(INSYDE_EVENTLOG_PKG)/DebugMessage/StoreDebugMessageDxe/StoreDebugMessageDxe.inf
  $(INSYDE_EVENTLOG_PKG)/DebugMessage/StoreDebugMessageSmm/StoreDebugMessageSmm.inf
!endif

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowEventsOnPostDisplaySupported
 $(INSYDE_EVENTLOG_PKG)/EventLogPostDisplayDxe/EventLogPostDisplayDxe.inf
!endif 
#[-end-171212-IB08400542-modify]#

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  $(INSYDE_EVENTLOG_PKG)/StatusCodeHandler/RuntimeDxe/H2OStatusCodeHandlerRuntimeDxe.inf
!endif

!if gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported
  $(INSYDE_EVENTLOG_PKG)/StatusCodeHandler/Smm/H2OStatusCodeHandlerSmm.inf
!endif
#[-start-180823-IB07400999-add]#
!endif  
#[-end-180823-IB07400999-add]#


