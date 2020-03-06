[Defines]
  !include $(COMPAL_COMMON_PATH)/CompalCommon.env
[BuildOptions]
#[PRJ]++ >>>> Modify for support  VirtualEEPROMVerifyTool and CMFCVerify  
  DEFINE EC_PLATFORM_ASL_SETTING     = /D EC_PLATFORM_SETTING=0 
  DEFINE ECRAM_OFFSET_SETTING     = /D ECRAM_OFFSET=$(ECRAM_OFFSET_ADDRESS)
  DEFINE COMPAL_ASL_FEATURE_BUILD_OPTIONS = $(EC_PLATFORM_ASL_SETTING) \
                                            $(ECRAM_OFFSET_SETTING)
  *_*_*_ASLPP_FLAGS   = $(COMPAL_ASL_FEATURE_BUILD_OPTIONS)
#[COM] Compal Common code - Start
      !if $(COMPAL_COMMON_CODE_SUPPORT) == YES
        DEFINE COMPAL_COMMON_CODE_SUPPORT_OPTIONS         = /D COMPAL_COMMON_CODE_SUPPORT
        DEFINE COMPAL_COMMON_CODE_SUPPORT_OPTIONS_GCC     = /D COMPAL_COMMON_CODE_SUPPORT
      !else
        DEFINE COMPAL_COMMON_CODE_SUPPORT_OPTIONS         =
        DEFINE COMPAL_COMMON_CODE_SUPPORT_OPTIONS_GCC     =
      !endif
      !if $(COMPAL_EC_SUPPORT) == YES
        DEFINE COMPAL_EC_SUPPORT_OPTIONS                  = /D COMPAL_EC_SUPPORT
        DEFINE COMPAL_EC_SUPPORT_OPTIONS_GCC              = /D COMPAL_EC_SUPPORT
      !else
        DEFINE COMPAL_EC_SUPPORT_OPTIONS                  =
        DEFINE COMPAL_EC_SUPPORT_OPTIONS_GCC              =
      !endif
      !if $(COMPAL_GLOBAL_NVS_SUPPORT) == YES
        DEFINE COMPAL_GLOBAL_NVS_SUPPORT_OPTIONS          = /D COMPAL_GLOBAL_NVS_SUPPORT
        DEFINE COMPAL_GLOBAL_NVS_SUPPORT_OPTIONS_GCC      = /D COMPAL_GLOBAL_NVS_SUPPORT
      !else
        DEFINE COMPAL_GLOBAL_NVS_SUPPORT_OPTIONS          =
        DEFINE COMPAL_GLOBAL_NVS_SUPPORT_OPTIONS_GCC      =
      !endif
      !if $(COMPAL_ASL_SUPPORT) == YES
        DEFINE COMPAL_ASL_SUPPORT_OPTIONS                 = /D COMPAL_ASL_SUPPORT
        DEFINE COMPAL_ASL_SUPPORT_OPTIONS_GCC             = /D COMPAL_ASL_SUPPORT
      !else
        DEFINE COMPAL_ASL_SUPPORT_OPTIONS                 =
        DEFINE COMPAL_ASL_SUPPORT_OPTIONS_GCC             =
      !endif
      !if $(COMPAL_EEPROM_SUPPORT) == YES
        DEFINE COMPAL_EEPROM_SUPPORT_OPTIONS              = /D COMPAL_EEPROM_SUPPORT
        DEFINE COMPAL_EEPROM_SUPPORT_OPTIONS_GCC          = /D COMPAL_EEPROM_SUPPORT
      !else
        DEFINE COMPAL_EEPROM_SUPPORT_OPTIONS              =
        DEFINE COMPAL_EEPROM_SUPPORT_OPTIONS_GCC          =
      !endif
      !if $(COMPAL_CMFC_SUPPORT) == YES
        DEFINE COMPAL_CMFC_SUPPORT_OPTIONS                = /D COMPAL_CMFC_SUPPORT
        DEFINE COMPAL_CMFC_SUPPORT_OPTIONS_GCC            = /D COMPAL_CMFC_SUPPORT
      !else
        DEFINE COMPAL_CMFC_SUPPORT_OPTIONS                =
        DEFINE COMPAL_CMFC_SUPPORT_OPTIONS_GCC            =
      !endif
      !if $(COMPAL_THERMAL_SUPPORT) == YES
        DEFINE COMPAL_THERMAL_SUPPORT_OPTIONS             = /D COMPAL_THERMAL_SUPPORT
        DEFINE COMPAL_THERMAL_SUPPORT_OPTIONS_GCC         = /D COMPAL_THERMAL_SUPPORT
      !else
        DEFINE COMPAL_THERMAL_SUPPORT_OPTIONS             =
        DEFINE COMPAL_THERMAL_SUPPORT_OPTIONS_GCC         =
      !endif
      !if $(COMPAL_FLASH_SUPPORT) == YES
        DEFINE COMPAL_FLASH_SUPPORT_OPTIONS               = /D COMPAL_FLASH_SUPPORT
        DEFINE COMPAL_FLASH_SUPPORT_OPTIONS_GCC           = /D COMPAL_FLASH_SUPPORT
      !else
        DEFINE COMPAL_FLASH_SUPPORT_OPTIONS               =
        DEFINE COMPAL_FLASH_SUPPORT_OPTIONS_GCC           =
      !endif
      !if $(COMPAL_VAREEPROM_SUPPORT) == YES
        DEFINE COMPAL_VAREEPROM_SUPPORT_OPTIONS           = /D COMPAL_VAREEPROM_SUPPORT
        DEFINE COMPAL_VAREEPROM_SUPPORT_OPTIONS_GCC       = /D COMPAL_VAREEPROM_SUPPORT
      !else
        DEFINE COMPAL_VAREEPROM_SUPPORT_OPTIONS           =
        DEFINE COMPAL_VAREEPROM_SUPPORT_OPTIONS_GCC       =
      !endif
      !if $(COMPAL_NOEC_EEPROM_SUPPORT) == YES
        DEFINE COMPAL_NOEC_EEPROM_SUPPORT_OPTIONS         = /D COMPAL_NOEC_EEPROM_SUPPORT
        DEFINE COMPAL_NOEC_EEPROM_SUPPORT_OPTIONS_GCC     = /D COMPAL_NOEC_EEPROM_SUPPORT
      !else
        DEFINE COMPAL_NOEC_EEPROM_SUPPORT_OPTIONS         =
        DEFINE COMPAL_NOEC_EEPROM_SUPPORT_OPTIONS_GCC     =
      !endif
      !if $(OEM_DEBUG_PAGE_SUPPORT) == YES
        DEFINE OEM_DEBUG_PAGE_SUPPORT_OPTIONS         = /D OEM_DEBUG_PAGE_SUPPORT
        DEFINE OEM_DEBUG_PAGE_SUPPORT_OPTIONS_GCC     = /D OEM_DEBUG_PAGE_SUPPORT
      !else
        DEFINE OEM_DEBUG_PAGE_SUPPORT_OPTIONS         =
        DEFINE OEM_DEBUG_PAGE_SUPPORT_OPTIONS_GCC     =
      !endif
        DEFINE EDK_EDKII_COMPAL_FEATURE_BUILD_OPTIONS = $(COMPAL_COMMON_CODE_SUPPORT_OPTIONS) \
                                                        $(COMPAL_EC_SUPPORT_OPTIONS) \
                                                        $(COMPAL_GLOBAL_NVS_SUPPORT_OPTIONS) \
                                                        $(COMPAL_ASL_SUPPORT_OPTIONS) \
                                                        $(COMPAL_EEPROM_SUPPORT_OPTIONS) \
                                                        $(COMPAL_CMFC_SUPPORT_OPTIONS) \
                                                        $(COMPAL_THERMAL_SUPPORT_OPTIONS) \
                                                        $(COMPAL_FLASH_SUPPORT_OPTIONS) \
                                                        $(COMPAL_VAREEPROM_SUPPORT_OPTIONS) \
                                                        $(COMPAL_NOEC_EEPROM_SUPPORT_OPTIONS) \
                                                        $(OEM_DEBUG_PAGE_SUPPORT_OPTIONS)
        DEFINE EDK_EDKII_COMPAL_FEATURE_BUILD_OPTIONS_GCC = $(COMPAL_COMMON_CODE_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_EC_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_GLOBAL_NVS_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_ASL_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_EEPROM_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_CMFC_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_THERMAL_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_FLASH_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_VAREEPROM_SUPPORT_OPTIONS_GCC) \
                                                            $(COMPAL_NOEC_EEPROM_SUPPORT_OPTIONS_GCC) \
                                                            $(OEM_DEBUG_PAGE_SUPPORT_OPTIONS_GCC)

        DEFINE EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS) $(EDK_EDKII_COMPAL_FEATURE_BUILD_OPTIONS)
        DEFINE EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC) $(EDK_EDKII_COMPAL_FEATURE_BUILD_OPTIONS_GCC)
