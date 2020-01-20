/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

//[-start-170410-IB07400857-add]//
#ifdef BUILD_TIME_CHECK_UNKNOWN_GPIO

#define N_HOSTSW_1            0x00C50084

#endif
//[-end-170410-IB07400857-add]//

scope (\_SB.PCI0) {
  //
  // LPIO1 PWM
  //
  Device(PWM) {
    Name (_ADR, 0x001A0000)
    Name (_DDN, "Intel(R) PWM Controller")
    Name (_UID, 1)

    Name (RBUF, ResourceTemplate ()
    {
    })
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  }

  //
  // LPIO1 HS-UART #1
  //
  Device(URT1) {
    Name (_ADR, 0x00180000)
    Name (_DDN, "Intel(R) HS-UART Controller #1")
    Name (_UID, 1)

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x2, 0x2, Width32Bit, )
      //FixedDMA(0x3, 0x3, Width32Bit, )
    })
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }

//[-start-160506-IB07400723-modify]//
//#if BXTI_PF_ENABLE
  Device (VUT0) {
    Name (_HID, "INT3511")
    Method (_STA, 0x0, NotSerialized)
    {
//[-start-160510-IB07400726-add]//
      If(LEqual(HLPS, 0))
      {
        Return(0x0)
      }
//[-end-160510-IB07400726-add]//
      If(LEqual(OSYS,2015)) {
        Return(0xf)
      }
      else {
        Return(0)
      }
    }
    Method(_CRS, 0x0, NotSerialized){
    Name(SBUF, ResourceTemplate (){
    UARTSerialBus(115200,,,0xfc,,,,32,32,"\\_SB.PCI0.URT1" )
    })
    Return (SBUF)
    }
  } //Device (VUT0)
//#endif
//[-end-160506-IB07400723-modify]//
  } //  Device (URT1)

  //
  // LPIO1 HS-UART #2
  //
  Device(URT2) {
    Name (_ADR, 0x00180001)
    Name (_DDN, "Intel(R) HS-UART Controller #2")
    Name (_UID, 2)

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x4, 0x4, Width32Bit, )
      //FixedDMA(0x5, 0x5, Width32Bit, )
    })
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }

    OperationRegion (KEYS, SystemMemory, U21A, 0x100)
    Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
    {
      Offset (0x84),
      PSAT,   32
    }

//[-start-160506-IB07400723-modify]//
//#if BXTI_PF_ENABLE
  Device (VUT1) {
    Name (_HID, "INT3512")
    Method (_STA, 0x0, NotSerialized)
    {
//[-start-160510-IB07400726-add]//
      If(LEqual(HLPS, 0))
      {
        Return(0x0)
      }
//[-end-160510-IB07400726-add]//
      If(LEqual(OSYS,2015)) {
        Return(0xf)
      }
      else {
        Return(0)
      }
    }
    Method(_CRS, 0x0, NotSerialized){
    Name(SBUF, ResourceTemplate (){
    UARTSerialBus(115200,,,0xfc,,,,32,32,"\\_SB.PCI0.URT2" )
    })
    Return (SBUF)
    }
  } // Device(VUT1)
//#endif
//[-end-160506-IB07400723-modify]//
  } // Device (URT2)

  //
  // LPIO1 HS-UART #3
  //
  Device(URT3) {
    Name (_ADR, 0x00180002)
    Name (_DDN, "Intel(R) HS-UART Controller #3")
    Name (_UID, 3)

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x4, 0x4, Width32Bit, )
      //FixedDMA(0x5, 0x5, Width32Bit, )
    })

//[-start-170410-IB07400857-modify]//
//#if BXTI_PF_ENABLE
    Name (SBUF, ResourceTemplate ()
    {
      GpioInt (Edge, ActiveLow, ExclusiveAndWake, PullNone, 0x0000, "\\_SB.GPO0", 0x00, ResourceConsumer, , ) {46}
	    GpioInt (Edge, ActiveLow, ExclusiveAndWake, PullNone, 0x0000, "\\_SB.GPO0", 0x00, ResourceConsumer, , ) {49}
    })

    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package () {"rx-gpios", Package() {^URT3, 0, 0, 0}},
		    Package () {"cts-gpios", Package() {^URT3, 1, 0, 0}},
      }
    })
