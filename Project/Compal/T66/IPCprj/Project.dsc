## @file
#  Platform Package Description file
#
#******************************************************************************
#* Copyright (c) 2013 - 2016, Insyde Software Corp. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

!import MdePkg/Package.dsc
!import MdeModulePkg/Package.dsc
!import UefiCpuPkg/Package.dsc
#!import PerformancePkg/Package.dsc
!import CryptoPkg/Package.dsc
!import FatBinPkg/Package.dsc
!import ShellPkg/Package.dsc
!import SecurityPkg/Package.dsc
!import PcAtChipsetPkg/Package.dsc
!import IA32FamilyCpuPkg/Package.dsc
!import IntelFrameworkModulePkg/Package.dsc
!import EdkCompatibilityPkg/Package.dsc
!import InsydeOemServicesPkg/Package.dsc
!import InsydeModulePkg/Package.dsc
#[-start-151124-IB07220017-add]#
!import InsydeSetupPkg/Package.dsc
#[-end-151124-IB07220017-add]#
!import InsydeNetworkPkg/Package.dsc
!import InsydeFlashDevicePkg/Package.dsc
#[-start-160411-IB07400715-add]#
!import InsydeCrPkg/Package.dsc
#[-end-160411-IB07400715-add]#
#[-start-170331-IB07400855-add]#
!import InsydeCbtPkg/Package.dsc
#[-end-170331-IB07400855-add]#
#[-start-170111-IB07400832-remove]#
#!import BroxtonChipsetPkg/Package.dsc
#[-end-170111-IB07400832-remove]#
!import BroxtonSiPkg/Package.dsc
#[-start-151126-IB02950555-modify]#
!import ClientSiliconPkg/Package.dsc
#[-end-151126-IB02950555-modify]#
#[-start-170110-IB07400831-add]#
!import InsydeH2oUvePkg/Package.dsc
#[-end-170110-IB07400831-add]#
#[-start-180822-IB07400999-add]#
!import InsydeEventLogPkg/Package.dsc
!import SegFeaturePkg/Package.dsc
#[-end-180822-IB07400999-add]#
#[-start-160812-IB07400770-add]#
!import SioWpcd374lPkg/Package.dsc
#[-end-160812-IB07400770-add]#
#[-start-170427-IB07400864-add]#
!import SioNpce285Pkg/Package.dsc
#[-end-170427-IB07400864-add]#

#[-start-170111-IB07400832-add]#
#
# High Priority for Chipset Package
#
!import BroxtonChipsetPkg/Package.dsc
#[-end-170111-IB07400832-add]#
!import ../../COMMON/Package.dsc
!import ../COMMON/T66Common.dsc
################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                       = ApolloLake
  PLATFORM_GUID                       = 465B0A0B-7AC1-443b-8F67-7B8DEC145F90
  PLATFORM_VERSION                    = 0.1
  DSC_SPECIFICATION                   = 0x00010005
  OUTPUT_DIRECTORY                    = Build/$(PROJECT_PKG)
  SUPPORTED_ARCHITECTURES             = IA32|X64
  BUILD_TARGETS                       = DEBUG|RELEASE
  SKUID_IDENTIFIER                    = DEFAULT
  FLASH_DEFINITION                    = Build/$(PROJECT_PKG)/Project.fdf

#[-start-151122-IB07220012-modify]#
#[-start-160411-IB07400715-modify]#
  !include Build/$(PROJECT_PKG)/Project.env
  !include $(PROJECT_PKG)/Project.env
#[-end-160411-IB07400715-modify]#
#[-end-151122-IB07220012-modify]#
#[-start-160217-IB07400702-remove]#
#  !include $(PROJECT_PKG)/BinConfig.env
#[-end-160217-IB07400702-remove]#
  
  DEFINE PLATFORM_RC_PACKAGE                = BroxtonSiPkg
!if $(X64_CONFIG) == YES
  DEFINE UEFI_ARCH = X64
!else 
  DEFINE UEFI_ARCH = IA32
!endif
  DEFINE SECURE_FLASH_CERTIFICATE_FILE_PATH = $(PROJECT_PKG)/FactoryCopyInfo/SecureFlash.cer
  DEFINE PLATFORM_SOC                       = BroxtonSoc

  DEFINE PROJECT_OVERRIDE =$(PROJECT_PKG)/Override
################################################################################
#
# SKU Identification section - list of all SKU IDs supported by this
#                              Platform.
#
################################################################################
[SkuIds]
  0|DEFAULT              # The entry: 0|DEFAULT is reserved and always required.
#[-start-170512-IB07400866-add]#
  0x01|AplRvp1
  #0x02|AplRvp2          # GPIO same as RVP1
  0x06|AplIOxbowHill
  #0x07|AplILeafHill     # GPIO same as Oxbow Hill
  #0x08|AplIJuniperHill  # GPIO same as Oxbow Hill
#[-end-170512-IB07400866-add]#

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]

  #
  # Insyde Features
  #
  
  gInsydeTokenSpaceGuid.PcdH2ODdtSupported|$(INSYDE_DEBUGGER)
!if $(EFI_DEBUG) == YES
#[-start-161023-IB07400803-modify]#
!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported AND $(H2O_DDT_DEBUG_IO) == Xhc
  gInsydeTokenSpaceGuid.PcdStatusCodeUseDdt|FALSE
!else
  gInsydeTokenSpaceGuid.PcdStatusCodeUseDdt|$(INSYDE_DEBUGGER)
!endif  
#[-end-161023-IB07400803-modify]#
  gInsydeTokenSpaceGuid.PcdStatusCodeUseUsb|$(USB_DEBUG_SUPPORT)
!if $(USB_DEBUG_SUPPORT) == NO
#[-start-160330-IB07400715-modify]#
!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported AND $(H2O_DDT_DEBUG_IO) == Com
  # Do not output debug message to serial port when using COM port DDT
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|FALSE
!else
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|TRUE
!endif  
#[-end-160330-IB07400715-modify]#
!else
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|FALSE
!endif
#[-start-161130-IB06740525-add]#
#[-start-161201-IB07400821-remove]#
#!if $(INSYDE_DEBUGGER) == YES and $(H2O_DDT_DEBUG_IO) == Com
#  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|FALSE
#!endif
#[-end-161201-IB07400821-remove]#
#[-end-161130-IB06740525-add]#
!endif
  gInsydeTokenSpaceGuid.PcdFrontPageSupported|$(FRONTPAGE_SUPPORT)
  gInsydeTokenSpaceGuid.PcdCrisisRecoverySupported|$(CRISIS_RECOVERY_SUPPORT)
  gInsydeTokenSpaceGuid.PcdUseFastCrisisRecovery|$(USE_FAST_CRISIS_RECOVERY)
#[-start-160823-IB07400773-modify]#
!if $(BUILD_SECURE_FLASH) == YES
  gInsydeTokenSpaceGuid.PcdSecureFlashSupported|TRUE
!else
  gInsydeTokenSpaceGuid.PcdSecureFlashSupported|$(SECURE_FLASH_SUPPORT)
!endif  
#[-end-160823-IB07400773-modify]#
  gInsydeTokenSpaceGuid.PcdBackupSecureBootSettingsSupported|$(BACKUP_SECURE_BOOT_SETTINGS_SUPPORT)
  gInsydeTokenSpaceGuid.PcdUnsignedFvSupported|$(UNSIGNED_FV_SUPPORT)
