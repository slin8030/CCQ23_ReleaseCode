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

#include <AmdPowerXpress.asi>

DefinitionBlock (
    "AmdPowerXpress.aml",
    "SSDT",
    1,
    "Insyde",
    "AmdTable",
    0x1000
    )
{
    External (\_SB.PCI0, DeviceObj)
    External (DGPU_BRIDGE_SCOPE, DeviceObj)
    External (DGPU_SCOPE, DeviceObj)
    External (IGPU_SCOPE, DeviceObj)
    External (DGPU_SCOPE._ADR, MethodObj)
    External (IGPU_SCOPE._DOD, MethodObj)
    External (\GPRW, MethodObj)
    External (\_SB.OSCO, MethodObj)
    External (P8XH, MethodObj)
    External (MBGS, MethodObj)
    External (DD2H, MethodObj)

    #include <../OpRegion/VbiosOpRegion.asl>
    #include <../OpRegion/SgOpRegion.asl>
    #include <../OpRegion/AmdOpRegion.asl>

    Include ("SgDgpuPch.asl")
    Include ("Atpx.asl")
    Include ("HybridGraphics.asl")
}
