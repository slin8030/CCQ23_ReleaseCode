/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal’s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date         Who          Change
 --------  --------     --------     ----------------------------------------------------
 1.00      2016.02.22   Stan         Init version.

*/
#if !PROJECT_I2C_TOUCHPAD_ENABLE
scope (\_SB.PCI0.LPCB) {
  Device(PS2M)                             // PS/2 Mouse
  {
    Name (TPID, Package(){TPAD_ID_TABLE})  // define the TouchPad table in Oemasl.asl
    Method(_HID, 0x00, Serialized)
    {
      If (Not(CondRefof(TPDF)))            // if we don't define the TPDF in project
      {
        Name(TPDF, 0xff)
      }

      Switch(TPD_ID_POLICY)                // Compal touchpad ID policy
      {
        Case (TPD_DRT_ASSIGN){
          Store(TPDN, TPDF)                // get the touchpad num from Oemasl.asl
        }
        Case (TPD_RD_EEPROM){

        }
        Default {                          // returen default _CID and _HID
          Store(0xff, TPDF)
        }
      }

      return (TPDS(0x01))
    }// End of _HID

    Method(_CID, 0x00, Serialized)
    {
      If (Not(CondRefof(TPDF)))            // if we don't define the TPDF in project
      {
        Name(TPDF, 0xff)
      }

      Switch(TPD_ID_POLICY)                // Compal touchpad ID policy
      {
        Case (TPD_DRT_ASSIGN){
          Store(TPDN, TPDF)                // get the touchpad num from Oemasl.asl
        }
        Case (TPD_RD_EEPROM){

        }
        Default {                          // returen default _CID and _HID
          Store(0xff, TPDF)
        }
      }

      return (TPDS(0x02))
    }// End of _CID


    //
    // Method(TPDS)
    // Arg0 : Package index for return _CID or _HID 
    //
    Method(TPDS, 0x01)                     // ID selection method
    {
      Store(0x00, local0)                  // Index count
      Store(0x00, local1)                  
      Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0)), local1)
      While(LAnd(LNotEqual(local1, 0xFF),LNotEqual(local1, TPDF)))
      {
        Increment(local0)
        If(LGreaterEqual(local0, SizeOf(TPID)))
        {
          If(LEqual(Arg0, 0x01)){
            return ("MSFT0003")
          }elseif(LEqual(Arg0, 0x02)){
            return (EISAID("PNP0F03"))
          }
        }
        Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0)), local1)
      }
      return (DeRefOf (Index (DeRefOf (Index (TPID, local0)), Arg0)))
    }

    Method(_STA)
    {
      return (0x0F) 
    }

    Name(_CRS,ResourceTemplate()
    {
      IRQ(Edge,ActiveHigh,Exclusive){0x0C}
    })

  }// end of Device(PS2M)
}// end of Scope(\_SB.PCI0.LPCB)
#endif

#if PROJECT_I2C_TOUCHPAD_ENABLE
Scope(PROJECT_TPD_I2C_PATH){

  Device (TPAD)
  {
    Name (_ADR, One)
    Name (_UID, One)
    Name (TPID, Package(){TPAD_ID_TABLE})  // define the TouchPad table in Oemasl.asl

    Method(_HID, 0x00, Serialized)
    {
      If (Not(CondRefof(TPDF)))            // if we don't define the TPDF in project
      {
        Name(TPDF, 0xfe)
      }

      Switch(TPD_ID_POLICY)                // Compal touchpad ID policy
      {
        Case (TPD_DRT_ASSIGN){
          Store(TPDN, TPDF)                // get the touchpad num from Oemasl.asl
        }
        Case (TPD_RD_EEPROM){

        }
        Default {                          // returen default _CID and _HID
          Store(0xfe, TPDF)
        }
      }

      return (TPDS(0x01))
    }// End of _HID

    Method(_CID, 0x00, Serialized)
    {
      If (Not(CondRefof(TPDF)))            // if we don't define the TPDF in project
      {
        Name(TPDF, 0xfe)
      }

      Switch(TPD_ID_POLICY)                // Compal touchpad ID policy
      {
        Case (TPD_DRT_ASSIGN){
          Store(TPDN, TPDF)                // get the touchpad num from Oemasl.asl
        }
        Case (TPD_RD_EEPROM){

        }
        Default {                          // returen default _CID and _HID
          Store(0xfe, TPDF)
        }
      }

      return (TPDS(0x02))
    }// End of _CID

    //
    // Method(TPDS)
    // Arg0 : Package index for return _CID or _HID 
    //
    Method(TPDS, 0x01)                     // ID selection method
    {
      Store(0x00, local0)                  // Index count
      Store(0x00, local1)                  
      Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0)), local1)
      While(LAnd(LNotEqual(local1, 0xFE),LNotEqual(local1, TPDF)))
      {
        Increment(local0)
        If(LGreaterEqual(local0, SizeOf(TPID)))
        {
          If(LEqual(Arg0, 0x01)){
            return ("MSFT0001")
          }elseif(LEqual(Arg0, 0x02)){
            return ("PNP0C50")
          }
        }
        Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0)), local1)
      }
      return (DeRefOf (Index (DeRefOf (Index (TPID, local0)), Arg0)))
    }

    Method(_DSM, 0x4, NotSerialized)
    {
      // DSM UUID for HIDI2C. Do Not change.
      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero))
        {
          // Revision 1
          If(LEqual(Arg1, One))
          {
            Return(Buffer(One) { 0x03 })
          }
          Else
          {
            Return(Buffer(One) { 0x00 })
          }
        }
        // Function 1 : HID Function
        If(LEqual(Arg2, One))
        {
          // HID Descriptor Address (IHV Specific)
          Return(TPD_I2C_HID_ADR)
        }
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }
      Return(0)
    }

    Method (_STA, 0, NotSerialized)
    {
      Return (0x0F)
    }

    Method (_CRS, 0, Serialized)
    {
      Name (SBFI, ResourceTemplate ()
      {
        I2cSerialBus (
          TPD_I2C_SLAV,                    //SlaveAddress: bus address
          ControllerInitiated,             //SlaveMode: Default to ControllerInitiated
          TPD_I2C_SPD,                     //ConnectionSpeed: in Hz
          AddressingMode7Bit,              //Addressing Mode: default to 7 bit
          TPD_I2C_CTL,                     //ResourceSource: I2C bus controller name
          ,                                //ResourceSourceIndex: defaults to 0
          ,                                //ResourceUsage: Defaults to ResourceConsumer
          ,                                //Descriptor Name: creates name for offset of resource descriptor
        )  //VendorData

        GpioInt (Level, ActiveLow, Exclusive, PullUp, 0x0000, TPD_I2C_GPIO_CTL, 0x00) {TPD_I2C_GPIO_INT} //GPP_B3
      })
      Return (SBFI)
    }
  } // End of Device (TPAD)
}
#endif      

