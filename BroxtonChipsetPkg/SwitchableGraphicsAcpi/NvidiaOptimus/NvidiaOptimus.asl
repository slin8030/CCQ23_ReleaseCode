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

#include <NvidiaOptimus.asi>

DefinitionBlock (
    "NvidiaOptimus.aml",
    "SSDT",
    1,
    "Insyde",
    "NvdTable",
    0x1000
    )
{
    External (\_SB.PCI0, DeviceObj)
    External (DGPU_BRIDGE_SCOPE, DeviceObj)
    External (DGPU_SCOPE, DeviceObj)
    External (IGPU_SCOPE, DeviceObj)
    External (DGPU_SCOPE._ADR, MethodObj)
    External (IGPU_SCOPE.CADL)
    External (IGPU_SCOPE.CAL2)
    External (IGPU_SCOPE.CAL3)
    External (IGPU_SCOPE.CAL4)
    External (IGPU_SCOPE.CAL5)
    External (IGPU_SCOPE.CAL6)
    External (IGPU_SCOPE.CAL7)
    External (IGPU_SCOPE.CAL8)
    External (IGPU_SCOPE.CDCK)
    External (IGPU_SCOPE.CPDL)
    External (IGPU_SCOPE.CPL2)
    External (IGPU_SCOPE.CPL3)
    External (IGPU_SCOPE.CPL4)
    External (IGPU_SCOPE.CPL5)
    External (IGPU_SCOPE.CPL6)
    External (IGPU_SCOPE.CPL7)
    External (IGPU_SCOPE.CPL8)
    External (IGPU_SCOPE._DOD, MethodObj)
    External (CPU0_SCOPE._PSS, BuffObj)
    External (CPU0_SCOPE.SPSS, BuffObj)
    External (CPU0_SCOPE.NPSS, BuffObj)
    External (CPU0_SCOPE._PPC, IntObj)
    External (CPU1_SCOPE._PPC, IntObj)
    External (CPU2_SCOPE._PPC, IntObj)
    External (CPU3_SCOPE._PPC, IntObj)
    External (CPU4_SCOPE, DeviceObj)
    External (CPU5_SCOPE, DeviceObj)
    External (CPU6_SCOPE, DeviceObj)
    External (CPU7_SCOPE, DeviceObj)
    External (\_TZ.TZ01._TMP)
    External (\GPRW, MethodObj)
    External (PDC0)
    External (DID1)
    External (DID2)
    External (DID3)
    External (DID4)
    External (DID5)
    External (DID6)
    External (DID7)
    External (DID8)
    External (MMCH)
    External (EC_SCOPE.EC6I)
    External (EC_SCOPE.EC6O)
    External (EC_SCOPE.FBST)
    External (\_SB.OSCO, MethodObj)
    External (P8XH, MethodObj)
    External (MBGS, MethodObj)
    External (DD2H, MethodObj)

    #include <../OpRegion/VbiosOpRegion.asl>
    #include <../OpRegion/SgOpRegion.asl>
    #include <../OpRegion/NvidiaOpRegion.asl>

    Include ("SgDgpuPch.asl")
    Include ("NvSgDsm.asl")
    Include ("NvGps.asl")
    Include ("NvGc6.asl")
    Include ("Optimus.asl")
    If (\_OSI ("Windows 2013")) {
        Include ("HybridGraphics.asl")
    }
}
