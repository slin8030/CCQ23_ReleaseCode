/** @file
  Intel Processor SSDT ACPI Code.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#include "CpuPowerMgmt.h"

DefinitionBlock (
  "CPUSSDT.aml",
  "SSDT",
  0x02,
  "CpuRef",
  "CpuSsdt",
  0x3000
  )
{
  External(\_PR.CPU0, DeviceObj)
  External(\_PR.CPU1, DeviceObj)
  External(\_PR.CPU2, DeviceObj)
  External(\_PR.CPU3, DeviceObj)
  External(\_PR.CPU0._PPC, IntObj)
 
Scope(\)
{

  // Package of pointers to SSDT's
  //
  // First column is SSDT name, used for debug only.
  // (First column must be EXACTLY eight characters.)
  // Second column is physical address.
  // Third column is table length.
  //
  // IF modifying this file, see warnings listed in ppminit.asm.
  //
  Name(SSDT,Package()
  {
    "CPU0IST ", 0x80000000, 0x80000000,
    "APIST   ", 0x80000000, 0x80000000,
    "CPU0CST ", 0x80000000, 0x80000000,
    "APCST   ", 0x80000000, 0x80000000
  })

  Name(\PDC0,0x80000000)  // CPU0 _PDC Flags.
  Name(\PDC1,0x80000000)  // CPU1 _PDC Flags.
  Name(\PDC2,0x80000000)  // CPU2 _PDC Flags.
  Name(\PDC3,0x80000000)  // CPU3 _PDC Flags.

  Name(\SDTL,0x00)        // Loaded SSDT Flags.
  // SDTL[1]   = CPU0 CST SSDT Loaded
  // SDTL[3]   = CPU0 IST SSDT Loaded
  // SDTL[4]   = AP IST SSDT Loaded
  // SDTL[5]   = AP CST SSDT Loaded
  // SDTL[6]   = Cpu0Hwp SSDT Loaded
  // SDTL[7]   = Ap0Hwp SSDT Loaded
}

Scope(\_PR)
{
  //
  // Define a Processor scope ACPI PPM GlobalNvs NVS Region
  //
  OperationRegion(PNVS,SystemMemory,0xFFFF0000,0xAA55)
  Field(PNVS,AnyAcc,Lock,Preserve)
  {
    PGRV,       8,      // (0) CPU GlobalNvs Revision
    //
    // PPM Flags Values
    //
    CFGD,    32,       // CFGD - PpmFlags
	//
	// Number of Logical Processors Values
	//
    NLPC,    8,        // (5) Number of Logical Processors
    //
    // Thermal Configuration Values
    //
    ACRT,    8,        // (6) Auto Critical Trip Point
    APSV,    8,        // (7) Auto Passive Trip Point
    AAC0,    8,        // (8) Auto Active Trip Point
    CPID,    32,       // (9-12) CPUID
                       // 
                       // (13-40) Reserved
                       //
    Offset(41),
    //
    // Mwait Hints and Latency values for C3/C6/C7/C7S
    //
    C3MW,    8,        // (41) Mwait Hint value for C3
    C6MW,    8,        // (42) Mwait Hint value for C6
    C7MW,    8,        // (43) Mwait Hint value for C7/C7s
    CDMW,    8,        // (44) Mwait Hint value for C8/C9/C10
    C3LT,   16,        // (45-46) Latency value for C3
    C6LT,   16,        // (47-48) Latency Value for C6
    C7LT,   16,        // (49-50) Latency Value for C7/C7S
    CDLT,   16,        // (51-52) Latency Value for C8/C9/C10
    CDLV,   16,        // (53-54) IO LVL value for C8/C9/C10
    CDPW,   16,        // (55-56) Power value for C8/C9/C10
    MPMF,   8,         // (57) MiscPowerManagementFlags
    //
    // DTS
    //
    DTSE,   8,         // (58) Digital Thermal Sensor Enable
    DTS1,   8,         // (59) Digital Thermal Sensor 1 Reading
    DTS2,   8,         // (60) Digital Thermal Sensor 2 Reading
    DTSF,   8,         // (61) DTS SMI Function Call
    PDTS,   8,         // (62) Package Temperature
    PKGA,   8,         // (63) Package Temperature MSR available
    DTS3,   8,         // (64) Digital Thermal Sensor 3 Reading
    DTS4,   8,         // (65) Digital Thermal Sensor 4 Reading
                       // 
                       // (66-82) Reserved
                       //
    Offset(83),
    //
    // SGX
    //
    EPCS, 8,           // (83) SGX Status
    EMNA, 64,          // (84-91) EPC Base Address
    ELNG, 64,          // (92-99) EPC Length
    //
    // HWP
    //
    HWPV, 8,           // (100) HWP Version
    HWPA, 16,          // (101-102) IoTrap Address for HWP
    HWPL, 16,          // (103-104) IoTrap Length for HWP
    POWS, 8,           // (105) Power State
    HDCE, 8,           // (106) Hardware Duty Cycling Policy
    HWPI, 8,           // (107) HWP Interrupt Status
	MWEN, 8            // (108) Mwait Enable
  }
}

Scope(\_PR.CPU0)
{
  //
  // Define handles for opregions (used by load.)
  //
  Name(HI0,0)    // Handle to CPU0IST
  Name(HC0,0)    // Handle to CPU0CST

  Method(_PDC,1)
  {
    //
    // Check and extract the _PDC information.
    //
    Store(CPDC(Arg0), Local0)
    //
    // Save the capability information and load tables as needed.
    //
    GCAP(Local0)
  }

  Method(_OSC, 4)
  {
    //
    // Check and extract the _OSC information.
    //
    Store(COSC(Arg0, Arg1, Arg2, Arg3), Local0)
    //
    // Save the capability information and load tables as needed.
    //
    GCAP(Local0)
    //
    // Return status.
    //
    Return (Local0)
  }

  //
  // Implement a generic Method to check _PDC information which may be called
  // by any of the processor scopes.  (The use of _PDC is deprecated in ACPI 3.
  // in favor of _OSC. However, for backwards compatibility, _PDC may be
  // implemented using _OSC as follows:)
  //
  Method(CPDC,1, Serialized)
  {
    CreateDwordField (Arg0, 0, REVS)
    CreateDwordField (Arg0, 4, SIZE)

    //
    // Local0 = Number of bytes for Arg0
    //
    Store (SizeOf (Arg0), Local0)

    //
    // Local1 = Number of Capabilities bytes in Arg0
    //
    Store (Subtract (Local0, 8), Local1)

    //
    // TEMP = Temporary field holding Capability DWORDs
    //
    CreateField (Arg0, 64, Multiply (Local1, 8), TEMP)

    //
    // Create the Status (STAT) buffer with the first DWORD = 0
    // This is required as per ACPI 3.0 Spec which says the
    // first DWORD is used to return errors defined by _OSC.
    //
    Name (STS0, Buffer () {0x00, 0x00, 0x00, 0x00})

    //
    // Concatenate the _PDC capabilities bytes to the STS0 Buffer
    // and store them in a local variable for calling OSC
    //
    Concatenate (STS0, TEMP, Local2)

    Return(COSC (ToUUID("4077A616-290C-47BE-9EBD-D87058713953"), REVS, SIZE, Local2))
  }

  //
  // Implement a generic Method to check _OSC information which may be called by any of the processor scopes.
  //
  // Arguments: (4)
  //   Arg0 - A Buffer containing a UUID
  //   Arg1 - An Integer containing a Revision ID of the buffer format
  //   Arg2 - An Integer containing a count of entries in Arg3
  //   Arg3 - A Buffer containing a list of DWORD capabilities
  // Return Value:
  //   A Buffer containing a list of capabilities
  //
  Method(COSC, 4,Serialized,,BuffObj,{BuffObj,IntObj,IntObj,BuffObj})
  {
    //
    // Point to Status DWORD in the Arg3 buffer (STATUS)
    //
    If(LGreater(Arg2,0)){
      CreateDWordField(Arg3, 0, STS0)
    }

    //
    // _OSC needs to validate the UUID and Revision.
    //
    // IF Unrecognized UUID
    //  Return Unrecognized UUID _OSC Failure
    // IF Unsupported Revision
    //  Return Unsupported Revision _OSC Failure
    //
    //    STS0[0] = Reserved
    //    STS0[1] = _OSC Failure
    //    STS0[2] = Unrecognized UUID
    //    STS0[3] = Unsupported Revision
    //    STS0[4] = Capabilities masked
    //
    // Note:  The comparison method used is necessary due to
    // limitations of certain OSes which cannot perform direct
    // buffer comparisons.
    //
    // Create a set of "Input" UUID fields.
    //
    CreateDwordField(Arg0, 0x0, IID0)
    CreateDwordField(Arg0, 0x4, IID1)
    CreateDwordField(Arg0, 0x8, IID2)
    CreateDwordField(Arg0, 0xC, IID3)
    //
    // Create a set of "Expected" UUID fields.
    //
    Name(UID0, ToUUID("4077A616-290C-47BE-9EBD-D87058713953"))
    CreateDwordField(UID0, 0x0, EID0)
    CreateDwordField(UID0, 0x4, EID1)
    CreateDwordField(UID0, 0x8, EID2)
    CreateDwordField(UID0, 0xC, EID3)
    //
    // Verify the input UUID matches the expected UUID.
    //
    If(LNot(LAnd(LAnd(LEqual(IID0, EID0),LEqual(IID1, EID1)),LAnd(LEqual(IID2, EID2),LEqual(IID3, EID3)))))
    {
      //
      // Return Unrecognized UUID _OSC Failure
      //
      Store (0x6, STS0)
      Return (Arg3)
    }

    If(LNot(LEqual(Arg1,1)))
    {
      //
      // Return Unsupported Revision _OSC Failure
      //
      Store (0xA, STS0)
      Return (Arg3)
    }

    Return (Arg3)
  }

  //
  // Get the capability information and load appropriate tables as needed.
  //
  Method(GCAP, 1, Serialized)
  {
    //Name(HI0,0)    // Handle to Cpu0IST

    //
    // Point to Status DWORD in the Arg0 buffer (STATUS)
    //
    CreateDWordField(Arg0, 0, STS0)
    //
    // Point to Caps DWORDs of the Arg0 buffer (CAPABILITIES)
    //
    CreateDwordField(Arg0, 4, CAP0)

    //
    // If the UUID was unrecognized or the _OSC revision was unsupported,
    // return without updating capabilities.
    //
    If(LOr(LEqual(STS0,0x6),LEqual(STS0,0xA)))
    {
      Return()
    }

    //
    // Check if this is a query (BIT0 of Status = 1).
    // If so, mask off the bits we support and return.
    //
    if (And(STS0, 1))
    {
      And(CAP0, 0xBFF, CAP0)
      Return()
    }

    //
    // Store result of PDC. (We clear out the MSB, which was just
    // used as a placeholder for the compiler; and then "OR" the
    // value in case we get multiple calls, each of which only
    // reports partial support.)
    //
    Or(And(PDC0, 0x7FFFFFFF), CAP0, PDC0)

    //
    // Check IF the CST SSDTs should be loaded.
    //   CFGD[5:1] = C7, C6, C3, C1E, C1 Capable/Enabled
    If(And(CFGD, 0x02))
    {
      //
      // Load the CST SSDTs if:
      //   (1) CMP capable/enabled
      //   (2) Driver supports multi-processor configurations
      //   (3) CPU0 CST ISDT is not already loaded
      //
      //   CFGD[9] = Two or more cores enabled
      //   PDCx[3]  = OS supports C1 and P-states in MP systems
      //   PDCx[4]  = OS supports ind. C2/C3 in MP systems
      //   SDTL[1]  = CPU0 CST SSDT Loaded
      //
      If(LAnd(LAnd(And(CFGD, 0x02),And(PDC0,0x0018)),LNot(And(SDTL,0x02))))
      {
        //
        // Flag the CST SSDT as loaded for CPU0
        //
        Or(SDTL, 0x02, SDTL)

        OperationRegion(CST0,SystemMemory,DeRefOf(Index(SSDT,7)),DeRefOf(Index(SSDT,8)))
        Load(CST0, HC0)  // Dynamically load the CPU0CST SSDT
      }
    }

    // Load the IST SSDTs if:
    //   (1) CMP capable/enabled
    //   (2) HWP not capable or enabled
    //   (3) Driver supports multi-processor configurations
    //   (4) CPU0 IST SSDT is not already loaded
    //
    //   CFGD[0]  = EIST Capable/Enabled
    //   CFGD[22] = HWP not capable or enabled
    //   SDTL[3]  = CPU0 IST SSDT Loaded
    //
    If(LAnd(LNot(And(CFGD, 0x01)),LAnd(And(CFGD,0x01),LNot(And(SDTL,0x08)))))
    {
      //
      // Flag the IST SSDT as loaded for CPU0
      //
      Or(SDTL, 0x08, SDTL)

      OperationRegion(IST0,SystemMemory,DeRefOf(Index(SSDT,1)),DeRefOf(Index(SSDT,2)))
      Load(IST0, HI0)  // Dynamically load the CPU1IST SSDT
    }

    Return ()
  }
}


Scope(\_PR.CPU1)
{
  //
  // Define handles for opregions (used by load.)
  //
  Name(HI1,0)    // Handle to APIST
  Name(HC1,0)    // Handle to APCST
  Name(HW1,0)    // Handle to APHWP

  Method(_PDC,1)
  {
    //
    // Refer to \_PR.CPU0._PDC for description.
    //
    Store(\_PR.CPU0.CPDC(Arg0), Local0)
    GCAP(Local0)
  }

  Method(_OSC, 4)
  {
    //
    // Refer to \_PR.CPU0._OSC for description.
    //
    Store(\_PR.CPU0.COSC(Arg0, Arg1, Arg2, Arg3), Local0)
    GCAP(Local0)
    Return (Local0)
  }

  //
  // Get the capability information and load appropriate tables as needed.
  //
  Method(GCAP, 1)
  {
    //
    // Point to Status DWORD in the Arg0 buffer (STATUS)
    //
    CreateDWordField(Arg0, 0, STS1)
    //
    // Point to Caps DWORDs of the Arg0 buffer (CAPABILITIES)
    //
    CreateDwordField(Arg0, 4, CAP1)
    //
    // If the UUID was unrecognized or the _OSC revision was unsupported,
    // return without updating capabilities.
    //
    If(LOr(LEqual(STS1,0x6),LEqual(STS1,0xA)))
    {
      Return()
    }

    //
    // Check if this is a query (BIT0 of Status = 1).
    // If so, mask off the bits we support and return.
    //
    if (And(STS1, 1))
    {
      And(CAP1, 0xBFF, CAP1)
      Return()
    }

    //
    // Store result of PDC. (We clear out the MSB, which was just
    // used as a placeholder for the compiler; and then "OR" the
    // value in case we get multiple calls, each of which only
    // reports partial support.)
    //
    Or(And(PDC1, 0x7FFFFFFF), CAP1, PDC1)

    //
    // Attempt to dynamically load the IST SSDTs if:
    //   (1) Driver supports P-States in MP configurations
    //   (2) Driver supports direct HW P-State control
    //
    //   PDCx[3]  = OS supports C1 and P-states in MP systems
    //   PDCx[0]  = OS supports direct access of the perf MSR
    //
    If(LEqual(And(PDC1, 0x0009), 0x0009))
    {
      APPT()
    }

    //
    // Load Ap0Hwp SSDT
    // PDCx[5]   = OS supports CPPC from OSPM
    //
    // if(And(PDC1,0x0020)) {
    //    HWPT()
    //  }
    //
    // Load the CST SSDTs if:
    //   (1) Driver supports multi-processor configurations
    //
    //   PDCx[3]  = OS supports C1 and P-states in MP systems
    //   PDCx[4]  = OS supports ind. C2/C3 in MP systems
    //
    If(And(PDC1,0x0018))
    {
      APCT()
    }

    Store (PDC1, PDC0)

    Return()
  }

  //
  // Dynamically load the CST SSDTs if:
  //   (1) C-States are enabled
  //   (2) SSDT is not already loaded
  //
  //   CFGD[5:1] = Basic C-States supported (C1, C1E, C3, C6, C7)
  //   SDTL[5]   = AP CST SSDT Loaded
  //
  Method(APCT,0, Serialized)
  {
    If(LAnd(And(CFGD,0x02),LNot(And(SDTL,0x20))))
    {
      //
      // Flag the CST SSDT as loaded for the AP's
      //
      Or(SDTL, 0x20, SDTL)
      //
      // Dynamically load the APCST SSDT
      //
      OperationRegion(CST1,SystemMemory,DeRefOf(Index(SSDT,10)),DeRefOf(Index(SSDT,11)))
      Load(CST1, HC1)
    }
  }

  //
  // Dynamically load the IST SSDTs if:
  //   (1) If EIST capable and enabled
  //   (2) SSDT is not already loaded
  //
  //   CFGD[0] = EIST Capable/Enabled
  //   SDTL[4] = AP IST SSDT Loaded
  //
  Method(APPT,0, Serialized)
  {
    If(LAnd(And(CFGD,0x01),LNot(And(SDTL,0x10))))
    {
      //
      // Flag the IST SSDT as loaded for CPU0
      //
      Or(SDTL, 0x10, SDTL)

      OperationRegion(IST1,SystemMemory,DeRefOf(Index(SSDT,4)),DeRefOf(Index(SSDT,5)))
      Load(IST1, HI1)  // Dynamically load the CPU1IST SSDT
    }
  }
}  // End CPU1


Scope(\_PR.CPU2)
{
  Method(_PDC,1)
  {
    //
    // Call the _PDC for CPU1.
    //
    Store(\_PR.CPU0.CPDC(Arg0), Local0)
    GCAP(Local0)
  }

  Method(_OSC, 4)
  {
    //
    // Call the _OSC for CPU1.
    //
    Store(\_PR.CPU0.COSC(Arg0, Arg1, Arg2, Arg3), Local0)
    GCAP(Local0)
    Return (Local0)
  }

  Method(GCAP,1)
  {
    // Point to Status DWORD in the Arg0 buffer (STATUS)
    CreateDWordField(Arg0, 0, STS2)

    // Point to Caps DWORDs of the Arg0 buffer (CAPABILITIES)
    CreateDwordField(Arg0, 4, CAP2)

    //
    // If the UUID was unrecognized or the _OSC revision was unsupported,
    // return without updating capabilities.
    //
    If(LOr(LEqual(STS2,0x6),LEqual(STS2,0xA)))
    {
      Return()
    }

    //
    // Check if this is a query (BIT0 of Status = 1).
    // If so, mask off the bits we support and return.
    //
    if (And(STS2, 1))
    {
      And(CAP2, 0xBFF, CAP2)
      Return()
    }

    //
    // Store result of PDC. (We clear out the MSB, which was just
    // used as a placeholder for the compiler; and then "OR" the
    // value in case we get multiple calls, each of which only
    // reports partial support.)
    //
    Or(And(PDC2, 0x7FFFFFFF), CAP2, PDC2)
    //
    // Attempt to dynamically load the IST SSDTs if:
    //   (1) Driver supports P-States in MP configurations
    //   (2) Driver supports direct HW P-State control
    //
    //   PDCx[3]  = OS supports C1 and P-states in MP systems
    //   PDCx[0]  = OS supports direct access of the perf MSR
    //
    If(LEqual(And(PDC2, 0x0009), 0x0009))
    {
      \_PR.CPU1.APPT()
    }

    //
    // Load the CST SSDTs if:
    //   (1) Driver supports multi-processor configurations
    //
    //   PDCx[3]  = OS supports C1 and P-states in MP systems
    //   PDCx[4]  = OS supports ind. C2/C3 in MP systems
    //
    If(And(PDC2,0x0018))
    {
      \_PR.CPU1.APCT()
    }

    Store (PDC2, PDC0)
    Return()
  }
}  // End CPU2

Scope(\_PR.CPU3)
{
  Method(_PDC,1)
  {
    //
    // Call the _PDC for CPU1.
    //
    Store(\_PR.CPU0.CPDC(Arg0), Local0)
    GCAP(Local0)
  }

  Method(_OSC, 4)
  {
    //
    // Call the _OSC for CPU1.
    //
    Store(\_PR.CPU0.COSC(Arg0, Arg1, Arg2, Arg3), Local0)
    GCAP(Local0)
    Return (Local0)
  }

  Method(GCAP,1)
  {
    // Point to Status DWORD in the Arg0 buffer (STATUS)
    CreateDWordField(Arg0, 0, STS3)

    // Point to Caps DWORDs of the Arg0 buffer (CAPABILITIES)
    CreateDwordField(Arg0, 4, CAP3)

    //
    // If the UUID was unrecognized or the _OSC revision was unsupported,
    // return without updating capabilities.
    //
    If(LOr(LEqual(STS3,0x6),LEqual(STS3,0xA)))
    {
      Return()
    }

    //
    // Check if this is a query (BIT0 of Status = 1).
    // If so, mask off the bits we support and return.
    //
    if (And(STS3, 1))
    {
      And(CAP3, 0xBFF, CAP3)
      Return()
    }

    //
    // Store result of PDC. (We clear out the MSB, which was just
    // used as a placeholder for the compiler; and then "OR" the
    // value in case we get multiple calls, each of which only
    // reports partial support.)
    //
    Or(And(PDC3, 0x7FFFFFFF), CAP3, PDC3)
    //
    // Attempt to dynamically load the IST SSDTs if:
    //   (1) Driver supports P-States in MP configurations
    //   (2) Driver supports direct HW P-State control
    //
    //   PDCx[3]  = OS supports C1 and P-states in MP systems
    //   PDCx[0]  = OS supports direct access of the perf MSR
    //
    If(LEqual(And(PDC3, 0x0009), 0x0009))
    {
      \_PR.CPU1.APPT()
    }
    //
    // Load the CST SSDTs if:
    //   (1) Driver supports multi-processor configurations
    //
    //   PDCx[3]  = OS supports C1 and P-states in MP systems
    //   PDCx[4]  = OS supports ind. C2/C3 in MP systems
    //
    If(And(PDC3,0x0018))
    {
      \_PR.CPU1.APCT()
    }

    Store (PDC3, PDC0)
    Return()
  }
} // End CPU3
} // End of Definition Block
