/** @file
  Intel ACPI Reference Code for Dynamic Platform & Thermal Framework

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

Scope(\_SB)
{

  Device(TDSP)
  {
    Name(_HID, EISAID("INT3406"))  // Intel DPTF Display Device
    Name(_UID, "DPLY")
//[-start-161018-IB06740518-add]//
    Name(PTYP, 0x0A)    
//[-end-161018-IB06740518-add]//

    // _STA (Status)
    //
    // This object returns the current status of a device.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   An Integer containing a device status bitmap:
    //    Bit 0 - Set if the device is present.
    //    Bit 1 - Set if the device is enabled and decoding its resources.
    //    Bit 2 - Set if the device should be shown in the UI.
    //    Bit 3 - Set if the device is functioning properly (cleared if device failed its diagnostics).
    //    Bit 4 - Set if the battery is present.
    //    Bits 5-31 - Reserved (must be cleared).
    //
    Method(_STA)
    {
      If(LEqual(\DDSP,0)){
        Return(0x00)
      }
      Return(0x0F)
    }

    // DDDL (DPTF Display Depth Limit)
    //
    // The DDDL object indicates dynamically a lower limit on the brightness control levels currently supported by the platform
    // for the participant. Value returned must be a Percentage value that is in the _BCL brightness list.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   Brightness Display Depth Limit in percent
    //
    Method(DDDL)
    {
      Return(30) // 30% - This value must appear in the _BCL package
    }

    // DDPC ( DPTF Display Power/Performance Control)
    //
    // The DDPC object indicates dynamically a higher limit (ceiling) on the brightness control levels currently supported by
    // the platform for the participant. Value returned must be a Percentage value that is in the _BCL brightness list.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   Display Power/Performance Control in percent
    //
    Method(DDPC)
    {
      Return(80) // 80% - This value must appear in the _BCL package
    }

    // _BCL (Query List of Brightness Control Levels Supported)
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   A variable-length Package containing a list of integers representing the supported brightness 
    //   levels. Each integer has 8 bits of significant dat
    // Notes:
    //   List of supported brightness levels in the following sequence:
    //   Level when machine has full power.
    //   Level when machine is on batteries.
    //   Other supported levels.
    //
    Method(_BCL,,,,PkgObj)
    {
      Return(Package(){80, 50, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100})
    }

    // _BCM (Set the Brightness Level)
    //
    // Arguments: (1)
    //   Arg0 - An Integer containing the new brightness level
    // Return Value:
    //   None
    //
    Method(_BCM,1)
    {
      If(CondRefOf(\_SB.PCI0.GFX0.DD1F._BCM))
      {
        \_SB.PCI0.GFX0.DD1F._BCM(Arg0) // call the core Graphics method
      }
    }

    // _BQC (Brightness Query Current level)
    //
    // This method returns the current brightness level of a built-in display output device.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   An Integer containing the current brightness level (must be one of the values returned from the 
    //   _BCL method)
    //
    Method(_BQC,0)
    {
      If(CondRefOf(\_SB.PCI0.GFX0.DD1F._BQC))
      {
        Return(\_SB.PCI0.GFX0.DD1F._BQC()) // call the core Graphics method
      } Else {
        Return(0x00)
      }
    }

    // _DCS (Return the Status of Output Device)
    //
    // This method is required if hotkey display switching is supported.
    //
    // Arguments: (0)
    //  None
    // Return Value:
    //  An Integer containing the device status (32 bits)
    //   Bit Definitions:
    //   [0] - Output connector exists in the system now
    //   [1] - Output is activated
    //   [2] - Output is ready to switch
    //   [3] - Output is not defective (it is functioning properly)
    //   [4] - Device is attached (this is optional)
    //   [31:5] Reserved (must be zero)
    //
    Method(_DCS,0)
    {
      If(CondRefOf(\_SB.PCI0.GFX0.DD1F._DCS))
      {
        Return(\_SB.PCI0.GFX0.DD1F._DCS()) // call the core Graphics method
      } Else {
        Return(0x00)
      }
    }

  } // End TDSP Device
}// end Scope(\_SB)
