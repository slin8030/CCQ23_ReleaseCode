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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal��s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.00      2016/4/6  Vanessa      Init version.

*/

// [COMMON ASL] AC using EDK II
//
#ifndef EC_SENSORHUB_HID
  #define EC_SENSORHUB_HID        "ENES6243"
#endif

#ifndef EC_INT_MOD
  #define EC_INT_MOD              GPIOINT       // Default set GPIOInt mode
#endif

#ifndef GPIOINT
  #define GPIOINT                 0x00
#endif

#ifndef INTERRUPT
  #define INTERRUPT               0x01
#endif

//-----------------------
//  EC I2C SensorHub Solution
//-----------------------
#ifdef PROJECT_EC_I2C_SHUB_PATH

Scope(PROJECT_EC_I2C_SHUB_PATH)
{
  Device (ECSH)
  {
      Name (_ADR, 0)
      Name (_HID, EC_SENSORHUB_HID)
      Name (_CID, "PNP0C50")
      Name (_UID, 1)
      
      Name (SBFB, ResourceTemplate (){
        I2cSerialBus (
          EC_SENSORHUB_ADDR,     //SlaveAddress: bus address
          ControllerInitiated,   //SlaveMode: Default to ControllerInitiated
          400000,                //ConnectionSpeed: in Hz
          AddressingMode7Bit,    //Addressing Mode: default to 7 bit
          EC_I2C_CONTR_NAME,     //ResourceSource: I2C bus controller name
          ,                      //ResourceSourceIndex: defaults to 0
          ,                      //ResourceUsage: Defaults to ResourceConsumer
          ,                      //Descriptor Name: creates name for offset of resource descriptor
        )  //VendorData
      })
 
      Name (SBFI, ResourceTemplate () {
        Interrupt(ResourceConsumer, Level, EC_INT_LEVEL, Exclusive,,,INTI) {EC_INTERRUPT}
      })
      
      Name(SBFG, ResourceTemplate () {
        GpioInt (Level, EC_INT_LEVEL, Exclusive, PullDefault, 0x0000, EC_GPIO_ResourceSource, 0x00) {EC_GPIO_INT}
      })

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
            Return(EC_IHV_ADDR)
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
        if(LLess(OSYS,2012)) { Return (SBFI) }  // For Windows 7 only report Interrupt; it doesn't support ACPI5.0 and wouldn't understand GpioInt nor I2cBus
        if(LEqual(EC_INT_MOD, GPIOINT)) { Return (ConcatenateResTemplate(SBFB, SBFG)) }
        Return (ConcatenateResTemplate(SBFB, SBFI))
      }
    } // End Device (ECSH)
}
#endif
