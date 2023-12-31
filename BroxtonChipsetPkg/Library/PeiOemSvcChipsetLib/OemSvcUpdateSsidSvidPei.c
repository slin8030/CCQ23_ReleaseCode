/** @file
  
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/PeiOemSvcChipsetLib.h>
//[-start-160217-IB03090424-remove]//
//// TODO need to figure out why need to use related path
//#include <../../../BroxtonSiPkg/SouthCluster/LibraryPrivate/PeiScInitLib/ScInitPei.h>
//[-end-160217-IB03090424-remove]//

STATIC SSID_SVID_PEI_CONFIG SvidSidInitTable[] = {
  { DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    PCI_SVID_OFFSET,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_GMM,  //GMM device
    PCI_FUNCTION_NUMBER_GMM,
    R_GMM_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_ISH,  //ISH device
    PCI_FUNCTION_NUMBER_ISH,
    R_ISH_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI2,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI1,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_SPI,
    PCI_FUNCTION_NUMBER_LPSS_SPI0,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART0,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART1,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART2,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART3,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_I2C1,
    PCI_FUNCTION_NUMBER_LPSS_I2C7,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  { DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_I2C1,
    PCI_FUNCTION_NUMBER_LPSS_I2C6,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_I2C1,
    PCI_FUNCTION_NUMBER_LPSS_I2C5,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  { 
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_I2C1,
    PCI_FUNCTION_NUMBER_LPSS_I2C4,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_I2C0,
    PCI_FUNCTION_NUMBER_LPSS_I2C3,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  { 
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_I2C0,
    PCI_FUNCTION_NUMBER_LPSS_I2C2,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_I2C0,
    PCI_FUNCTION_NUMBER_LPSS_I2C1,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_I2C0,
    PCI_FUNCTION_NUMBER_LPSS_I2C0,
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_HDA,
    PCI_FUNCTION_NUMBER_HDA,
    R_HDA_SVID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_1,
    R_PCIE_SVID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_2,
    R_PCIE_SVID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3,
    R_PCIE_SVID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_4,
    R_PCIE_SVID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_5,
    R_PCIE_SVID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2,
    PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_6,
    R_PCIE_SVID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_XHCI,
    PCI_FUNCTION_NUMBER_XHCI,
    R_XHCI_SVID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_OTG,
    PCI_FUNCTION_NUMBER_OTG,
    R_OTG_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
//[-start-160317-IB03090425-modify]//
    PCI_DEVICE_NUMBER_PMC_PWM,
    PCI_FUNCTION_NUMBER_PMC_PWM,
//[-end-160317-IB03090425-modify]//
    R_LPSS_IO_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SATA,
    PCI_FUNCTION_NUMBER_SATA,
    R_SATA_SS,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SCC_SDCARD,
    PCI_FUNCTION_NUMBER_SCC_FUNC0,
    R_SCC_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SCC_UFS,
    PCI_FUNCTION_NUMBER_SCC_FUNC0,
    R_SCC_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SCC_SDIO,
    PCI_FUNCTION_NUMBER_SCC_FUNC0,
    R_SCC_SSID,
    V_SC_DEFAULT_SVID_SID
  },
  {
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_SCC_EMMC,
    PCI_FUNCTION_NUMBER_SCC_FUNC0,
    R_SCC_SSID,
    V_SC_DEFAULT_SVID_SID
  }
};

/**
 This function offers an interface to modify SSID_SVID_PEI_CONFIG data before the system 
 update SSID/SVID in PEI phase.

 @param[in, out]   *SsidSvidTbl          On entry, points to SSID_SVID_PEI_CONFIG structure.
                                         On exit, points to SSID_SVID_PEI_CONFIG structure.

 @param[in, out]   *SsidSvidTblSize      On entry, points to a value that the length of SSID_SVID_PEI_CONFIG table 
                                         On exit, points to the updated value.


 @retval            EFI_UNSUPPORTED      Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED    Alter the Configuration Parameter.
 @retval            EFI_SUCCESS          The function performs the same operation as caller.
                                         The caller will skip the specified behavior and assuming
                                         that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateSsidSvidPei (
  IN OUT SSID_SVID_PEI_CONFIG         **SsidSvidTbl,
  IN OUT UINTN                        *SsidSvidTblSize
  )
{
  /*++
    Todo:
      Add project specific code in here.
  --*/

  
  *SsidSvidTbl = &SvidSidInitTable[0];
  *SsidSvidTblSize = (UINTN) (sizeof (SvidSidInitTable) / sizeof (SSID_SVID_PEI_CONFIG));
  return EFI_MEDIA_CHANGED;
}