[PcdsFixedAtBuild.common]
  gCompalCommonCodeGuid.PcdEcRamMapAddress|$(ECRAM_OFFSET_ADDRESS)
#[PRJ]++ <<<< Modify for support  VirtualEEPROMVerifyTool and CMFCVerify  
[LibraryClasses]
!if $(COMPAL_COMMON_CODE_SUPPORT) == YES
  !if ("$(EC_PLATFORM_SETTING)" == "LPC" || "$(EC_PLATFORM_SETTING)" == "eSPI")
  CompalEcLib|$(COMPAL_COMMON_PATH)/CompalEC/$(COMPAL_EC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/$(EC_CHIPSET_VENDOR)/$(EC_CHIPSET_NAME)/CompalEcLib.inf
  !else if "$(EC_PLATFORM_SETTING)" == "I2C"
    CompalEcLib|$(COMPAL_COMMON_PATH)/CompalEC/$(COMPAL_EC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/$(EC_CHIPSET_VENDOR)/$(EC_CHIPSET_NAME)/CompalEcLib/CompalEcLib.inf
  !endif 
  !if $(COMPAL_THERMAL_SUPPORT) == YES
    CompalThermalLib|$(COMPAL_COMMON_PATH)/CompalThermal/$(COMPAL_THERMAL_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Library/CompalThermalLib.inf
  !endif
!endif
[LibraryClasses.common.PEIM]
!if $(COMPAL_COMMON_CODE_SUPPORT) == YES
  !if "$(EC_PLATFORM_SETTING)" == "I2C"
    CompalEcLibPei|$(COMPAL_COMMON_PATH)/CompalEC/$(COMPAL_EC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/$(EC_CHIPSET_VENDOR)/$(EC_CHIPSET_NAME)/CompalEcLibPei/CompalEcLibPei.inf
  !endif 
!endif
[LibraryClasses.common.DXE_SMM_DRIVER]
#[PRJ]++ >>>> Modify for support  VirtualEEPROMVerifyTool and CMFCVerify  
  SmmCompalSvcLib|$(COMPAL_COMMON_PATH)/Library/SmmCompalLib/SmmCompalSvcLib.inf
#[PRJ]++ <<<< Modify for support  VirtualEEPROMVerifyTool and CMFCVerify   
!if $(COMPAL_COMMON_CODE_SUPPORT) == YES
  CompalFlashLib|$(COMPAL_COMMON_PATH)/CompalFlash/$(COMPAL_FLASH_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Library/CompalFlash.inf
!endif

[LibraryClasses.common.COMBINED_SMM_DXE]
!if $(COMPAL_COMMON_CODE_SUPPORT) == YES
  CompalFlashLib|$(COMPAL_COMMON_PATH)/CompalFlash/$(COMPAL_FLASH_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Library/CompalFlash.inf
!endif

[LibraryClasses.common.SMM_CORE]
!if $(COMPAL_COMMON_CODE_SUPPORT) == YES
  CompalFlashLib|$(COMPAL_COMMON_PATH)/CompalFlash/$(COMPAL_FLASH_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Library/CompalFlash.inf
!endif

[Components.X64]
!if $(COMPAL_COMMON_CODE_SUPPORT) == YES
  !if $(COMPAL_EEPROM_SUPPORT) == YES
    $(COMPAL_COMMON_PATH)/CompalEEPROM/$(COMPAL_EEPROM_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Dxe/CompalEepromDxe.inf
    $(COMPAL_COMMON_PATH)/CompalEEPROM/$(COMPAL_EEPROM_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Smm/CompalEepromSmm.inf
  !endif
  !if $(COMPAL_PLATFORM_HOOK_SUPPORT) == YES
    $(COMPAL_COMMON_PATH)/CompalPlatformHook/$(COMPAL_PLATFORM_HOOK_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Dxe/CompalPlatformHookDxe/CompalPlatformHookDxe.inf
  !endif
  !if $(COMPAL_THERMAL_SUPPORT) == YES
    $(COMPAL_COMMON_PATH)/CompalThermal/$(COMPAL_THERMAL_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Dxe/CompalThermalDxe.inf
  !endif
  !if $(COMPAL_SSID_SVID_SUPPORT) == YES
    $(COMPAL_COMMON_PATH)/CompalSsidSvid/$(COMPAL_SSID_SVID_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Dxe/CompalSsidSvidDxeProgram/CompalSsidSvidDxeProgram.inf
    $(COMPAL_COMMON_PATH)/CompalSsidSvid/$(COMPAL_SSID_SVID_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Dxe/CompalSsidSvidDxeSetting/CompalSsidSvidDxeSetting.inf
  !endif
  !if $(COMPAL_ASL_SUPPORT) == YES
    $(COMPAL_COMMON_PATH)/CompalGlobalNvs/$(COMPAL_GLOBAL_NVS_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Dxe/CompalGlobalNvsDxe.inf
  !endif
  !if $(COMPAL_PLATFORM_SMM_SUPPORT) == YES
    $(COMPAL_COMMON_PATH)/CompalPlatformSmm/$(COMPAL_PLATFORM_SMM_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Smm/Smi/CompalPlatformSmm/CompalPlatformSmm.inf
  !endif
  !if $(COMPAL_THERMAL_UTILITY_SUPPORT) == YES
  $(COMPAL_COMMON_PATH)/CompalThermalUtility/$(COMPAL_THERMAL_UTILITY_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Smm/Smi/CompalThermalToolSmi/CompalThermalToolSmi.inf
  !endif
  $(COMPAL_COMMON_PATH)/CompalCMFC/$(COMPAL_CMFC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Dxe/CompalCMFCDxe.inf
  $(COMPAL_COMMON_PATH)/CompalCMFC/$(COMPAL_CMFC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Smm/Hook/CompalCMFCSmmHook.inf
  $(COMPAL_COMMON_PATH)/CompalCMFC/$(COMPAL_CMFC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Smm/Smi/CompalCMFCSmmCommon/CompalCMFCCommonSwSmi.inf
  $(COMPAL_COMMON_PATH)/CompalCMFC/$(COMPAL_CMFC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Smm/Smi/CompalCMFCSmmOEM/CompalCMFCOEMSwSmi.inf  
  !if $(COMPAL_BRLV_SUPPORT) == YES
    $(COMPAL_COMMON_PATH)/CompalBRLV/$(COMPAL_BRLV_VERSION)/$(COMPAL_CHIPSET_VENDOR)/Dxe/CompalBRLVDxe.inf
  !endif
  !if ("$(EC_PLATFORM_SETTING)" == "LPC" || "$(EC_PLATFORM_SETTING)" == "eSPI")
    !if $(COMPAL_EC_AUTO_FLASH_SUPPORT) == YES
      $(COMPAL_COMMON_PATH)/CompalEcAutoFlash/$(COMPAL_EC_AUTO_FLASH_VERSION)/CompalEcAutoFlash.inf
    !endif
    !if $(COMPAL_EC_FLASH_TOOL_SUPPORT) == YES
      $(COMPAL_COMMON_PATH)/CompalEcAutoFlash/$(COMPAL_EC_AUTO_FLASH_VERSION)/CompalEcFlash.inf
    !endif
  !endif
!endif

[Components.$(DXE_ARCH)]
!if $(COMPAL_COMMON_CODE_SUPPORT) == YES
  !if "$(EC_PLATFORM_SETTING)" == "I2C"
   $(COMPAL_COMMON_PATH)/CompalEC/$(COMPAL_EC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/$(EC_CHIPSET_VENDOR)/$(EC_CHIPSET_NAME)/CompalECFlashI2C/CompalECFlashI2C.inf
   $(COMPAL_COMMON_PATH)/CompalEC/$(COMPAL_EC_VERSION)/$(COMPAL_CHIPSET_VENDOR)/$(EC_CHIPSET_VENDOR)/$(EC_CHIPSET_NAME)/CompalECFwAutoLoadDxe/CompalECFwAutoLoadDxe.inf 
  !endif
!endif
[Defines]
  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = SKYLAKE  
