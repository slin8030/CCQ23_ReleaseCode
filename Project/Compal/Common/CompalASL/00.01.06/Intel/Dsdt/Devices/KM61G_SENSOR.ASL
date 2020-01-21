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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.00                Vanessa      Init version.
 1.01      16/04/13  Vaneesa      Add more device definition at oemasl.asl for project design to changed.
 1.02      16/09/26  Vaneesa      Updated device path definition, and change device support rule.
*/

// [COMMON ASL] AC using EDK II

#ifndef KM61G_SENSORS_HID
  #define KM61G_SENSORS_HID       "KIOX0017"
#endif

#ifndef KM61G_INT_MOD
  #define KM61G_INT_MOD           GPIOINT       // Default set GPIOInt mode
#endif

#ifndef GPIOINT
  #define GPIOINT                 0x00
#endif

#ifndef INTERRUPT
  #define INTERRUPT               0x01
#endif

//-------------------------------
//  Kionix KM61G Sensor Solution
//-------------------------------
Scope(KM61G_I2C_PATH)
{
        Device(KMX0)
        {
          Name(_ADR, Zero)
          Name(_HID, KM61G_SENSORS_HID)
          Name(_CID, KM61G_SENSORS_HID)
          Name(_UID, One)

          Name (SBFI, ResourceTemplate () {
            Interrupt(ResourceConsumer, Level, KM61G_GYRO_INT_LEVEL, Exclusive,,,INTI) {KM61G_GYRO_INTERRUPT}
          })
      
          Name(SBFG, ResourceTemplate () {
            GpioInt (Level, KM61G_GYRO_INT_LEVEL, Exclusive, PullDefault, 0x0000, KM61G_GPIO_ResourceSource, 0x00) {KM61G_GYRO_GPIO_INT} 
            GpioInt (Level, KM61G_ACC_INT_LEVEL, Exclusive, PullDefault, 0x0000, KM61G_GPIO_ResourceSource, 0x00) {KM61G_ACC_GPIO_INT} 
          })

          Name(SBFB, ResourceTemplate()
            {
              I2cSerialBus (
              KM61G_ADDR,                       //SlaveAddress: bus address
              ControllerInitiated,              //SlaveMode: Default to ControllerInitiated
              400000,                           //ConnectionSpeed: in Hz
              AddressingMode7Bit,               //Addressing Mode: default to 7 bit
              KM61G_I2C_CONTR_NAME,             //ResourceSource: I2C bus controller name
               0,                               //ResourceSourceIndex: defaults to 0
              ResourceConsumer,                 //ResourceUsage: Defaults to ResourceConsumer
              ,                                 //Descriptor Name: creates name for offset of resource descriptor
              )                                 //VendorData
            })
            
          Method(_STA, 0x0, NotSerialized)
          {
            Return(0xf)
          }
          Method(_CRS, 0x0, NotSerialized)
          {
            if(LLess(OSYS,2012)) { Return (SBFI) }  // For Windows 7 only report Interrupt; it doesn't support ACPI5.0 and wouldn't understand GpioInt nor I2cBus
            if(LEqual(KM61G_INT_MOD, GPIOINT)) { Return (ConcatenateResTemplate(SBFB, SBFG)) }
            Return (ConcatenateResTemplate(SBFB, SBFI))
          }

          Method(_DSM, 0x4, NotSerialized)
          {
            Name(TBUF,Buffer(52){})
//          Store(KNIC,TBUF) //INVC from the NVRAM for calibration data

            If(LEqual(Arg0, ToUUID("9E59A725-55AF-432E-A70A-8C4D2CCDA4B2"))){
                Return(TBUF)
           }
          }
        }
}