//#endif
//[-end-170410-IB07400857-modify]//

    Method (_CRS, 0x0, NotSerialized)
    {
//[-start-170410-IB07400857-modify]//
//#if BXTI_PF_ENABLE
      If (LEqual(IOTP(),0x01)) { // IOTG CRB
        If(LEqual(\OSSL,3))
        {
          Store(\_SB.GPC0(N_HOSTSW_1), Local1)
          Or(Local1, 0x24000, Local1) // set GPIO_46 and GPIO_49 as GPIO Driver mode
  		    \_SB.SPC0(N_HOSTSW_1, Local1)
          Return (SBUF)
        } Else {
          Return (RBUF)
        }
      } Else {
        Return (RBUF)
      }
//#else
//      Return (RBUF)
//#endif
//[-end-170410-IB07400857-modify]//
    }
//[-start-170308-IB07400847-add]//
#ifdef H2ODDT_HSUART_SUPPORT
    //
    // H2ODDT via HS-UART, hide this debug port for OS
    //
    Method (_STA, 0x0, NotSerialized)
    {
      Return (0x1)
    }
#endif
//[-end-170308-IB07400847-add]//
//[-start-170719-IB07400890-add]//
    Device (VUT2) {
      Name (_HID, "INT3513")
      Method (_STA, 0x0, NotSerialized)
      {
        If(LEqual(HLPS, 0))
        {
          Return(0x0)
        }
        If(LEqual(OSYS,2015)) {
          Return(0xf)
        }
        else {
          Return(0)
        }
      }
      Method(_CRS, 0x0, NotSerialized){
      Name(SBUF, ResourceTemplate (){
      UARTSerialBus(115200,,,0xfc,,,,32,32,"\\_SB.PCI0.URT3" )
      })
      Return (SBUF)
      }
    } // Device(VUT2)
//[-end-170719-IB07400890-add]//
  } // Device (URT3)

  //
  // LPIO1 HS-UART #4
  //
  Device(URT4) {
    Name (_ADR, 0x00180003)
    Name (_DDN, "Intel(R) HS-UART Controller #4")
    Name (_UID, 4)

    Name (RBUF, ResourceTemplate ()
    {
    })
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
//[-start-170719-IB07400890-add]//
    Device (VUT3) {
      Name (_HID, "INT3514")
      Method (_STA, 0x0, NotSerialized)
      {
        If(LEqual(HLPS, 0))
        {
          Return(0x0)
        }
        If(LEqual(OSYS,2015)) {
          Return(0xf)
        }
        else {
          Return(0)
        }
      }
      Method(_CRS, 0x0, NotSerialized){
      Name(SBUF, ResourceTemplate (){
      UARTSerialBus(115200,,,0xfc,,,,32,32,"\\_SB.PCI0.URT4" )
      })
      Return (SBUF)
      }
    } // Device(VUT3)
//[-end-170719-IB07400890-add]//
  } // Device (URT4)

  //
  // LPIO1 SPI
  //
  Device(SPI1) {
    Name (_ADR, 0x00190000)
    Name (_DDN, "Intel(R) SPI Controller #1")
    Name (_UID, 1)

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x0, 0x0, Width32Bit, )
      //FixedDMA(0x1, 0x1, Width32Bit, )
    })
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device (SPI1)


  //
  // LPIO1 SPI #2
  //
  Device(SPI2) {
    Name (_ADR, 0x00190001)
    Name (_DDN, "Intel(R) SPI Controller #2")
    Name (_UID, 2)

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x6, 0x6, Width32Bit, )
      //FixedDMA(0x7, 0x7, Width32Bit, )
    })
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
   } // Device (SPI2)

  //
  // LPIO1 SPI #3
  //
  Device(SPI3) {
    Name (_ADR, 0x00190002)
    Name (_DDN, "Intel(R) SPI Controller #3")
    Name (_UID, 3)

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x8, 0x8, Width32Bit, )
      //FixedDMA(0x9, 0x9, Width32Bit, )
    })
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device (SPI3)


  //
  // LPIO2 I2C #0
  // Connect to AUDIO CODEC, MCSI CAM
  //
  Device(I2C0) {
    Name (_ADR, 0x00160000)
    Name (_DDN, "Intel(R) I2C Controller #0")
    Name (_UID, 1)
    Name (LINK,"\\_SB.PCI0.I2C0") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x10, 0x0, Width32Bit, )
      //FixedDMA(0x11, 0x1, Width32Bit, )
    })
