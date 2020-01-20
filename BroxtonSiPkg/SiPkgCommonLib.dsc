## @file
#  Component description file for the Broxton RC both Pei and Dxe libraries DSC file.
#
# @copyright
#  INTEL CONFIDENTIAL
#  Copyright 2004 - 2016 Intel Corporation.
#
#  The source code contained or described herein and all documents related to the
#  source code ("Material") are owned by Intel Corporation or its suppliers or
#  licensors. Title to the Material remains with Intel Corporation or its suppliers
#  and licensors. The Material may contain trade secrets and proprietary and
#  confidential information of Intel Corporation and its suppliers and licensors,
#  and is protected by worldwide copyright and trade secret laws and treaty
#  provisions. No part of the Material may be used, copied, reproduced, modified,
#  published, uploaded, posted, transmitted, distributed, or disclosed in any way
#  without Intel's prior express written permission.
#
#  No license under any patent, copyright, trade secret or other intellectual
#  property right is granted to or conferred upon you by disclosure or delivery
#  of the Materials, either expressly, by implication, inducement, estoppel or
#  otherwise. Any license under such intellectual property rights must be
#  express and approved by Intel in writing.
#
#  Unless otherwise agreed by Intel in writing, you may not remove or alter
#  this notice or any other notice embedded in Materials by Intel or
#  Intel's suppliers or licensors in any way.
#
#  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
#  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
#  the terms of your license agreement with Intel or your vendor. This file may
#  be modified by the user, subject to additional terms of the license agreement.
#
# @par Specification Reference:
#
# @par Glossary:
###

#
# RC Common Library
#

#
# MdePkg
#


#
# Common
#
  AslUpdateLib|$(PLATFORM_SI_PACKAGE)/Library/DxeAslUpdateLib/DxeAslUpdateLib.inf
  ConfigBlockLib|ClientSiliconPkg/Library/BaseConfigBlockLib/BaseConfigBlockLib.inf
  BaseIpcLib|$(PLATFORM_SI_PACKAGE)/Library/PmcIpcLib/BaseIpcLib.inf
  GpioLib|$(PLATFORM_SI_PACKAGE)/Library/GpioLib/GpioLib.inf
  MmPciLib|$(PLATFORM_SI_PACKAGE)/Library/PeiDxeSmmMmPciLib/PeiDxeSmmMmPciLib.inf
  PreSiliconLib|$(PLATFORM_SI_PACKAGE)/Library/PreSiliconLib/PreSiliconLib.inf
  SideBandLib|$(PLATFORM_SI_PACKAGE)/Library/SideBandLib/SideBandLib.inf
  SteppingLib|$(PLATFORM_SI_PACKAGE)/Library/SteppingLib/SteppingLib.inf
  CseVariableStorageLib|$(PLATFORM_SI_PACKAGE)/Library/Private/BaseCseVariableStorageLib/BaseCseVariableStorageLib.inf
  VariableNvmStorageLib|$(PLATFORM_SI_PACKAGE)/Library/BaseVariableNvmStorageLib/BaseVariableNvmStorageLib.inf
  BaseCseVariableStorageSelectorLib|$(PLATFORM_SI_PACKAGE)/Library/Private/BaseCseVariableStorageSelectorLib/BaseCseVariableStorageSelectorLib.inf

#
# SC
#
  HsioLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/PeiDxeSmmHsioLib/PeiDxeSmmHsioLib.inf
  I2cLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/I2CLib/I2CLib.inf
  PeiDxeSmmScPciExpressHelpersLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiDxeSmmScPciExpressHelpersLib/PeiDxeSmmScPciExpressHelpersLib.inf
  PeiVtdLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/PeiVtdLib/PeiVtdLib.inf
  SataInitLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiDxeSataInitLib/PeiDxeSataInitLib.inf
  ScInitCommonLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiDxeSmmScInitCommonLib/PeiDxeSmmScInitCommonLib.inf
  ScPlatformLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/ScPlatformLib/ScPlatformLib.inf
  ScSpiCommonLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/BaseScSpiCommonLib/BaseScSpiCommonLib.inf
  ScSmbusCommonLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiDxeSmmScSmbusCommonLib/PeiDxeSmmScSmbusCommonLib.inf
  UsbCommonLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiDxeUsbCommonLib/PeiDxeUsbCommonLib.inf
  ScAslUpdateLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/SampleCode/Library/AslUpdate/Dxe/ScAslUpdateLib.inf
  ScHdaLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/DxeScHdaLib/DxeScHdaLib.inf
  DxeVtdLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/DxeVtdLib/DxeVtdLib.inf
#[-start-151126-IB02950555-add]#
 PeiDxeSmmScPciExpressHelpersLib|$(PLATFORM_SI_PACKAGE)/SouthCluster/Library/Private/PeiDxeSmmScPciExpressHelpersLib/PeiDxeSmmScPciExpressHelpersLib.inf
#[-end-151126-IB02950555-add]#
#
# NC
#
  DxeSaPolicyLib|$(PLATFORM_SI_PACKAGE)/NorthCluster/Library/DxeSaPolicyLib/DxeSaPolicyLib.inf

#
# CPU
#
  BootGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeSmmBootGuardLib/PeiDxeSmmBootGuardLib.inf
  CpuCommonLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiDxeSmmCpuCommonLib/PeiDxeSmmCpuCommonLib.inf
  CpuMailboxLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeSmmCpuMailboxLib/PeiDxeSmmCpuMailboxLib.inf
  CpuPlatformLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeSmmCpuPlatformLib/PeiDxeSmmCpuPlatformLib.inf
  PeiTxtLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiTxtLibNull/PeiTxtLibNull.inf
#  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf  
  MtrrLib|UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
  LocalApicLib|UefiCpuPkg/Library/BaseXApicX2ApicLib/BaseXApicX2ApicLib.inf
  SmbiosCpuLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiSmbiosCpuLib/PeiSmbiosCpuLib.inf


#
# TXE
#
  HeciInitLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/Private/PeiDxeHeciInitLib/PeiDxeHeciInitLib.inf
  SeCChipsetLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/PeiDxeSeCChipsetLib/PeiDxeSeCChipsetLib.inf
  Heci2PowerManagementLib|$(PLATFORM_SI_PACKAGE)/Txe/Library/BaseHeci2PowerManagementNullLib/BaseHeci2PowerManagementNullLib.inf
