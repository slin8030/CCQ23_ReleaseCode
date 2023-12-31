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
 1.00                Vanessa      Init version.
 1.01      16/04/13  Vaneesa      Add more device definition at oemasl.asl for project design to changed.
 1.02      16/09/26  Vaneesa      Updated device path definition, and change device support rule.
 
*/

// [COMMON ASL] AC using EDK II

#ifndef ALS_SENSORS_HID
  #define ALS_SENSORS_HID         "CPLM3218"
#endif

#ifndef ALS_INT_MOD
  #define ALS_INT_MOD             GPIOINT       // Default set GPIOInt mode
#endif

#ifndef GPIOINT
  #define GPIOINT                 0x00
#endif

#ifndef INTERRUPT
  #define INTERRUPT               0x01
#endif

//------------------------------
//  Capella 3218 ALS Solution
//------------------------------
Scope(CM3218_I2C_PATH)
{
        // Device CM3218
        Device(CMSR)
        {
          Name(_ADR, 0x0)
          Name(_HID, ALS_SENSORS_HID)
          Name(_CID, ALS_SENSORS_HID)
          Name(_UID, 0x1)

          Name(SBFB,ResourceTemplate () {
            I2CSerialBus(
              ALS_ADDR,                         //SlaveAddress: bus address
              ,                                 //SlaveMode: default to ControllerInitiated
              400000,                           //ConnectionSpeed: in Hz
              ,						            //Addressing Mode: default to 7 bit
              ALS_I2C_CONTR_NAME,               //ResourceSource: I2C bus controller name
              ,                                 //Descriptor Name: creates name for offset of resource descriptor
            )                                   //VendorData
          })
          Name(SBFI, ResourceTemplate () {
            Interrupt(ResourceConsumer, Level, ALS_INT_LEVEL, ExclusiveAndWake,,,) {ALS_INTERRUPT}
          })

          Name(SBFG, ResourceTemplate () {
            GpioInt (Level, ALS_INT_LEVEL, Exclusive, PullDefault, 0x0000, ALS_GPIO_ResourceSource, 0x0) {ALS_GPIO_INT} 
          })
  
          Method(_STA, 0x0, NotSerialized)
          {
		    return(0x0F)
          }
            
          Method(_CRS, 0x0, NotSerialized)
          {
              if(LLess(OSYS,2012)) { Return (SBFI) }  // For Windows 7 only report Interrupt; it doesn't support ACPI5.0 and wouldn't understand GpioInt nor I2cBus
              if(LEqual(ALS_INT_MOD, GPIOINT)) { Return (ConcatenateResTemplate(SBFB, SBFG)) }
              Return (ConcatenateResTemplate(SBFB, SBFI))
          }				
		  // Initial setting
		  Name(CPM0, Package(0x7)
		  {
			32181,0x1,0xF,0x0840,0x013B,0x011D,0x0
		  })

		  // Calibration data
		  Name(CPM1, Package(0x3)
		  {
			1428,2000000,100000
		  })

				// Sensitivity factor
	      Name(CPM2, Package(0x6)
		  {
			25000,100000,100000,100000,200000,1600000
		  })					
				
		  // ALR curve
		  Name(CPM3, Package()
		  {
			Package() {70, 0},                  // Min          ( -30% adjust at    0 lux)
			Package() {73, 10},                 //              ( -27% adjust at   10 lux)
			Package() {85, 80},                 //              ( -15% adjust at   80 lux)
			Package() {100,300},                // Baseline     (   0% adjust at  300 lux)
			Package() {150,1000}                // Max          ( +50% adjust at 1000 lux)
		  })		
				
		  // Default property
		  Name(CPM5, Package(0x7)
		  {
			0,150000,400,1000,2,50,5
		  })					
				
		  Method(_DSM, 0x4, NotSerialized)
		  {
			If(LEqual(Arg0, Buffer(0x10)
			{
			  0xB6, 0xC6, 0x03, 0x07, 0xCA, 0x1C, 0x44, 0x41, 0x9F, 0xE7, 0x46, 0x54,	0xF5, 0x3A, 0x0B, 0xD9
			}))
			
			{
			  If(LEqual(Arg2, 0x0))
			  {
				Return(Buffer(0x1)
			    {
				  0x03
				})
			  }
			  If(LEqual(Arg2, 0x1))
			  {
				If(LEqual(ToInteger(Arg3, ), 0x0))
				{												
				    Return(CPM0)
				}
				If(LEqual(ToInteger(Arg3, ), 0x1))
				{
					Return(CPM1)
				}
				If(LEqual(ToInteger(Arg3, ), 0x2))
				{
					Return(CPM2)
				}
				If(LEqual(ToInteger(Arg3, ), 0x3))
				{
					Return(CPM3)
				}
				If(LEqual(ToInteger(Arg3, ), 0x5))
				{
					Return(CPM5)
				}							
		      }						
            }
			Return(Buffer(One)
            {         
			    0x0
            })
          }
        }// Device CM3218
}
