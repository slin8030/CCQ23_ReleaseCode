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
 1.00      2016.03.30   Stan_Chen    Init version.

*/

#define TPNL_DRT_ASSIGN           0x00                  // TPNL_DRT_ASSIGN : assign the _HID and _CID directly
#define TPNL_RD_TPLF              0x01                  // TPNL_RD_TPLF  : read touchpanel flag from TPLF of NVS to 
                                                        //                 select touchpanel _CID and _HID.
#ifndef GPIOINT
  #define GPIOINT                 0x00
#endif
#ifndef INTERRUPT
  #define INTERRUPT               0x01
#endif

#if PROJECT_I2C_TOUCHPNL_ENABLE
Scope(PROJECT_TPNL_I2C_PATH){

  Device (TPNL)
  {
    Name (_ADR, One)
    Name (_UID, One)
    Name (_S0W, 4)
    Name (HID2, 0)                         // HID Descriptor Address (IHV Specific) 
    Name (TPID, Package(){TPNL_ID_TABLE})  // define the TouchPanel table in Oemasl.asl

    Name (SBFB, ResourceTemplate ()
    {
      I2cSerialBus (
        0x00,                              //SlaveAddress: bus address
        ControllerInitiated,               //SlaveMode: Default to ControllerInitiated
        TPNL_I2C_SPD,                      //ConnectionSpeed: in Hz
        AddressingMode7Bit,                //Addressing Mode: default to 7 bit
        TPNL_I2C_CTL,                      //ResourceSource: I2C bus controller name
        ,                                  //ResourceSourceIndex: defaults to 0
        ,                                  //ResourceUsage: Defaults to ResourceConsumer
        DEV0,                              //Descriptor Name: creates name for offset of resource descriptor
      )  //VendorData
    })

    Name (SBFI, ResourceTemplate () {
      Interrupt(ResourceConsumer, Level, ActiveLow, ExclusiveAndWake,,,) {TPNL_I2C_INTERRUPT}
    })

    Name (SBFG, ResourceTemplate () {
      GpioInt (Level, ActiveLow, Exclusive, PullUp, 0x0000, TPNL_I2C_GPIO_CTL, 0x00) {TPNL_I2C_GPIO_INT}
    })

    CreateWordField (SBFB, DEV0._ADR, ADR0)


    Method(_HID, 0x00, Serialized)
    {
      If (Not(CondRefof(TPLF)))            // if we don't define the TPLF in project
      {
        Name(TPLF, 0xff)
      }

      Switch(TPNL_ID_POLICY)               // Compal touchpanel ID policy
      {
        Case (TPNL_DRT_ASSIGN){
          Store(TPLN, TPLF)                // get the touchpanel num from Oemasl.asl
        }
        Case (TPNL_RD_TPLF){

        }
        Default {                          // returen default _CID and _HID
          Store(0xff, TPLF)
        }
      }

      return (TPDS(0x03, 0xFF, "ATML3432"))
    }// End of _HID

    Method(_CID, 0x00, Serialized)
    {
      If (Not(CondRefof(TPLF)))            // if we don't define the TPLF in project
      {
        Name(TPLF, 0xff)
      }

      Switch(TPNL_ID_POLICY)               // Compal touchpanel ID policy
      {
        Case (TPNL_DRT_ASSIGN){
          Store(TPLN, TPLF)                // get the touchpanel num from Oemasl.asl
        }
        Case (TPNL_RD_TPLF){

        }
        Default {                          // returen default _CID and _HID
          Store(0xff, TPLF)
        }
      }

      return (TPDS(0x04, 0xFF, "PNP0C50"))
    }// End of _CID


    //
    // Method(TPDS)
    // Arg0 : Package index for return _CID or _HID
    // Arg1 : default TouchPanel flag value
    // Arg2 : default _HID or _CID string
    //
    Method(TPDS, 0x03)                     // ID selection method
    {
      Store(0x00, local0)                  // Index count
      Store(0x00, local1)
      Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0)), local1)
      While(LAnd(LNotEqual(local1, Arg1),LNotEqual(local1, TPLF)))
      {
        Increment(local0)
        If(LGreaterEqual(local0, SizeOf(TPID))){ return (Arg2) }
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
          Return(HID2)
        }
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }

      // Common _DSM method for touch devices support in Win7
      If(LEqual(Arg0, ToUUID("EF87EB82-F951-46DA-84EC-14871AC6F84B"))) 
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
        // Function 1 : Report resources
        If(LEqual(Arg2, One))
        {
          Return (ConcatenateResTemplate(SBFB, SBFG))
        }
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }

      Return(Buffer(One) { 0x00 })
    }

    Method (_STA, 0, NotSerialized)
    {
      Return (0x0F)
    }

    Method (_CRS, 0, Serialized)
    {
      Store(0x00, local0)                  // Index count
      Store(0x00, local1)

      //
      // get slave address and HID descriptor address from TPID table
      //
      Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0)), local1)
      While(LAnd(LNotEqual(local1, 0xFF),LNotEqual(local1, TPLF)))
      {
        Increment(local0)
        If(LGreaterEqual(local0, SizeOf(TPID))){ break }
        Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0)), local1)
      }
      Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0x01)), ADR0)
      Store (DeRefOf (Index (DeRefOf (Index (TPID, local0)), 0x02)), HID2)

      if(LLess(OSYS,2012)) { Return (SBFI) }  // For Windows 7 only report Interrupt; it doesn't support ACPI5.0 and wouldn't understand GpioInt nor I2cBus
      if(LEqual(TPNL_I2C_INT_MOD, GPIOINT)) { Return (ConcatenateResTemplate(SBFB, SBFG)) }
      Return (ConcatenateResTemplate(SBFB, SBFI))
    }
  } // End of Device (TPNL)
}
#endif