//[-start-160803-IB07400768-modify]//
//  #if BXTI_PF_ENABLE
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"clock-frequency", IC0S}
      }
    })
    Method (FMCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 100, 214, 28 })
      Return (PKG)
    }        
    Method (FPCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 38, 80, 12 })
      Return (PKG)
    }    
    Method (HSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 5, 24, 12 })
      Return (PKG)
    }      
    Method (SSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 580,730, 28})
      Return (PKG)
    }
//  #endif
//[-end-160803-IB07400768-modify]//
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device(I2C0)

  //
  // LPIO2 I2C #1
  // Connect to NFC
  //
  Device(I2C1) {
    Name (_ADR, 0x00160001)
    Name (_DDN, "Intel(R) I2C Controller #1")
    Name (_UID, 2)
    Name (LINK,"\\_SB.PCI0.I2C1") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x12, 0x2, Width32Bit, )
      //FixedDMA(0x13, 0x3, Width32Bit, )
    })
//[-start-160803-IB07400768-modify]//
//  #if BXTI_PF_ENABLE
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"clock-frequency", IC1S}
      }
    })
    Method (FMCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 100, 214, 28 })
      Return (PKG)
    }        
    Method (FPCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 38, 80, 12 })
      Return (PKG)
    }    
    Method (HSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 5, 24, 12 })
      Return (PKG)
    }       
    Method (SSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 580,730, 28})
      Return (PKG)
    }      
//  #endif
//[-end-160803-IB07400768-modify]//
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device (I2C1)

  //
  // LPIO2 I2C #2
  // Connect to MCSI CAMERA
  //
  Device(I2C2) {
    Name (_ADR, 0x00160002)
    Name (_DDN, "Intel(R) I2C Controller #2")
    Name (_UID, 3)
    Name (LINK,"\\_SB.PCI0.I2C2") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x14, 0x4, Width32Bit, )
      //FixedDMA(0x15, 0x5, Width32Bit, )
    })
//[-start-160803-IB07400768-modify]//
//  #if BXTI_PF_ENABLE
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"clock-frequency", IC2S}
      }
    })
    Method (FMCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 100, 214, 28 })
      Return (PKG)
    }        
    Method (FPCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 38, 80, 12 })
      Return (PKG)
    }   
    Method (HSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 5, 24, 12 })
      Return (PKG)
    }      
    Method (SSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 580,730, 28})
      Return (PKG)
    }
//  #endif
//[-end-160803-IB07400768-modify]//
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device(I2C2)

  //
  // LPIO2 I2C #3
  // Connect to TOUCH PANEL
  //
  Device(I2C3) {
    Name (_ADR, 0x00160003)
    Name (_DDN, "Intel(R) I2C Controller #3")
    Name (_UID, 4)
    Name (LINK,"\\_SB.PCI0.I2C3") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x16, 0x6, Width32Bit, )
      //FixedDMA(0x17, 0x7, Width32Bit, )
    })
//[-start-160803-IB07400768-modify]//
//  #if BXTI_PF_ENABLE
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"clock-frequency", IC3S}
      }
    })
    
    Method (FMCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 100, 214, 28 })
      Return (PKG)
    }       
    Method (FPCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 38, 80, 12 })
      Return (PKG)
    } 
    Method (HSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 5, 24, 12 })
      Return (PKG)
    }       
    Method (SSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 580,730, 28})
      Return (PKG)
    }
