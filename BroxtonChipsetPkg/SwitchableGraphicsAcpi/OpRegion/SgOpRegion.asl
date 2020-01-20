/** @file
;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

Scope (DGPU_SCOPE) {
    OperationRegion (SGOP, SystemMemory, 0x55AA55AA, 0x55AA55AA)
    Field (SGOP, AnyAcc, Lock, Preserve)
    {
        XBAS,  32,  // Any Device's PCIe Config Space Base Address
        DBPA,  32,  // dGPU bridge's PCIe Address
        DDPA,  32,  // dGPU device's PCIe Base Address
        EECP,  32,  // PEG Endpoint PCIe Capability Structure Base Address
        SGGP,   8,  // SG GPIO Support
        GBAS,  32,  // GPIO Base Address
        APDT,  32,  // Active dGPU_PWR_EN Delay time
        AHDT,  32,  // Active dGPU_HOLD_RST_PIN Delay time
        IHDT,  32,  // Inactive dGPU_HOLD_RST_PIN Delay time
        DSSV,  32,  // dGPU SSID SVID
        HRCA,  32,  // dGPU HLD RST GPIO Community Offset
        HRPA,  32,  // dGPU HLD RST GPIO Pin Offset
        HRAB,   8,  // dGPU HLD RST GPIO Active Information
        PECA,  32,  // dGPU PWR Enable GPIO Community Offset
        PEPA,  32,  // dGPU PWR Enable GPIO Pin Offset
        PEAB,   8,  // dGPU PWR Enable GPIO Active Information
        ENBA,   8,  // dGPU Hold Rst Enable Bit Value
        DISA,   8,  // dGPU Hold Rst Disable Bit Value
        ENPA,   8,  // dGPU Pwr Enable Bit Value
        DIPA,   8,  // dGPU Pwr Disable Bit Value
    }
}
