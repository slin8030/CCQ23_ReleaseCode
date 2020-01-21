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

*/

// [COMMON ASL] AC using EDK II
//

#ifndef MTK_NFC_HID
  #define MTK_NFC_HID             "MTK6605"
#endif

#ifndef MTK_INT_MOD
  #define MTK_INT_MOD             GPIOINT       // Default set GPIOInt mode
#endif

#ifndef GPIOINT
  #define GPIOINT                 0x00
#endif

#ifndef INTERRUPT
  #define INTERRUPT               0x01
#endif

//-------------------------------
//  MTK NFC 6605 Sensor Solution
//-------------------------------
#ifdef PROJECT_I2C_MTKNFC_PATH
Scope(PROJECT_I2C_MTKNFC_PATH)
{
        Device(MNFC)  //MTKNFC        
        {           
          Name (_ADR, 0)
          Name (_HID, MTK_NFC_HID)              //MTK6605 QCOM2497
          Name (_CID, MTK_NFC_HID)
          Name (_DDN, "MEDIATEK NFC Controller " )
          Name (_UID, 1)

          Name(SBFB,ResourceTemplate () {
            I2CSerialBus(
              MTK_NFC_ADDR,                     //SlaveAddress: bus address
              ,                                 //SlaveMode: default to ControllerInitiated
              400000,                           //ConnectionSpeed: in Hz
              ,                                 //Addressing Mode: default to 7 bit
              NFC_I2C_CONTR_NAME,               //ResourceSource: I2C bus controller name
               ,                                //Descriptor Name: creates name for offset of resource descriptor
              )                                 //VendorData                   
              //NFC_FW_RESET
              GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, MTK_GPIO_ResourceSource, ) {MTK_NFC_RST_PIN_NUM} 
              //NFC_ENABLE
              GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, MTK_GPIO_ResourceSource, ) {MTK_NFC_EN_PIN_NUM} 
            })
            
          Name(SBFI, ResourceTemplate () {
            Interrupt(ResourceConsumer, Level, MTK_NFC_INT_LEVEL, ExclusiveAndWake,,,) {MTK_INTERRUPT}
          })

          Name(SBFG, ResourceTemplate () {
            // NFC_INT 
            GpioInt (Level, MTK_NFC_INT_LEVEL, Exclusive, PullDefault, 0x0000, MTK_GPIO_ResourceSource, 0x0) {MTK_NFC_GPIO_INT} 
          })

          Method (_STA, 0x0, NotSerialized)
          {
            Return(0xF)
          }
          Method(_CRS, 0x0, NotSerialized)
          {
              if(LLess(OSYS,2012)) { Return (SBFI) }  // For Windows 7 only report Interrupt; it doesn't support ACPI5.0 and wouldn't understand GpioInt nor I2cBus
              if(LEqual(MTK_INT_MOD, GPIOINT)) { Return (ConcatenateResTemplate(SBFB, SBFG)) }
              Return (ConcatenateResTemplate(SBFB, SBFI))
          }
        }// MNFC 
}
#endif 