//  #endif
//[-end-160803-IB07400768-modify]//
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  }  // Device(I2C3)

  //
  // LPIO2 I2C #4
  // Connect to TOUCH PAD
  //
  Device(I2C4) {
    Name (_ADR, 0x00170000)
    Name (_DDN, "Intel(R) I2C Controller #4")
    Name (_UID, 1)
    Name (LINK,"\\_SB.PCI0.I2C4") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x18, 0x0, Width32Bit, )
      //FixedDMA(0x19, 0x1, Width32Bit, )
    })
//[-start-160803-IB07400768-modify]//
//  #if BXTI_PF_ENABLE
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"clock-frequency", IC4S}
      }
    })
    
    Method (FMCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 100, 214, 28 })
      Return (PKG)
    }        
    Method (FPCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 38, 80, 12 })
      Return (PKG)
    }    
    Method (HSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 5, 24, 12 })
      Return (PKG)
    }       
    Method (SSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 580,730, 28})
      Return (PKG)
    }
//  #endif
//[-end-160803-IB07400768-modify]//
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device(I2C4)


  //
  // LPIO2 I2C #5
  // Connect to  SENSOR AIC
  //
  Device(I2C5) {
	Name (_ADR, 0x00170001)
    Name (_DDN, "Intel(R) I2C Controller #5")
    Name (_UID, 2)
    Name (LINK,"\\_SB.PCI0.I2C5") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x1A, 0x02, Width32Bit, )
      //FixedDMA(0x1B, 0x03, Width32Bit, )
    })
//[-start-160803-IB07400768-modify]//
//  #if BXTI_PF_ENABLE
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"clock-frequency", IC5S}
      }
    })
    Method (FMCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 100, 214, 28 })
      Return (PKG)
    }       
    Method (FPCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 38, 80, 12 })
      Return (PKG)
    }    
    Method (HSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 5, 24, 12 })
      Return (PKG)
    }       
    Method (SSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 580,730, 28})
      Return (PKG)
    }
//  #endif
//[-end-160803-IB07400768-modify]//
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device(I2C5)


  //
  // LPIO2 I2C #6
  // Connect to SENSOR AIC
  //
  Device(I2C6) {
	Name (_ADR, 0x00170002)
    Name (_DDN, "Intel(R) I2C Controller #6")
    Name (_UID, 3)
    Name (LINK,"\\_SB.PCI0.I2C6") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x1C, 0x4, Width32Bit, )
      //FixedDMA(0x1D, 0x5, Width32Bit, )
    })
//[-start-160803-IB07400768-modify]//
//  #if BXTI_PF_ENABLE
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"clock-frequency", IC6S}
      }
    })
    Method (FMCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 100, 214, 28 })
      Return (PKG)
    }        
    Method (FPCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 38, 80, 12 })
      Return (PKG)
    }    
    Method (HSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 5, 24, 12 })
      Return (PKG)
    }       
    Method (SSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 580,730, 28})
      Return (PKG)
    }
//  #endif
//[-end-160803-IB07400768-modify]//
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device(I2C6)

  //
  // LPIO2 I2C #7
  // Connect to PSS RFID / INA DEVICESx16
  //
  Device(I2C7) {
	Name (_ADR, 0x00170003)
    Name (_DDN, "Intel(R) I2C Controller #7")
    Name (_UID, 4)
    Name (LINK,"\\_SB.PCI0.I2C7") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      //FixedDMA(0x1C, 0x4, Width32Bit, )
      //FixedDMA(0x1D, 0x5, Width32Bit, )
    })
//[-start-160803-IB07400768-modify]//
//  #if BXTI_PF_ENABLE
    Name (_DSD, Package () {
      ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
      Package () {
        Package (2) {"clock-frequency", IC7S}
      }
    })
    Method (FMCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 100, 214, 28 })
      Return (PKG)
    }        
    Method (FPCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 38, 80, 12 })
      Return (PKG)
    }   
    Method (HSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 5, 24, 12 })
      Return (PKG)
    }      
    Method (SSCN, 0x0, Serialized)
    {
      Name (PKG, Package(3) { 580,730, 28})
      Return (PKG)
    }
//  #endif
//[-end-160803-IB07400768-modify]//
    Method (_CRS, 0x0, NotSerialized)
    {
      Return (RBUF)
    }
  } // Device(I2C7)
}// PCI0

