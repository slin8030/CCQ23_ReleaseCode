/*
 * (C) Copyright 2013-2014 Compal Electronics, Inc.
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
*/

//
// [COMMON ASL] Using EDK II
//

        Device (INSH)
        {
          Name (_HID, INVN_SENSORS_HID)

           Method (_STA, 0, NotSerialized)
           {
               Return (0x0F)
           }
           Method (_CRS, 0, NotSerialized)
           {
              Name (SBUF, ResourceTemplate ()
                {
                    I2CSerialBus ( GYRO_ADDR,ControllerInitiated, I2C_SPEED, AddressingMode7Bit, "\\_SB.PCI0.I2C0",,,,)
                    I2CSerialBus ( COMPASS_ADDR,ControllerInitiated, I2C_SPEED, AddressingMode7Bit, "\\_SB.PCI0.I2C0",,,,)
                    Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive, , ,) {35} //Route to PIRQT(GPIO51)
                })
              Return (SBUF)

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

       // Device CM3218
            Device(CMSR)
            {
                Name(_ADR, 0x0)
                Name(_HID, "CPLM3218")
                Name(_CID, "CPLM3218")
                Name(_UID, 0x1)
                Method(_STA, 0x0, NotSerialized)
                {
			        return(0x0f)
                }
                Method(_CRS, 0x0, NotSerialized)
                {
                    Name(SBUF,ResourceTemplate () {
                        I2CSerialBus(0x10,          //SlaveAddress: bus address
                            ,                       //SlaveMode: default to ControllerInitiated
                            400000,                 //ConnectionSpeed: in Hz
                            ,						//Addressing Mode: default to 7 bit
                            "\\_SB.PCI0.I2C0",      //ResourceSource: I2C bus controller name
                            ,                       //Descriptor Name: creates name for offset of resource descriptor
                            )  //VendorData
                    Interrupt(ResourceConsumer, Level, ActiveLow, Exclusive, , ,) {28} //Route to PIRQM
                    })
                    Return (SBUF)
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
					Package() {70, 0},     // Min          ( -30% adjust at    0 lux)
					Package() {73, 10},    //              ( -27% adjust at   10 lux)
					Package() {85, 80},    //              ( -15% adjust at   80 lux)
					Package() {100,300},   // Baseline     (   0% adjust at  300 lux)
					Package() {150,1000}   // Max          ( +50% adjust at 1000 lux)
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
        
