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
//
#ifndef INVN_SENSORS_HID
  #define INVN_SENSORS_HID        "INVN6500"
#endif

#ifndef GYRO_DEVICE_NAME
  #define GYRO_DEVICE_NAME        "MPU6500"
#endif

#ifndef COMPASS_DEVICE_NAME
  #define COMPASS_DEVICE_NAME     "AK8963"
#endif

#ifndef INVN_INT_MOD
  #define INVN_INT_MOD            GPIOINT       // Default set GPIOInt mode
#endif

#ifndef GPIOINT
  #define GPIOINT                 0x00
#endif

#ifndef INTERRUPT
  #define INTERRUPT               0x01
#endif

//-----------------------
//  INVN Sensor Solution
//-----------------------
Scope(INVN_I2C_PATH)
{
        Device (INSH)
        {
          Name (_HID, INVN_SENSORS_HID)
            
          Name (SBFI, ResourceTemplate () {
            Interrupt(ResourceConsumer, Level, INVN_INT_LEVEL, Exclusive,,,INTI) {INVN_INTERRUPT}
          })
      
          Name(SBFG, ResourceTemplate () {
            GpioInt (Level, INVN_INT_LEVEL, Exclusive, PullDefault, 0x0000, INVN_GPIO_ResourceSource, 0x0) {INVN_GPIO_INT} 
          })

          Name (SBFB, ResourceTemplate (){
            I2CSerialBus(
              GYRO_ADDR,                        //SlaveAddress: bus address
              ,                                 //SlaveMode: default to ControllerInitiated
              400000,                           //ConnectionSpeed: in Hz
              ,						            //Addressing Mode: default to 7 bit
              INVN_I2C_CONTR_NAME,              //ResourceSource: I2C bus controller name
              ,                                 //Descriptor Name: creates name for offset of resource descriptor
              )                                 //VendorData
            I2CSerialBus(
              COMPASS_ADDR,                     //SlaveAddress: bus address
              ,                                 //SlaveMode: default to ControllerInitiated
              400000,                           //ConnectionSpeed: in Hz
              ,						            //Addressing Mode: default to 7 bit
              INVN_I2C_CONTR_NAME,              //ResourceSource: I2C bus controller name
              ,                                 //Descriptor Name: creates name for offset of resource descriptor
              )                                 //VendorData
            })

          Method (_STA, 0, NotSerialized)
          {
            Return (0x0F)
          }
          Method (_CRS, 0, NotSerialized)
          {
            if(LLess(OSYS,2012)) { Return (SBFI) }  // For Windows 7 only report Interrupt; it doesn't support ACPI5.0 and wouldn't understand GpioInt nor I2cBus
            if(LEqual(INVN_INT_MOD, GPIOINT)) { Return (ConcatenateResTemplate(SBFB, SBFG)) }
            Return (ConcatenateResTemplate(SBFB, SBFI))

          }
          //[COM]++Compass calibration try to get it from variable and copy to OEMNVS.
          Name(CNF0,Package()
          {
            //Name, Slave Address, Bus, Position
            //Bus: 1 - primary; 2 - secondary (MPU I2C bus)
            //Position - Mounting Matrix mapping
            Package() {GYRO_DEVICE_NAME, GYRO_ADDR, 1, 0x00},        // Gyro
            Package() {GYRO_DEVICE_NAME, GYRO_ADDR, 1, 0x00},        // Accel
            Package() {COMPASS_DEVICE_NAME, COMPASS_ADDR, 0x02, 0x01},  // Compass
            Package() {
            Buffer (92) {
                0x5A, 0xA5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00}                              //92 bytes 
            }
          })
          Method(CNFG)
          {
            Return (CNF0)
          }  
        }  // Device (INSH)
}