#[-start-160217-IB07400702-modify]#
  gInsydeTokenSpaceGuid.PcdH2ONetworkSupported|$(H2O_NETWORK_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2ONetworkIpv6Supported|$(H2O_NETWORK_IPV6_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2ONetworkIscsiSupported|$(H2O_NETWORK_ISCSI_ENABLE)
  gInsydeTokenSpaceGuid.PcdH2OTpmSupported|$(H2O_TPM_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2OTpm2Supported|$(H2O_TPM2_SUPPORT)
#[-end-160217-IB07400702-modify]#
  gInsydeTokenSpaceGuid.PcdSysPasswordInCmos|$(SYS_PASSWORD_IN_CMOS)
  gInsydeTokenSpaceGuid.PcdSysPasswordSupportUserPswd|$(SUPPORT_USER_PASSWORD)
  gInsydeTokenSpaceGuid.PcdReturnDialogCycle|$(RETURN_DIALOG_CYCLE)
  gInsydeTokenSpaceGuid.PcdH2OUsbSupported|TRUE
#[-start-160901-IB07400777-modify]#
  gInsydeTokenSpaceGuid.PcdH2OAhciSupported|$(H2O_AHCI_SUPPORT)
#[-end-160901-IB07400777-modify]#
  gInsydeTokenSpaceGuid.PcdH2OQ2LServiceSupported|$(Q2LSERVICE_SUPPORT)
#[-start-160901-IB07400777-modify]#
  gInsydeTokenSpaceGuid.PcdH2OIdeSupported|$(H2O_IDE_SUPPORT)
#[-end-160901-IB07400777-modify]#
#[-start-160217-IB07400702-modify]#
#[-start-161007-IB11270165-modify]#
  gInsydeTokenSpaceGuid.PcdH2OSdhcSupported|$(H2O_SDHC_SUPPORT)
#[-end-161007-IB11270165-modify]#
  gInsydeTokenSpaceGuid.PcdSnapScreenSupported|$(SNAPSCREEN_SUPPORT)
#[-end-160217-IB07400702-modify]#
  gInsydeTokenSpaceGuid.PcdUseEcpVariableStoreHeader|FALSE
#[-start-160429-IB07220075-modify]#
  gInsydeTokenSpaceGuid.PcdMemoryMapConsistencyCheck|TRUE
#[-end-160429-IB07220075-modify]#
  gInsydeTokenSpaceGuid.PcdGraphicsSetupSupported|$(GRAPHICS_SETUP_SUPPORT)  
  gInsydeTokenSpaceGuid.PcdUefiPauseKeyFunctionSupport|$(UEFI_PAUSE_KEY_FUNCTION_SUPPORT)
  gInsydeTokenSpaceGuid.PcdTextModeFullScreenSupport|$(TEXT_MODE_FULL_SCREEN_SUPPORT)
  gInsydeTokenSpaceGuid.PcdOnlyUsePrimaryMonitorToDisplay|$(ONLY_USE_PRIMARY_MONITOR_TO_DISPLAY)    
  gInsydeTokenSpaceGuid.PcdMultiConfigSupported|$(MULTI_CONFIG_SUPPORT)
  gInsydeTokenSpaceGuid.PcdDynamicHotKeySupported|$(DYNAMIC_HOTKEY_SUPPORT)
#[-start-160217-IB07400702-modify]#
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported|$(H2O_FORM_BROWSER_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalTextDESupported|$(H2O_FORM_BROWSER_TEXT_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalMetroDESupported|$(H2O_FORM_BROWSER_METRO_SUPPORT)
#[-end-160217-IB07400702-modify]#
#[-start-151124-IB07220017-add]#
!errif (gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported == TRUE) and (gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalTextDESupported == FALSE) and (gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalMetroDESupported == FALSE), "Must have at least one display engine enabled in Project.dsc when PcdH2OFormBrowserSupported == TRUE; Otherwise set PcdH2OFormBrowserSupported = FALSE in Project.dsc"
#[-end-151124-IB07220017-add]#
!if gInsydeTokenSpaceGuid.PcdH2OFormBrowserSupported == TRUE
  gInsydeTokenSpaceGuid.PcdGraphicsSetupSupported|FALSE
!else
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalTextDESupported|FALSE
  gInsydeTokenSpaceGuid.PcdH2OFormBrowserLocalMetroDESupported|FALSE
!endif
  gInsydeTokenSpaceGuid.PcdBvdtGenBiosBuildTimeSupported|$(BUILD_TIME_FLAG)
#[-start-160411-IB07400715-modify]#
  gInsydeTokenSpaceGuid.PcdEdkShellBinSupported|$(EFI_SHELL_SUPPORT)
  gInsydeTokenSpaceGuid.PcdShellBinSupported|$(UEFI_SHELL_SUPPORT)
#[-end-160411-IB07400715-modify]#
#[-start-160618-IB07400744-modify]#
  gInsydeTokenSpaceGuid.PcdShellBuildSupported|$(UEFI_SHELL_BUILD_SUPPORT)
#[-end-160618-IB07400744-modify]#
#[-start-170222-IB07400843-add]#
!if $(EFI_DEBUG) == YES
  #
  # Disable EFI Shell Support for debug BIOS to reduce BIOS image size
  #
  gInsydeTokenSpaceGuid.PcdEdkShellBinSupported|FALSE
  gInsydeTokenSpaceGuid.PcdShellBinSupported|FALSE
  gInsydeTokenSpaceGuid.PcdShellBuildSupported|FALSE
!endif
#[-end-170222-IB07400843-add]#
#[-start-160302-IB11270148-add]#
#
# SUPPORT_HDD_PASSWORD
#
#[-start-160329-IB07400711-modify]#
  gInsydeTokenSpaceGuid.PcdH2OHddPasswordSupported|FALSE
#[-end-160329-IB07400711-modify]#
#[-end-160302-IB11270148-add]#  
#[-start-160808-IB07220123-add]#
  gInsydeTokenSpaceGuid.PcdEndOfDxeEventSupported|FALSE
#[-end-160808-IB07220123-add]#
  #
  # Chipset configuration related
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSwitchToLongMode|$(X64_CONFIG)
  gEfiMdeModulePkgTokenSpaceGuid.PcdBrowserGrayOutTextStatement|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|FALSE #$(CAPSULE_RESET_ENABLE)
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdStatusCodeUseDataHub|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreImageLoaderSearchTeSectionFirst|FALSE
  gEfiSerialPortTokenSpaceGuid.PcdStatusCodeUseIsaSerial|TRUE
  gEfiSerialPortTokenSpaceGuid.PcdStatusCodeUseUsbSerial|FALSE
#[-start-160531-IB08450347-add]#
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdPs2KbdExtendedVerification|FALSE
#[-end-160531-IB08450347-add]#

  #
  # Chipset features
  #
  gChipsetPkgTokenSpaceGuid.PcdUseEmuVariable|$(USE_EMU_VARIABLE)
  gChipsetPkgTokenSpaceGuid.GpioEfiDebug|FALSE
#[-start-180504-IB07400964-remove]#
#  gChipsetPkgTokenSpaceGuid.SecureBootStage1Check|$(SECURE_BOOT_STAGE1_CHECK)
#  gChipsetPkgTokenSpaceGuid.SecureBootStage2Check|$(SECURE_BOOT_STAGE2_CHECK)
#[-end-180504-IB07400964-remove]#
  gChipsetPkgTokenSpaceGuid.PcdIsctSupported|TRUE
  gChipsetPkgTokenSpaceGuid.FtpmSupport|$(FTPM_ENABLE)
  gChipsetPkgTokenSpaceGuid.PcdLegacyBootEnabled|$(LEGACY_BOOT_ENABLE)
  gChipsetPkgTokenSpaceGuid.PcdIntelAndroidWR|FALSE
#[-start-160415-IB08450340-modify]#
  gChipsetPkgTokenSpaceGuid.PcdEnableSmiEnableAcpi|TRUE
#[-end-160415-IB08450340-modify]#
  gChipsetPkgTokenSpaceGuid.PcdSmmInt10Enable|$(SMM_INT10_ENABLE)
  gChipsetPkgTokenSpaceGuid.PcdSusPwrFlrReset|FALSE
#[-start-161123-IB07250310-add]#
  gChipsetPkgTokenSpaceGuid.PcdFspEnabled|$(FSP_ENABLE)
#[-end-161123-IB07250310-add]#
  gChipsetPkgTokenSpaceGuid.PcdSwitchableGraphicsSupported|$(HYBRID_GRAPHICS_SUPPORT)
  gChipsetPkgTokenSpaceGuid.PcdNvidiaOptimusSupported|$(NVIDIA_OPTIMUS_SUPPORT)
  gChipsetPkgTokenSpaceGuid.PcdAmdPowerXpressSupported|$(AMD_POWERXPRESS_SUPPORT)
  gChipsetPkgTokenSpaceGuid.PcdEnableDdr32xRefreshRate|FALSE  
  gChipsetPkgTokenSpaceGuid.PcdSpi2mSupport|FALSE
  gChipsetPkgTokenSpaceGuid.PcdEmmcDriverBootPartitionSupport|FALSE

#[-start-160121-IB06740461-add]#
  gInsydeTokenSpaceGuid.PcdH2OI2cSupported|TRUE
#[-end-160121-IB06740461-add]#

#[-start-170517-IB07400866-remove]#
#!if $(USE_PCD_GPIO_TABLE) == YES
#  gChipsetPkgTokenSpaceGuid.PcdUsePcdGpioTable|$(USE_PCD_GPIO_TABLE)
#  gChipsetPkgTokenSpaceGuid.PcdSelectBswGpioTable|$(ENBDT_SELECT_CRB_GPIO)
#  gChipsetPkgTokenSpaceGuid.PcdSelectBswCHGpioTable|$(ENBDT_SELECT_CH_GPIO)
#  gChipsetPkgTokenSpaceGuid.PcdSelectBswCIGpioTable|$(ENBDT_SELECT_CI_GPIO)
#!endif
#[-end-170517-IB07400866-remove]#
#[-start-160421-IB08450341-modify]#
  gChipsetPkgTokenSpaceGuid.PcdRuntimeFtpmDtpmSwitch|TRUE
#[-end-160421-IB08450341-modify]#
 
#[-start-151123-IB02950555-add]#
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwarePerformanceDataTableS3Support|FALSE
#[-end-151123-IB02950555-add]#

#[-start-160711-IB03090428-add]#
  #
  # Host Managed Variable Enable
  #   TRUE  - Variables are managed on SPI directly by IAFW
  #   FALSE - Variables are managed on CSE NVM by the CSE file system driver
  #
  gBxtRefCodePkgTokenSpaceGuid.PcdHostManagedVariables|FALSE
#[-end-160711-IB03090428-add]#
#
#  Set PcdBuildActivatesSecureBoot to restore factory default keys during the first boot.
#
#[-start-171117-IB07400930-modify]#
  gInsydeTokenSpaceGuid.PcdBuildActivatesSecureBoot|$(DEFAULT_SECURE_BOOT_ENABLED)
#[-end-171117-IB07400930-modify]#

#[-start-160217-IB07400702-add]#
#[-start-160614-IB07400744-modify]#
#  gInsydeTokenSpaceGuid.PcdH2OConsoleRedirectionSupported|$(H2O_CONSOLE_REDIRECTION_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2OI2cSupported|$(H2O_I2C_SUPPORT)
  gChipsetPkgTokenSpaceGuid.PcdCrisisRecoverySdEmmcSupport|$(CRISIS_RECOVERY_SD_EMMC_SUPPORT)  
  gChipsetPkgTokenSpaceGuid.PcdCrisisRecoverySataSupport|$(CRISIS_RECOVERY_SATA_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2OUsbPeiSupported|$(CRISIS_RECOVERY_USB_SUPPORT)
  gChipsetPkgTokenSpaceGuid.PcdCrisisRecoveryNvmeSupport|$(CRISIS_RECOVERY_NVME_SUPPORT)
  gChipsetPkgTokenSpaceGuid.PcdCrisisRecoveryCdDvdSupport|$(CRISIS_RECOVERY_CD_DVD_SUPPORT)
#[-end-160614-IB07400744-modify]#
#[-end-160217-IB07400702-add]#
#[-start-160308-IB07400709-add]#
  # UART Post Code
  gChipsetPkgTokenSpaceGuid.PcdUartPostCodeSupport|$(UART_POST_CODE_SUPPORT)
#[-start-160819-IB07400772-add]#
  gChipsetPkgTokenSpaceGuid.PcdUartDebugSupport|$(UART_DEBUG_SUPPORT)
#[-end-160819-IB07400772-add]#
#[-start-160324-IB07400711-add]#
  gChipsetPkgTokenSpaceGuid.PcdHsUartDebugSupport|$(HS_UART_DEBUG_SUPPORT)
#[-end-160324-IB07400711-add]#
  
!if $(EFI_DEBUG) == YES || $(USB_DEBUG_SUPPORT) == YES
  gChipsetPkgTokenSpaceGuid.PcdUartPostCodeSupport|FALSE
!endif  
!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported AND $(H2O_DDT_DEBUG_IO) == Com
  gChipsetPkgTokenSpaceGuid.PcdUartPostCodeSupport|FALSE
!endif
#[-end-160308-IB07400709-add]#
#[-start-160628-IB07400749-add]#
#[-start-161205-IB07400822-modify]#
  gChipsetPkgTokenSpaceGuid.PcdKscSupport|$(KSC_SUPPORT)
  gChipsetPkgTokenSpaceGuid.PcdCmosRestoreSetupDefault|$(CMOS_RESTORE_SETUP_DEFAULT)
#[-end-161205-IB07400822-modify]#
#[-end-160628-IB07400749-add]#
#[-start-161017-IB07400798-add]#
#===============================================================================#
#                                                                               #
# SPI ROM Pcds enable/disable, override InsydeFlashDevicePkg.dec Pcds           #
# Note:If vendor pcd is disabled, all SPI ROM of this vendor will be disabled   #
#                                                                               #
#===============================================================================#
  
  # SST
  gH2OFlashDeviceGuid.PcdSstEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSst25lf040SpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSst25lf040aSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSst25lf080aSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSst25vf080bSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSst25vf016bSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSst39vf080LpcEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSst49lf008aLpcEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSst49lf004aLpcEnable|FALSE
  
  # Atem
  gH2OFlashDeviceGuid.PcdAtmelEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdAtmel25df041SpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdAtmel25df081aSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdAtmel26df161SpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdAtmel26df321SpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdAtmel25df641SpiEnable|FALSE

  # Winbond
  gH2OFlashDeviceGuid.PcdWinbondEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdWinbondW25q64bvSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdWinbondW25q64fwSpiEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdWinbondW25q16dwSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdWinbond39v040fpaLpcEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdWinbondW25q128fwSpiEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdWinbondW25q128bvSpiEnable|FALSE

#[-start-161118-IB07400816-modify]#
  # Mxic
  gH2OFlashDeviceGuid.PcdMxicEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdMxic25l8005SpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdMxic25l1605aSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdMxic25l6405dSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdMxic25l6436eSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdMxic25l12805dSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdMxic25l25635eSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdMxic25u3235fSpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdMxic25u6435fSpiEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdMxic25u12835fSpiEnable|TRUE
#[-end-161118-IB07400816-modify]#
  
  # Intel
  gH2OFlashDeviceGuid.PcdIntelEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdIntel82802acLpcEnable|FALSE

  # ST
  gH2OFlashDeviceGuid.PcdStEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdStM25pe80SpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdSt50flw040aLpcEnable|FALSE

  # PMC
  gH2OFlashDeviceGuid.PcdPmcEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdPmc25lv040SpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdPmc49fl004tLpcEnable|FALSE

  # GD
  gH2OFlashDeviceGuid.PcdGdEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdGd25lq16SpiEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdGd25lq64cSpiEnable|TRUE
  
#[-start-161118-IB07400816-modify]#
  # Numonyx
  gH2OFlashDeviceGuid.PcdNumonyxEnable|FALSE
  gH2OFlashDeviceEnableGuid.PcdNumonyxn25q128a13SpiEnable|FALSE
#  gH2OFlashDeviceEnableGuid.PcdNumonyxn25q128a11SpiEnable|TRUE # Change to PcdMicronN25q128aSpiEnable
#[-end-161118-IB07400816-modify]#

  # eFeon/EON
  gH2OFlashDeviceGuid.PcdcFeonEonEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdcFeonEonEn25s64SpiEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdcFeonEonEn25s32SpiEnable|FALSE
  
#[-start-161118-IB07400816-add]#
  # Micron
  gH2OFlashDeviceGuid.PcdMicronEnable|TRUE
  gH2OFlashDeviceEnableGuid.PcdMicronN25q128aSpiEnable|TRUE
#[-end-161118-IB07400816-add]#

  # Common
#[-start-190722-IB16530010-modify]#
  gH2OFlashDeviceEnableGuid.PcdCommonvidCommondidSpiEnable|FALSE
#[-end-190722-IB16530010-modify]#
#[-end-161017-IB07400798-add]#
#[-start-170124-IB07400837-add]#
  gInsydeCrTokenSpaceGuid.PcdH2OConsoleRedirectionSupported|$(H2O_CONSOLE_REDIRECTION_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2ORotateScreenSupported|$(H2O_ROTATE_SCREEN_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2ORotateShellSupported|$(H2O_ROTATE_SCREEN_SHELL_SUPPORT)
#[-end-170124-IB07400837-add]#
#[-start-170331-IB07400855-add]#
!if $(BUILD_H2O_CBT) == YES
  gInsydeCbtTokenSpaceGuid.PcdH2OCBTSupported|TRUE
!endif  
#[-end-170331-IB07400855-add]#
#[-start-170511-IB07400866-add]#
!if $(H2O_ESRT_CONFIG_TABLE_SUPPORT) == NO
  gInsydeTokenSpaceGuid.PcdH2OCapsuleUpdateSupported|FALSE
  gInsydeTokenSpaceGuid.PcdUefiCapsuleUpdateSupported|TRUE
!endif
#[-end-170511-IB07400866-add]#
#[-start-170605-IB07400873-add]#
  gInsydeTokenSpaceGuid.PcdAutoCreateDummyBootOption|FALSE
#[-end-170605-IB07400873-add]#
#[-start-170830-IB07400903-add]#
  gInsydeTokenSpaceGuid.PcdH2ONvmeSupported|$(H2O_NVME_SUPPORT)
#[-end-170830-IB07400903-add]#
#[-start-180823-IB07400999-add]#
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogSupported|$(H2O_EVENT_LOG_SUPPORTED)
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageSupported|$(H2O_BIOS_EVENT_STORAGE_SUPPORTED)
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBiosEventStorageUseVariableStorageSupported|$(H2O_BIOS_VARIABLE_STORAGE_SUPPORTED)
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OBmcSelEventStorageSupported|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OEventLogConfigManagerSupported|$(H2O_EVENT_LOG_CONFIG_SUPPORTED)
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OPostMessageSupported|$(H2O_POST_MESSAGE_SUPPORTED)
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OPostBeepSupported|$(H2O_POST_BEEP_SUPPORTED)
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OEfiDebugMsgSupported|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OStoreDebugMsgSupported|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OStoreDebugMsgPeiSupported|FALSE
  gH2OEventLogPkgTokenSpaceGuid.PcdH2OShowEventsOnPostDisplaySupported|FALSE
#[-end-180823-IB07400999-add]#
#[-start-170119-IB07401072-add]#
  gInsydeTokenSpaceGuid.Pcd64BitAmlSupported|$(PCI_64BITS_MMIO_DECODE_SUPPORT)
#[-end-170119-IB07401072-add]#
[PcdsFixedAtBuild.common]
#  #
#  # Crisis File name definition
#  #
#  # New File Path Definition : //Volume_Label\\File_Path\\File_Name
#  # Notice : "//" is signature that volume label start definition.
#  #
#  # Example path : //RECOVERY\\BIOS\\Current\\Chv_Ia32_RELEASE.ROM
#  gInsydeTokenSpaceGuid.PcdPeiRecoveryFile|L"Chv_Ia32_RELEASE.ROM"
  
!if $(FIRMWARE_PERFORMANCE) == YES
  gEfiCpuTokenSpaceGuid.PcdTemporaryRamBase|0xFEFC0000
  # 
  # Please make the following Pcd values same. 
  #    1. gInsydeTokenSpaceGuid.PcdTemporaryRamSizeWhenRecovery
  #    2. gEfiCpuTokenSpaceGuid.PcdTemporaryRamSize
  # 
  # 
  gEfiCpuTokenSpaceGuid.PcdTemporaryRamSize|0x00040000
  gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|0x1
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxPeiPerformanceLogEntries|45
!else
#[-start-151228-IB03090424-modify]#
  #
  # According SecEntry.asm, before calling SecStartup, the TempRamBase
  # is calculated by below method:
  #   TempRamBase = CarBase + IBBSize + IBBLSize + 800h(For FIT)
  #               = 0xFEF00000 + 0x59000 + 0x1000 + 0x800
  #               = 0xFEF5A800
  #
  gEfiCpuTokenSpaceGuid.PcdTemporaryRamBase|0xFEF5A800
  #
  # PcdTemporaryRamBase + PcdTemporaryRamSize should equal top of CAR
  # Top of CAR (CarBase + CarSize) = 0xFF000000 on BXT
  #
  gEfiCpuTokenSpaceGuid.PcdTemporaryRamSize|0x000A5800
  gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|0
#[-start-161123-IB07250310-modify]#
!if gChipsetPkgTokenSpaceGuid.PcdFspEnabled
  gEfiCpuTokenSpaceGuid.PcdPeiTemporaryRamStackSize|0x4000
!else
  gEfiCpuTokenSpaceGuid.PcdPeiTemporaryRamStackSize|0x12000
!endif
#[-end-161123-IB07250310-modify]#
#[-end-151228-IB03090424-modify]#
!endif
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x10000
  gEfiMdeModulePkgTokenSpaceGuid.PcdHwErrStorageSize|0x00000800
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxHardwareErrorVariableSize|0x400
  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreMaxPeimPerFv|50
  gEfiMdeModulePkgTokenSpaceGuid.PcdSrIovSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdAriSupport|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuSmmApSyncTimeout|1000
  gEfiMdeModulePkgTokenSpaceGuid.PcdPeiCoreMaxFvSupported|16
#  gPlatformModuleTokenSpaceGuid.PcdFlashMicroCodeSize|0x00042000
!if $(S4_ENABLE) == YES
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|TRUE
!else
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE
!endif
!if $(EFI_DEBUG) == NO && $(USB_DEBUG_SUPPORT) == NO
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x01
!else
#[-start-160329-IB07400711-modify]#
!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported
  # Enable DebugBreak if H2ODDT is enabled
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x3F
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2F
!endif
#[-end-160329-IB07400711-modify]#
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07
!endif
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdEbdaReservedMemorySize|0x10000
  gEfiMdeModulePkgTokenSpaceGuid.PcdLoadModuleAtFixAddressEnable|$(TOP_MEMORY_ADDRESS)
  gEfiMdeModulePkgTokenSpaceGuid.PcdBrowserSubtitleTextColor|0x0
  gEfiMdeModulePkgTokenSpaceGuid.PcdBrowserFieldTextColor|0x01
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdS3AcpiReservedMemorySize|0x1900000
  gEfiCpuTokenSpaceGuid.PcdCpuIEDEnabled|FALSE
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdBiosVideoCheckVbeEnable|TRUE
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdBiosVideoCheckVgaEnable|TRUE
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdLogoFile |{0xD1, 0x77,0x1F, 0x93, 0xFE,  0x10, 0xbf, 0x48, 0xAB, 0x72, 0x77, 0x3D, 0x38, 0x9E, 0x3F, 0xAA}
  #
  # Hot key Configuration
  # Platform Hot key Define
  # ScanCode, ShiftKey, AltKey, CtrlKey
  # ex:
  #    0x54, 0x0, 0x1, 0x0      F1(Combination Key ScanCode) + ShiftKey
  #    0x68, 0x0, 0x2, 0x0      F1(Combination Key ScanCode) + AltKey
  #    0x5f, 0x0, 0x4, 0x0      F1(Combination Key ScanCode) + CtrlKey
  #
  gInsydeTokenSpaceGuid.PcdPlatformKeyList|{ \
    0x3b, 0x0, 0x0, 0x0,                     \ # F1_KEY
    0x3c, 0x0, 0x0, 0x0,                     \ # F2_KEY
    0x53, 0x0, 0x0, 0x0,                     \ # DEL_KEY
    0x44, 0x0, 0x0, 0x0,                     \ # F10_KEY
    0x86, 0x0, 0x0, 0x0,                     \ # F12_KEY
    0x01, 0x0, 0x0, 0x0,                     \ # ESC_KEY
    0x40, 0x0, 0x0, 0x0,                     \ # UP_ARROW_KEY_BIT
    0x3d, 0x0, 0x0, 0x0,                     \ # F3_KEY
    0x43, 0x0, 0x0, 0x0,                     \ # F9_KEY
    0x00, 0x0, 0x0, 0x0}                       # EndEntry
  # @PcdTypeStruct OEM_LOGO_RESOLUTION_DEFINITION {UINT32 LogoResolutionX; UINT32 LogoResolutionY; UINT32 ScuResolutionX; UINT32 ScuResolutionY;}
  # @PcdValueType OEM_LOGO_RESOLUTION_DEFINITION
  #  LogoResolution : 1920 x 1080  , ScuResolution : 1024 x 768
  gInsydeTokenSpaceGuid.PcdDefaultLogoResolution|{0x80, 0x07, 0x00, 0x00, 0x38, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00}

  # 
  # Please make the following Pcd values same. 
  #    1. gInsydeTokenSpaceGuid.PcdTemporaryRamSizeWhenRecovery
  #    2. gEfiCpuTokenSpaceGuid.PcdTemporaryRamSize
  # 
  # 
  gInsydeTokenSpaceGuid.PcdTemporaryRamSizeWhenRecovery|0x20000
  
  #
  # Note: If change sizeof(SYSTEM_CONFIGURATION) in SetupConfig.h, must update really structure size in here!!!
  #
#[-start-160217-IB07400702-modify]#
  gChipsetPkgTokenSpaceGuid.PcdSetupConfigSize|$(FULL_SETUP_CONFIG_SIZE)
#[-end-160217-IB07400702-modify]#
#
# fix RC's incorrect Bmp PCD data 
#
# correct { 0x878AC2CC, 0x5343, 0x46F2, {0xB5, 0x63, 0x51, 0xF8, 0x9D, 0xAF, 0x56, 0xBA}}
#  gEfiBxtTokenSpaceGuid.PcdBmpImageGuid|{0xCC, 0xC2, 0x8A, 0x87, 0x43, 0x53, 0xF2, 0x46, 0xB5, 0x63, 0x51, 0xF8, 0x9D, 0xAF, 0x56, 0xBA}

  gInsydeTokenSpaceGuid.PcdScuFormsetGuidList|{ \
    GUID("C1E0B01A-607E-4B75-B8BB-0631ECFAACF2"), \ # Main
    GUID("C6D4769E-7F48-4D2A-98E9-87ADCCF35CCC"), \ # Avance
    GUID("5204F764-DF25-48A2-B337-9EC122B85E0D"), \ # Security
    GUID("A6712873-925F-46C6-90B4-A40F86A0917B"), \ # Power
    GUID("2D068309-12AC-45AB-9600-9187513CCDD8"), \ # Boot
    GUID("B6936426-FB04-4A7B-AA51-FD49397CDC01"), \ # Exit
    GUID("00000000-0000-0000-0000-000000000000")}
#[-start-151123-IB11270134-add]#
  gInsydeTokenSpaceGuid.PcdScuFormsetFlagList|{ \
     UINT8(0), \ # Main
     UINT8(0), \ # Avance
     UINT8(0), \ # Security
     UINT8(0), \ # Power
     UINT8(0), \ # Boot
     UINT8(0), \ # Exit
     UINT8(0xFF)}
#[-end-151123-IB11270134-add]#
#  gEfiBxtTokenSpaceGuid.PcdPmcGcrBaseAddress|0xFE043000

  gInsydeTokenSpaceGuid.PcdReservedMemorySizeForSecureFlash|0x01000000

#[-start-160324-IB07400711-add]#
  gBxtRefCodePkgTokenSpaceGuid.PcdSerialIoUartNumber|$(HS_UART_DEBUG_PORT_NUMBER)
#[-end-160324-IB07400711-add]#
#[-start-160406-IB07400715-add]#
  # PcdBuildPlatformType using BoardId in PlatformInfo.h
  !if $(BUILD_PLATFORM_TYPE) == ApolloLake_RVP1
    gChipsetPkgTokenSpaceGuid.PcdBuildPlatformType|0x01
  !endif

  !if $(BUILD_PLATFORM_TYPE) == ApolloLake_RVP2
    gChipsetPkgTokenSpaceGuid.PcdBuildPlatformType|0x02
  !endif

  !if $(BUILD_PLATFORM_TYPE) == ApolloLakeI_OxbowHill
    gChipsetPkgTokenSpaceGuid.PcdBuildPlatformType|0x06
  !endif

  !if $(BUILD_PLATFORM_TYPE) == ApolloLakeI_LeafHill
    gChipsetPkgTokenSpaceGuid.PcdBuildPlatformType|0x07
  !endif
  
  !if $(BUILD_PLATFORM_TYPE) == ApolloLakeI_JuniperHill
    gChipsetPkgTokenSpaceGuid.PcdBuildPlatformType|0x08
  !endif
#[-end-160406-IB07400715-add]#
#[-start-160819-IB07400772-add]#
#[-start-161018-IB07400798-modify]#
  gEfiMdeModulePkgTokenSpaceGuid.PcdSerialRegisterBase|$(UART_DEBUG_IO_PORT)
#[-end-161018-IB07400798-modify]#
#[-end-160819-IB07400772-add]#
#[-start-161128-IB07400820-add]#
  #
  # Apollo Lake GPIO GPE event Group
  #
  # SOUTHWEST GPIO pin  0 ~ 31 belong to BXTP GROUP0
  # SOUTHWEST GPIO pin 32 ~ 42 belong to BXTP GROUP1
  # WEST      GPIO pin  0 ~ 25 belong to BXTP GROUP2
  # NORTHWEST GPIO pin  0 ~ 17 belong to BXTP GROUP4
  # NORTHWEST GPIO pin 32 ~ 63 belong to BXTP GROUP5
  # NORTHWEST GPIO pin 64 ~ 76 belong to BXTP GROUP6
  # NORTH     GPIO pin  0 ~ 31 belong to BXTP GROUP7
  # NORTH     GPIO pin 32 ~ 61 belong to BXTP GROUP8
  #
  gChipsetPkgTokenSpaceGuid.PcdGpe0bGpioGroup|0x07
  gChipsetPkgTokenSpaceGuid.PcdGpe0cGpioGroup|0x00
  gChipsetPkgTokenSpaceGuid.PcdGpe0dGpioGroup|0x01
#[-end-161128-IB07400820-add]#
#[-start-161216-IB07400826-add]#
!if $(GPIO_POWER_BUTTON_EVNET_EXAMPLE) == YES
  #
  # Example Pin: N_GPIO_44, GPE 0x4C(12(c) + 32(b) + 32(a))
  #
  gChipsetPkgTokenSpaceGuid.PcdGpe0cGpioGroup|0x08
!endif
#[-end-161216-IB07400826-add]#

#[-start-170323-IB07400848-add]#
!if $(EFI_DEBUG) == YES
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|0
!endif
#[-end-170323-IB07400848-add]#

[PcdsPatchableInModule.common]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x803805c6
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0x$(PLATFORM_PCIEXPRESS_BASE)
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdLegacyBiosCacheLegacyRegion|FALSE

################################################################################
#
# Pcd Dynamic Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsDynamicDefault]
#[-start-161006-IB05400716-modify]#
  gInsydeTokenSpaceGuid.PcdPreserveMemoryTable|{ \
     UINT32(0x09), UINT32(0x40),   \ # Preserve 256K(0x40 pages) for ASL (EfiACPIReclaimMemory)
     UINT32(0x0a), UINT32(0x100),  \ # Preserve 1M(0x100 pages) for S3, SMM, etc (EfiACPIMemoryNVS)
     UINT32(0x00), UINT32(0x2500), \ # Preserve 37M(0x2500 pages) for BIOS reserved memory (EfiReservedMemoryType)
     UINT32(0x06), UINT32(0x500),  \ # Preserve 5M(0x500 pages) for UEFI OS runtime data to make S4 memory consistency (EfiRuntimeServicesData)
     UINT32(0x05), UINT32(0x290),  \ # Preserve 2624K(0x290 pages) for UEFI OS runtime drivers to make S4 memory consistency (EfiRuntimeServicesCode)
     UINT32(0x03), UINT32(0x1000), \ # Preserve 16M(0x1000 pages) for boot service drivers to reduce memory fragmental (EfiBootServicesCode)
     UINT32(0x04), UINT32(0xB800), \ # Preserve 184M(0xB800 pages) for boot service data to reduce memory fragmental (EfiBootServicesData)
     UINT32(0x01), UINT32(0x200),  \ # Preserve 2M(0x200 pages) for UEFI OS boot loader to keep on same address (EfiLoaderCode)
     UINT32(0x02), UINT32(0x100),  \ # Preserve 1M(0x100 pages) for UEFI OS boot loader data to reduce memory fragmental (EfiLoaderData)
     UINT32(0x0e), UINT32(0x00)}     #EndEntry
#[-end-161006-IB05400716-modify]#
#[-start-171018-IB07400919-add]#
!if $(H2O_FORM_BROWSER_SUPPORT) == YES AND $(H2O_FORM_BROWSER_METRO_SUPPORT) == YES
  gInsydeTokenSpaceGuid.PcdPreserveMemoryTable|{ \
     UINT32(0x09), UINT32(0x40),   \ # Preserve 256K(0x40 pages) for ASL (EfiACPIReclaimMemory)
     UINT32(0x0a), UINT32(0x100),  \ # Preserve 1M(0x100 pages) for S3, SMM, etc (EfiACPIMemoryNVS)
     UINT32(0x00), UINT32(0x6000), \ # Preserve 100M(0x6000 pages) for BIOS reserved memory (EfiReservedMemoryType)
     UINT32(0x06), UINT32(0x500),  \ # Preserve 5M(0x500 pages) for UEFI OS runtime data to make S4 memory consistency (EfiRuntimeServicesData)
     UINT32(0x05), UINT32(0x290),  \ # Preserve 2624K(0x290 pages) for UEFI OS runtime drivers to make S4 memory consistency (EfiRuntimeServicesCode)
     UINT32(0x03), UINT32(0x1000), \ # Preserve 16M(0x1000 pages) for boot service drivers to reduce memory fragmental (EfiBootServicesCode)
     UINT32(0x04), UINT32(0x7D00), \ # Preserve 125M(0x7D00 pages) for boot service data to reduce memory fragmental (EfiBootServicesData)
     UINT32(0x01), UINT32(0x200),  \ # Preserve 2M(0x200 pages) for UEFI OS boot loader to keep on same address (EfiLoaderCode)
     UINT32(0x02), UINT32(0x100),  \ # Preserve 1M(0x100 pages) for UEFI OS boot loader data to reduce memory fragmental (EfiLoaderData)
     UINT32(0x0e), UINT32(0x00)}     #EndEntry
!endif
#[-end-171018-IB07400919-add]#
  #
  # Crisis File name definition
  #
  # New File Path Definition : //Volume_Label\\File_Path\\File_Name
  # Notice : "//" is signature that volume label start definition.
  #
  # Example path : //RECOVERY\\BIOS\\Current\\Chv_Ia32_RELEASE.ROM
#[-start-160505-IB07220072-modify]#
#[-start-160613-IB07400744-modify]#
#  !if $(X64_CONFIG) == YES
#    gInsydeTokenSpaceGuid.PcdPeiRecoveryFile|L"APL_X64_RELEASE.ROM"|VOID*|0x100
#  !else
#    gInsydeTokenSpaceGuid.PcdPeiRecoveryFile|L"APL_Ia32_RELEASE.ROM"|VOID*|0x100
#  !endif
  
  #
  # Note: Please modify file Name for CD/DVD recovery, iso9660 Level1 8.3 format.
  #       File names are limited to eight characters with a three-character extension.
  #
#  gInsydeTokenSpaceGuid.PcdPeiRecoveryFile|L"BSW.ROM"|VOID*|0x100
  gInsydeTokenSpaceGuid.PcdPeiRecoveryFile|L"$(BUILD_BIOS_NAME).bin"|VOID*|0x100
#[-end-160613-IB07400744-modify]#
#[-start-170208-IB07400839-add]#
  #
  # Note: This Short File Name used by CD/DVD recovery, iso9660 Level1 8.3 format.
  #       File names are limited to eight characters with a three-character extension.
  #
  gChipsetPkgTokenSpaceGuid.PcdPeiRecoveryShortFileName|L"APL.bin"|VOID*|0x100
#[-end-170208-IB07400839-add]#
#[-end-160505-IB07220072-modify]#
#  gEfiBxtTokenSpaceGuid.PcdBmpImageGuid |{0x87, 0x8A, 0xC2, 0xCC, 0x53, 0x43, 0x46, 0xF2, 0xB5, 0x63, 0x51, 0xF8, 0x9D, 0xAF, 0x56, 0xBA}
  #
  # Platform Flavor  1=mobile,2=desktop,3=tablet
  # 
  gChipsetPkgTokenSpaceGuid.PcdPlatformFlavor|0x00000001

  #
  # Variable Base will be updated by PcdSet32 during PEI phase
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageVariableBase|0xFFF7D000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwWorkingBase|0xFFF9B000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFlashNvStorageFtwSpareBase|0xFFF9C000
  gInsydeTokenSpaceGuid.PcdFlashNvStorageFactoryCopyBase|0x00000000

#[-start-170517-IB07400866-remove]#
# !include $(PROJECT_PKG)/ApolloLakeGpioTable.dsc
#[-end-170517-IB07400866-remove]#

[PcdsDynamicExDefault]
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType000|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType001|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType002|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType003|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType008|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType009|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType011|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType012|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType013|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType015|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType021|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType022|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType026|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType027|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType028|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType032|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType039|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType041|TRUE
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType131|TRUE
#[PRJ]+ >>>> Modify for support  VirtualEEPROMVerifyTool and CMFCVerify
  gSmbiosTokenSpaceGuid.PcdActiveSmbiosType255|TRUE 
#[PRJ]+ <<<< Modify for support  VirtualEEPROMVerifyTool and CMFCVerify
  gSmbiosTokenSpaceGuid.PcdSmbiosMaxMultiRecords |32
  gSmbiosTokenSpaceGuid.PcdSmbiosMultiRecordsType|{2, 3, 4, 7, 8, 9, 17, 21, 22, 26, 27, 28, 29, 39, 41}

  gSmbiosTokenSpaceGuid.PcdType000Record | { \
      0x00,                       \ # Type
      0x18,                       \ # Length
      UINT16(0x0000),             \ # Handle
      0x01,                       \ # Vendor    
      0xFF,                       \ # BIOS Version   
      UINT16(0xF000),             \ # BIOS Starting Address Segment
      0x03,                       \ # BIOS Release Date   
      0x2F,                       \ # BIOS ROM Size       
      UINT64(0x0000000078099880), \ # BIOS Characteristics
      UINT16(0x0723),             \ # BIOS Characteristics Extension Bytes
      0x05,                       \ # System BIOS Major Release
      0x04,                       \ # System BIOS Minor Release
      0x00,                       \ # Embedded Controller Firmware Major Release
      0x00                        \ # Embedded Controller Firmware Minor Release
  }
#[-start-160504-IB07400722-modify]#
#  gSmbiosTokenSpaceGuid.PcdType000Strings|"INSYDE Corp.;APL.05.11.16.0000;04/28/2016;"
!if $(GEN_PROJECT_UNI) == YES
!if $(NO_TIME_STAMP) == YES
  gSmbiosTokenSpaceGuid.PcdType000Strings|$(SMBIOS_TYPE00_STRINGS)
!else  
  gSmbiosTokenSpaceGuid.PcdType000Strings|$(GEN_SMBIOS_TYPE00)
!endif
!else
  gSmbiosTokenSpaceGuid.PcdType000Strings|$(SMBIOS_TYPE00_STRINGS)
!endif
#[-end-160504-IB07400722-modify]#

  gSmbiosTokenSpaceGuid.PcdType001Record |{0x01, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x78, 0x56, 0x34, 0x12, 0x34, 0x12, 0x78, 0x56, 0x90, 0xAB, 0xCD, 0xDE, 0xEF, 0xAA, 0xBB, 0xCC, 0x06, 0x05, 0x06}
  gSmbiosTokenSpaceGuid.PcdType001Strings|"Insyde;ApolloLake;Type1 - TBD by OEM;Type1 - 123456789;Type1 - SKU0;Type1 - Family;"

  gSmbiosTokenSpaceGuid.PcdType002Record000 |{0x02, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x09, 0x06, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00}
#[PRJ]+ >>>> Modify for support  VirtualEEPROMVerifyTool and CMFCVerify  
  gSmbiosTokenSpaceGuid.PcdType002Strings000|" ;FR1AA;V0.01;Type2 - Board Serial Number;Type2 - Board Asset Tag;Type2 - Board Chassis Location;"      #[PRJ]Modify for VirtualEEPROMVerifyTool and MCMFCVerify_V1.0.0.9
#[PRJ]+ <<<< Modify for support  VirtualEEPROMVerifyTool and CMFCVerify                                               
  gSmbiosTokenSpaceGuid.PcdType003Record000 |{0x03, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x02, 0x03, 0x04, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05}
  gSmbiosTokenSpaceGuid.PcdType003Strings000|"Chassis Manufacturer;Chassis Version;Chassis Serial Number;Chassis Asset Tag;SKU Number;"

  gSmbiosTokenSpaceGuid.PcdType008Record000 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x12, 0x10}
  gSmbiosTokenSpaceGuid.PcdType008Strings000|"J1A2;USB2.0;"
  gSmbiosTokenSpaceGuid.PcdType008Record001 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x12, 0x10}
  gSmbiosTokenSpaceGuid.PcdType008Strings001|"J1A2;USB2.0;"
  gSmbiosTokenSpaceGuid.PcdType008Record002 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x12, 0x10}
  gSmbiosTokenSpaceGuid.PcdType008Strings002|"J2A2;USB3.0 Port0;"
  gSmbiosTokenSpaceGuid.PcdType008Record003 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x12, 0x10}
  gSmbiosTokenSpaceGuid.PcdType008Strings003|"J9D1;UART to Micro USB;"
  gSmbiosTokenSpaceGuid.PcdType008Record004 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0xFF, 0xFF}
  gSmbiosTokenSpaceGuid.PcdType008Strings004|"CON6A1;SD Card;"
  gSmbiosTokenSpaceGuid.PcdType008Record005 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x0F, 0x0D}
  gSmbiosTokenSpaceGuid.PcdType008Strings005|"J9B1;Keyboard;"
  gSmbiosTokenSpaceGuid.PcdType008Record006 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x0F, 0x0E}
  gSmbiosTokenSpaceGuid.PcdType008Strings006|"J9B1;Mouse;"
  gSmbiosTokenSpaceGuid.PcdType008Record007 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x1F, 0x1D}
  gSmbiosTokenSpaceGuid.PcdType008Strings007|"J5A1;Microphone;"
  gSmbiosTokenSpaceGuid.PcdType008Record008 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x1F, 0x1D}
  gSmbiosTokenSpaceGuid.PcdType008Strings008|"J5A1;Line In;"
  gSmbiosTokenSpaceGuid.PcdType008Record009 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x1F, 0x1D}
  gSmbiosTokenSpaceGuid.PcdType008Strings009|"J5A1;Line Out;"
  gSmbiosTokenSpaceGuid.PcdType008Record010 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x1F, 0x1D}
  gSmbiosTokenSpaceGuid.PcdType008Strings010|"J7A1;Audio Jack;"
  gSmbiosTokenSpaceGuid.PcdType008Record011 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x0B, 0x1F}
  gSmbiosTokenSpaceGuid.PcdType008Strings011|"J2A2;Network Rj45 Jack;"
  gSmbiosTokenSpaceGuid.PcdType008Record012 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x22, 0xFF}
  gSmbiosTokenSpaceGuid.PcdType008Strings012|"J4J1;SATA;"
  gSmbiosTokenSpaceGuid.PcdType008Record013 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x22, 0x02, 0x00, 0xFF}
  gSmbiosTokenSpaceGuid.PcdType008Strings013|"J4E2;SATA Cable;"
  gSmbiosTokenSpaceGuid.PcdType008Record014 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x07, 0x1C}
  gSmbiosTokenSpaceGuid.PcdType008Strings014|"J4A1;VGA;"
  gSmbiosTokenSpaceGuid.PcdType008Record015 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0xFF, 0x1C}
  gSmbiosTokenSpaceGuid.PcdType008Strings015|"J1A1;Display Port;"
  gSmbiosTokenSpaceGuid.PcdType008Record016 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0xFF, 0x1C}
  gSmbiosTokenSpaceGuid.PcdType008Strings016|"J3A1;Display Port;"
  gSmbiosTokenSpaceGuid.PcdType008Record017 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x08, 0x09}
  gSmbiosTokenSpaceGuid.PcdType008Strings017|"J9B4;Serial Port;"
  gSmbiosTokenSpaceGuid.PcdType008Record018 |{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0xFF, 0xFF}
  gSmbiosTokenSpaceGuid.PcdType008Strings018|"J2A1;HDMI;"

  gSmbiosTokenSpaceGuid.PcdType009Record000 |{0x09, 0x00, 0x00, 0x00, 0x01, 0xA8, 0x0A, 0x03, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00}
  gSmbiosTokenSpaceGuid.PcdType009Strings000|"J5C1;"
  gSmbiosTokenSpaceGuid.PcdType009Record001 |{0x09, 0x00, 0x00, 0x00, 0x01, 0xA6, 0x08, 0x03, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00}
  gSmbiosTokenSpaceGuid.PcdType009Strings001|"J4B3;"
  gSmbiosTokenSpaceGuid.PcdType009Record002 |{0x09, 0x00, 0x00, 0x00, 0x01, 0xA6, 0x08, 0x03, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00}
  gSmbiosTokenSpaceGuid.PcdType009Strings002|"J6C1;"
  gSmbiosTokenSpaceGuid.PcdType009Record003 |{0x09, 0x00, 0x00, 0x00, 0x00, 0xA6, 0x08, 0x03, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00}
  gSmbiosTokenSpaceGuid.PcdType009Strings003|"J6D1;"

  gSmbiosTokenSpaceGuid.PcdType011Record |{0x0B, 0x00, 0x00,0x00, 0x01}
  gSmbiosTokenSpaceGuid.PcdType011Strings|"Insyde Chipset;"

  gSmbiosTokenSpaceGuid.PcdType012Record |{0x0C, 0x00, 0x00, 0x00, 0x04}
  gSmbiosTokenSpaceGuid.PcdType012Strings|"String1 for Type12 Equipment Manufacturer;String2 for Type12 Equipment Manufacturer;String3 for Type12 Equipment Manufacturer;String4 for Type12 Equipment Manufacturer;"

  gSmbiosTokenSpaceGuid.PcdType013Record |{0x0D, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}
