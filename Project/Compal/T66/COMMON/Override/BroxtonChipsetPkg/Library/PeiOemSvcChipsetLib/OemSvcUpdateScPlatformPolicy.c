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
#include <Library/PeiScPolicyLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/SideBandLib.h>
#include <Library/BaseMemoryLib.h>
#include  "SaAccess.h"

GLOBAL_REMOVE_IF_UNREFERENCED PRIVATE_PCICFGCTRL OemDirectIrqTable[] = {
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C0,  27, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C1,  28, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C2,  29, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C3,  30, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C4,  31, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C5,  32, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C6,  33, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_I2C7,  34, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART0, 44, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART1, 45, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART2, 46, V_PCICFG_CTRL_INTC },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_UART3, 47, V_PCICFG_CTRL_INTD },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI0,  35, V_PCICFG_CTRL_INTA },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI1,  36, V_PCICFG_CTRL_INTB },
  { SB_LPSS_PORT,        R_LPSS_SB_PCICFGCTRL_SPI2,  37, V_PCICFG_CTRL_INTC },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDCARD, 40, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_EMMC  , 39, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_UFS   , 38, V_PCICFG_CTRL_INTA },
  { SB_SCC_PORT,         R_SCC_SB_PCICFGCTRL_SDIO  , 42, V_PCICFG_CTRL_INTA },
  { SB_ISH_BRIDGE_PORT,  R_ISH_SB_PCICFGCTRL_ISH   , 26, V_PCICFG_CTRL_INTA },
  { SB_USB_DEVICE_PORT,  R_USB_SB_PCICFGCTRL_XDCI  , 13, V_PCICFG_CTRL_INTB },
};

EFI_STATUS
UpdatePcieConfig (
  IN OUT SC_POLICY_PPI                         *ScPolicyPpi
  )
{
  EFI_STATUS          Status;
  SC_PCIE_CONFIG      *PcieConfig;

  PcieConfig = NULL;
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PcieConfig);

  if (EFI_ERROR(Status)) {
    return EFI_UNSUPPORTED;
  }

  PcieConfig->RootPort[0].ClkReqSupported = FALSE;
  PcieConfig->RootPort[0].ClkReqNumber    = 0xF;
  PcieConfig->RootPort[1].ClkReqSupported = FALSE;
  PcieConfig->RootPort[1].ClkReqNumber    = 0xF;
  PcieConfig->RootPort[2].ClkReqSupported = TRUE;   // port 0 wifi
  PcieConfig->RootPort[2].ClkReqNumber    = 0;
  PcieConfig->RootPort[3].ClkReqSupported = TRUE;   // port 1 LAN1
  PcieConfig->RootPort[3].ClkReqNumber    = 1;
  PcieConfig->RootPort[4].ClkReqSupported = TRUE;   // port 2 LAN2
  PcieConfig->RootPort[4].ClkReqNumber    = 2;
  PcieConfig->RootPort[5].ClkReqSupported = TRUE;   // port 3 NA
  PcieConfig->RootPort[5].ClkReqNumber    = 3;
  return EFI_MEDIA_CHANGED;
}

EFI_STATUS
UpdateSerialIrq (
  IN OUT SC_POLICY_PPI                         *ScPolicyPpi
  )
{
  SC_LPC_SIRQ_CONFIG        *SerialIrqConfig;

  SerialIrqConfig = NULL;
  GetConfigBlock ((VOID *) ScPolicyPpi, &gSerialIrqConfigGuid, (VOID *) &SerialIrqConfig);
  if (SerialIrqConfig == NULL) {
    return EFI_UNSUPPORTED;
  }

  SerialIrqConfig->SirqMode         = ScContinuousMode;
  return EFI_MEDIA_CHANGED;
}

EFI_STATUS
UpdateInterruptConfig (
  IN OUT SC_POLICY_PPI                         *ScPolicyPpi
  )
{
  SC_INTERRUPT_CONFIG        *InterruptConfig;
  UINT8                       TableSize;

  InterruptConfig = NULL;
  GetConfigBlock ((VOID *) ScPolicyPpi, &gInterruptConfigGuid, (VOID *) &InterruptConfig);
  if (InterruptConfig == NULL) {
    return EFI_UNSUPPORTED;
  }

  TableSize = sizeof (OemDirectIrqTable) / sizeof (PRIVATE_PCICFGCTRL);
  ASSERT (TableSize <= SC_MAX_DIRECT_IRQ_CONFIG);
  InterruptConfig->NumOfDirectIrqTable = TableSize;
  CopyMem (
    InterruptConfig->DirectIrqTable,
    OemDirectIrqTable,
    sizeof (OemDirectIrqTable)
    );

  return EFI_MEDIA_CHANGED;
}


EFI_STATUS
UpdateUsbConfig (
  IN OUT SC_POLICY_PPI                         *ScPolicyPpi
  )
{
  SC_USB_CONFIG              *UsbConfig;

  UsbConfig = NULL;
  GetConfigBlock ((VOID *) ScPolicyPpi, &gUsbConfigGuid, (VOID *) &UsbConfig);
  if (UsbConfig == NULL) {
    return EFI_UNSUPPORTED;
  }

//[PRJ]+>>>> Fix USB 3 port 2 no function.
  UsbConfig->PortUsb30[0].Enable = TRUE;
  UsbConfig->PortUsb30[1].Enable = TRUE;
  UsbConfig->PortUsb30[2].Enable = TRUE;
  UsbConfig->PortUsb30[3].Enable = TRUE;
  UsbConfig->PortUsb30[4].Enable = TRUE;
  UsbConfig->PortUsb30[5].Enable = TRUE;
//[PRJ]+<<<< Fix USB 3 port 2 no function.  
  return EFI_MEDIA_CHANGED;
}


/**
 This function offers an interface to modify SC_POLICY_PPI data before the system
 installs SC_POLICY_PPI.

 @param[in, out]    *ScPolicyPpi                  On entry, points to SC_POLICY_PPI structure.
                                                  On exit, points to updated SC_POLICY_PPI structure.

 @retval            EFI_UNSUPPORTED               Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED             Alter the Configuration Parameter.
 @retval            EFI_SUCCESS                   The function performs the same operation as caller.
                                                  The caller will skip the specified behavior and assuming
                                                  that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcUpdateScPlatformPolicy (
  IN OUT SC_POLICY_PPI                         *ScPolicyPpi
  )
{
  // Common design modify:
  // EC common design :set SirqMode to ScContinuousMode
  //
  UpdateSerialIrq(ScPolicyPpi);

  //
  // Update PcieConfig
  //
  UpdatePcieConfig (ScPolicyPpi);

  //
  // Update Interrupt config
  //
  UpdateInterruptConfig(ScPolicyPpi);

  //
  // Update USB config
  //
  UpdateUsbConfig(ScPolicyPpi);
 

  return EFI_MEDIA_CHANGED;
}
//[-end-140909-IB10820533-modify]//