#[-start-151124-IB07220017-modify]#
  gSmbiosTokenSpaceGuid.PcdType013Strings|"en|US|iso8859-1,0;fr|FR|iso8859-1,0;zh|TW|unicode,0;ja|JP|unicode,0;it|IT|iso8859-1,0;es|ES|iso8859-1,0;de|DE|iso8859-1,0;pt|PT|iso8859-1,0;"
#[-end-151124-IB07220017-modify]#

  gSmbiosTokenSpaceGuid.PcdType015Record |{0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x04, 0x01, 0x78, 0x56, 0x34, 0x12, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x02, 0x07, 0x00, 0x08, 0x04, 0x16, 0x00}

  gSmbiosTokenSpaceGuid.PcdType021Record000 |{0x15, 0x00, 0x00, 0x00, 0x07, 0x04, 0x02}

  gSmbiosTokenSpaceGuid.PcdType022Record000 |{0x16, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xB0, 0x04, 0xA6, 0x0E, 0x06, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00}
  gSmbiosTokenSpaceGuid.PcdType022Strings000|"I2C2;Intel SR 1;Date;123456789;SR Real Battery;CRB Battery 0;LiIon;"

  gSmbiosTokenSpaceGuid.PcdType026Record000 |{0x1A, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80}
  gSmbiosTokenSpaceGuid.PcdType026Strings000|"Voltage Probe Description;"

  gSmbiosTokenSpaceGuid.PcdType027Record000 |{0x1B, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01}
  gSmbiosTokenSpaceGuid.PcdType027Strings000|"Cooling Device Description;"

  gSmbiosTokenSpaceGuid.PcdType028Record000 |{0x1C, 0x00, 0x00, 0x00, 0x01, 0x42, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80}
  gSmbiosTokenSpaceGuid.PcdType028Strings000|"Temperature Probe Description;"

  gSmbiosTokenSpaceGuid.PcdType032Record |{0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

  gSmbiosTokenSpaceGuid.PcdType039Record000 |{0x27, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x4B, 0x00, 0xA6, 0x21, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
  gSmbiosTokenSpaceGuid.PcdType039Strings000|"OEM Define 0;OEM Define 1;OEM Define 2;OEM Define 3;OEM Define 4;OEM Define 5;OEM Define 6;"

  gSmbiosTokenSpaceGuid.PcdType041Record000 |{0x29, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x10}
  gSmbiosTokenSpaceGuid.PcdType041Strings000|"IGD;"

  gSmbiosTokenSpaceGuid.PcdType131Record |{0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x50, 0x72, 0x6F, 0x00, 0x00, 0x00, 0x00}
#[PRJ]+ >>>> Modify for support  VirtualEEPROMVerifyTool and CMFCVerify  
  gSmbiosTokenSpaceGuid.PcdType255Record |{0xFF,0x00,0x00,0x00,0x01,0x02}
  gSmbiosTokenSpaceGuid.PcdType255Strings|"_SIDLyDbRrqMsMAgDg;FR1AA;"
#[PRJ]+ <<<< Modify for support  VirtualEEPROMVerifyTool and CMFCVerify  
#[-start-160121-IB06740461-add]#
  gI2cDeviceTokenSpaceGuid.PcdI2cTouchScreen|{                                                 \  # The definition of I2C mouse PCD
    GUID({  0x234124E9,0x40B7,0x43EF,{0x9B,0x5E,0x97,0x47,0x08,0x08,0xD4,0x40}}),           \  # The unique GUID specific for this device, it will be part of device path node
    UINT32(0x00000010),                                                                     \  # Slave address
    UINT32(0x00000001),                                                                     \  # Hardware revision
    0x00,                                                                                   \  # Interrupt GPIO pin active level, 0 = low active, 1 = high active
    21,                                                                                     \  # Interrupt GPIO pin number
    UINT16(0x0001),                                                                         \  # HID descriptor register number
    UINT16(0x0d00),                                                                         \  # HID device type, 0x0000 = Non-HID device, 0x0d00 = Touch panel, 0x0102 = Mouse, 0x0106 = Keyboard
    0x03,                                                                                   \  # The index of I2C Host controller to the PcdI2cControllerTable, 0 based
    0x01,                                                                                   \  # The index of I2C bus speed configuration to the PcdI2cBusSpeedTable, 0x00 = V_SPEED_STANDARD, 0x01 = V_SPEED_FAST, 0x02 = V_SPEED_HIGH
    0x00                                                                                    \  # GPIO controller 0 based, North = 0, NorthWest = 1, West = 2, Southwest = 3, South = 4. (sync up to Platform.asl)
  }

  gI2cDeviceTokenSpaceGuid.PcdI2cTouchpad|{                                                 \  # The definition of I2C mouse PCD
    GUID({  0x234124E9,0x40B7,0x43EF,{0x9B,0x5E,0x97,0x47,0x08,0x08,0xD4,0x41}}),           \  # The unique GUID specific for this device, it will be part of device path node
    UINT32(0x0000002C),                                                                     \  # Slave address
    UINT32(0x00000001),                                                                     \  # Hardware revision
    0x00,                                                                                   \  # Interrupt GPIO pin active level, 0 = low active, 1 = high active
    18,                                                                                     \  # Interrupt GPIO pin number
    UINT16(0x0020),                                                                         \  # HID descriptor register number
    UINT16(0x0102),                                                                         \  # HID device type, 0x0000 = Non-HID device, 0x0d00 = Touch panel, 0x0102 = Mouse, 0x0106 = Keyboard
    0x04,                                                                                   \  # The index of I2C Host controller to the PcdI2cControllerTable, 0 based
    0x01,                                                                                   \  # The index of I2C bus speed configuration to the PcdI2cBusSpeedTable, 0x00 = V_SPEED_STANDARD, 0x01 = V_SPEED_FAST, 0x02 = V_SPEED_HIGH
    0x00                                                                                    \  # GPIO controller 0 based, North = 0, NorthWest = 1, West = 2, Southwest = 3, South = 4. (sync up to Platform.asl)
  }  
#[-end-160121-IB06740461-add]#

[PcdsDynamicDefault.common.DEFAULT]
  gEfiMdeModulePkgTokenSpaceGuid.PcdS3BootScriptTablePrivateDataPtr|0x0
#
# It is the same value as SecurityPkg's. Just create defatult data for it to let the other module can set the value of it. 
#
#[-start-160107-IB08450333-remove]#
#!if $(FTPM_ENABLE) == YES
#  gEfiSecurityPkgTokenSpaceGuid.PcdTpmBaseAddress|0xFED40000
#!endif
#[-end-160107-IB08450333-remove]#
#[-start-160531-IB08450347-add]#
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdFastPS2Detection|FALSE
#[-end-160531-IB08450347-add]#
  
[PcdsDynamicExDefault]
#[-start-160217-IB07400702-modify]#
  gInsydeTokenSpaceGuid.PcdH2OSdhcEmmcSupported|$(H2O_SDHC_EMMC_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2OSdhcEmmcHs200Supported|$(H2O_SDHC_EMMC_HS200_SUPPORT)
  gInsydeTokenSpaceGuid.PcdH2OSdhcSdCardSupported|$(H2O_SDHC_SDCARD_SUPPORT) 
#[-end-160217-IB07400702-modify]#

#[PRJ]+ >>>> Modify for support  VirtualEEPROMVerifyTool and CMFCVerify  
gCompalCommonCodeGuid.PcdVEEPROMTYPE|0x2  # VEEPROM Type
                                                                           # 0x00 : Not Support (Access EC EEPROM)
                                                                           # 0x01 : ROM VEEPROM (EC EEPROM with BIOS ROM Mapping)
                                                                           # 0x02 : BIOS Variable EEPROM (BIOS variable bank0~7)
                                                                           # 0x03 : Non-EC   VEEPROM (BIOS variable bank0~6)
#[PRJ]+ <<<< Modify for support  VirtualEEPROMVerifyTool and CMFCVerify  
################################################################################
#
# Library Class section - list of all Library Classes needed by this Platform.
#
################################################################################
[LibraryClasses]
#[PRJ]++ >>>> Show EC version on SCU Main page
  !disable PlatformVfrLib|$(CHIPSET_PKG)/UefiSetupUtilityDxe/PlatformVfrLib.inf
  PlatformVfrLib|$(CHIPSET_PKG)/UefiSetupUtilityDxe/PlatformVfrLib.inf{
    <SOURCE_OVERRIDE_PATH>
     $(PROJECT_PKG)/Override/$(CHIPSET_PKG)/UefiSetupUtilityDxe/
  }
  BaseOemSvcKernelLib|$(PROJECT_PKG)/Library/BaseOemSvcKernelLib/BaseOemSvcKernelLib.inf
#[PRJ]++ <<<< Show EC version on SCU Main page
  !disable PlatformBdsLib|$(CHIPSET_PKG)/Library/PlatformBdsLib/PlatformBdsLib.inf
  PlatformBdsLib|$(CHIPSET_PKG)/Library/PlatformBdsLib/PlatformBdsLib.inf{
    <SOURCE_OVERRIDE_PATH>
      $(PROJECT_PKG)/Override/$(CHIPSET_PKG)/Library/PlatformBdsLib/
  }
[LibraryClasses.common]
  
[LibraryClasses.common.SEC]

[LibraryClasses.common.PEIM]
  PeiOemSvcKernelLib|$(PROJECT_PKG)/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLib.inf
#[-start-170511-IB07400866-add]#
!if $(H2OIDE_GPIO_CONFIG_SUPPORT) == YES
  GpioCfgLib|$(PROJECT_PKG)/Library/GpioCfgLib/GpioCfgLib.inf
!endif
#[-end-170511-IB07400866-add]#

[LibraryClasses.common.PEI_CORE]
  PeiOemSvcKernelLib|$(PROJECT_PKG)/Library/PeiOemSvcKernelLib/PeiOemSvcKernelLib.inf


[LibraryClasses.common.DXE_CORE]
  DxeOemSvcKernelLib|$(PROJECT_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  DxeOemSvcKernelLib|$(PROJECT_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf
  SmmOemSvcKernelLib|$(PROJECT_PKG)/Library/SmmOemSvcKernelLib/SmmOemSvcKernelLib.inf

[LibraryClasses.common.COMBINED_SMM_DXE]  
  DxeOemSvcKernelLib|$(PROJECT_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf
  SmmOemSvcKernelLib|$(PROJECT_PKG)/Library/SmmOemSvcKernelLib/SmmOemSvcKernelLib.inf

[LibraryClasses.common.SMM_CORE]  
  DxeOemSvcKernelLib|$(PROJECT_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf
  SmmOemSvcKernelLib|$(PROJECT_PKG)/Library/SmmOemSvcKernelLib/SmmOemSvcKernelLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  DxeOemSvcKernelLib|$(PROJECT_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf
  
[LibraryClasses.common.DXE_DRIVER]  

  !disable BadgingSupportLib|InsydeModulePkg/Library/CommonPlatformLib/BadgingSupportLib/BadgingSupportLib.inf
  BadgingSupportLib|InsydeModulePkg/Library/CommonPlatformLib/BadgingSupportLib/BadgingSupportLib.inf {
    <SOURCE_OVERRIDE_PATH>
      $(PROJECT_PKG)/Override/InsydeModulePkg/Library/CommonPlatformLib/BadgingSupportLib
  }

  DxeOemSvcKernelLib|$(PROJECT_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf
  
[LibraryClasses.common.UEFI_DRIVER]
  DxeOemSvcKernelLib|$(PROJECT_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  DxeOemSvcKernelLib|$(PROJECT_PKG)/Library/DxeOemSvcKernelLib/DxeOemSvcKernelLib.inf

################################################################################
#
# Library Section - list of all EDK/Framework libraries
#
################################################################################
[Libraries.common]

[Libraries.IA32]

################################################################################
#
# Platform related components
#
################################################################################
[Components.$(PEI_ARCH)]

!disable InsydeModulePkg/H2ODebug/$(H2O_DDT_DEBUG_IO)DebugIoPei/$(H2O_DDT_DEBUG_IO)DebugIoPei.inf
!disable InsydeModulePkg/H2ODebug/DebugEnginePei/DebugEnginePei.inf
!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported
#[-start-160820-IB07400772-modify]#
!if $(H2O_DDT_DEBUG_IO) == Com AND $(H2O_DDT_DEBUG_IO_HSUART) == YES
$(PROJECT_OVERRIDE)/InsydeModulePkg/H2ODebugHsUart/$(H2O_DDT_DEBUG_IO)DebugIoPei/$(H2O_DDT_DEBUG_IO)DebugIoPei.inf
!else
$(PROJECT_OVERRIDE)/InsydeModulePkg/H2ODebug/$(H2O_DDT_DEBUG_IO)DebugIoPei/$(H2O_DDT_DEBUG_IO)DebugIoPei.inf
!endif
#[-end-160820-IB07400772-modify]#
$(PROJECT_OVERRIDE)/InsydeModulePkg/H2ODebug/DebugEnginePei/DebugEnginePei.inf
!endif
#
# Please put DXE Modules in Components.X64 whatever target is 64 or 32bit.
# The futher build process will change this groupt to [Components.IA32] if  
# target is 32 bit
#
[Components.$(DXE_ARCH)]

!disable InsydeModulePkg/H2ODebug/$(H2O_DDT_DEBUG_IO)DebugIoDxe/$(H2O_DDT_DEBUG_IO)DebugIoDxe.inf
!disable InsydeModulePkg/H2ODebug/DebugEngineDxe/DebugEngineDxe.inf
!if gInsydeTokenSpaceGuid.PcdH2ODdtSupported
#[-start-160820-IB07400772-modify]#
!if $(H2O_DDT_DEBUG_IO) == Com AND $(H2O_DDT_DEBUG_IO_HSUART) == YES
$(PROJECT_OVERRIDE)/InsydeModulePkg/H2ODebugHsUart/$(H2O_DDT_DEBUG_IO)DebugIoDxe/$(H2O_DDT_DEBUG_IO)DebugIoDxe.inf
!else
$(PROJECT_OVERRIDE)/InsydeModulePkg/H2ODebug/$(H2O_DDT_DEBUG_IO)DebugIoDxe/$(H2O_DDT_DEBUG_IO)DebugIoDxe.inf
!endif
#[-end-160820-IB07400772-modify]#
$(PROJECT_OVERRIDE)/InsydeModulePkg/H2ODebug/DebugEngineDxe/DebugEngineDxe.inf
!endif
!if $(SMSC_USBNET_SUPPORT) == YES
  $(PROJECT_PKG)/Binary/SmscUsbNetDriver/SmscUsbNetDriver.inf
!endif
#[-start-190722-IB16530042-add]#
ApolloLakeBoardPkg/Tools/Source/Shell/TriggerRecovery/TriggerRecovery.inf
#[-end-190722-IB16530042-add]#

  !disable $(CHIPSET_PKG)/OEMBadgingSupportDxe/OEMBadgingSupportDxe.inf
  $(CHIPSET_PKG)/OEMBadgingSupportDxe/OEMBadgingSupportDxe.inf {
    <SOURCE_OVERRIDE_PATH>
    $(PROJECT_PKG)/Override/$(CHIPSET_PKG)/OEMBadgingSupportDxe/
  }
  
#[PRJ]++ >>>> Show EC version on SCU Main page
  !disable $(CHIPSET_PKG)/UefiSetupUtilityDxe/SetupUtilityDxe.inf
  $(CHIPSET_PKG)/UefiSetupUtilityDxe/SetupUtilityDxe.inf {
    <SOURCE_OVERRIDE_PATH>
    $(PROJECT_PKG)/Override/$(CHIPSET_PKG)/UefiSetupUtilityDxe/
  }  
  $(PROJECT_PKG)/OemConfig/Smm/IhisiOemSmm/IhisiOemSmm.inf

#[PRJ]++ <<<< Show EC version on SCU Main page  
#[-start-160217-IB07400702-add]#
[BuildOptions]
  DEFINE EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS      = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS) $(CC_FLAGS)     
  DEFINE EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC  = $(EDK_EDKII_DSC_FEATURE_BUILD_OPTIONS_GCC) $(CC_FLAGS)     
#[-end-160217-IB07400702-add]#

#[-start-190722-IB16530041-modify]#
#[-start-190402-H2OIDE-5.0.14.52-IB16530009-add]#
#[-start-190722-IB16530042-remove]#
#[Components]
#	ApolloLakeBoardPkg/Tools/Source/Shell/TriggerRecovery/TriggerRecovery.inf
#[-end-190722-IB16530042-remove]#
#[-end-190402-H2OIDE-5.0.14.52-IB16530009-add]#
#[-end-190722-IB16530041-modify]#

