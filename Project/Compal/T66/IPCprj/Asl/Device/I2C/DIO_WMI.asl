/*
 * (C) Copyright 2016 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2016 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "T66WMI_Def.asl"

//
// For DIO (TCA955 driver)
//
#define  DIO_CONTROLLER    \_SB.PCI0.I2C2

#define  WMI_DIO_AMOUNT    16
#define  DIO_BIT_MASK      0xFFFF

#define  IN_REG_OFFSET          0x00
#define  OUT_REG_OFFSET         0x02
#define  CONFIG_REG_OFFSET      0x06
#define  RW_SIZE                0x08   // Read/Write size 8bit = 1Byte

  Scope(DIO_CONTROLLER) {
    Name(PINB, DIO_BIT_MASK)   // Pin bit
    Name(PIND, DIO_BIT_MASK)   // Pin Bit Data
    Name(ERRO, 0)              // WMI Error level

    Name(DIOS,ResourceTemplate () {
      I2CSerialBus(0x20, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.PCI0.I2C2", , )
    })

    OperationRegion(EDIO, GenericSerialBus, Zero, 0x100)
    Field(EDIO, BufferAcc, NoLock, Preserve)
    {
      Connection(DIOS),
      AccessAs(BufferAcc, AttribByte),
      INP0, 8,      // Input regsiter0 :DIO_GP0~7
      INP1, 8,      // Input regsiter1 :DIO_GP8~15
      OTP0, 8,      // Output regsiter0 :DIO_GP0~7
      OTP1, 8,      // Output regsiter1 :DIO_GP8~15
      INV0, 8,      // Polarity inversion register regsiter0 :DIO_GP0~7
      INV1, 8,      // Polarity inversion register regsiter1 :DIO_GP8~15
      CFP0, 8,      // Config regsiter0 :DIO_GP0~7
      CFP1, 8,      // Config regsiter1 :DIO_GP8~15
    }

    Name(BUFF, Buffer(4){})
    CreateByteField(BUFF, 0x00, STAT) // STAT = Status (Byte)
    CreateByteField(BUFF, 0x02, DATA) // DATA = Data (Byte)

    Method(RDRG, 0x02, NotSerialized) {
    // ***************************************************************************
    // Method:
    //   RDRG
    //
    // Description:
    //   Read Register
    //
    // Entry:
    //   Arg0 : Register offset
    //   Arg1 : Length
    //
    // Return:
    //    Read data
    // ***************************************************************************
      Store (WMI_SUCCESS, ERRO)
      if (LLessEqual(Arg1,0)) {
        Store(0x0,\STDT)  //CMFC False
        Store (WMI_INVALID_PARAMETER, ERRO)
        return (ERRO)
      }

      Store (0, Local0)     // return data buffer
      Store (0, Local1)     // Read Data shift bits
      while(LGreater(Arg1,0)) {
        switch(Arg0) {
          case (0) {
            Store (DIO_CONTROLLER.INP0, BUFF)
          }
          case (1) {
            Store (DIO_CONTROLLER.INP1, BUFF)
          }
          case (2) {
            Store (DIO_CONTROLLER.OTP0, BUFF)
          }
          case (3) {
            Store (DIO_CONTROLLER.OTP1, BUFF)
          }
          case (4) {
            Store (DIO_CONTROLLER.INV0, BUFF)
          }
          case (5) {
            Store (DIO_CONTROLLER.INV1, BUFF)
          }
          case (6) {
            Store (DIO_CONTROLLER.CFP0, BUFF)
          }
          case (7) {
            Store (DIO_CONTROLLER.CFP1, BUFF)
          }
          Default {
            Store(0x0,\STDT)  //CMFC False
            Store (WMI_INVALID_PARAMETER, ERRO)
            return (ERRO)
          }
        }
        If(LNotEqual(STAT, 0x00)){
          Store(0x0,\STDT)  //CMFC False
          Store (WMI_DEVICE_ERROR, ERRO)
          return (ERRO)
        }
        Or (ShiftLeft (DATA, Multiply (Local1, RW_SIZE)), Local0, Local0)
        Add (Local1, 1, Local1)
        Add (Arg0, Divide (RW_SIZE, 8), Arg0)
        Subtract (Arg1, Divide (RW_SIZE, 8), Arg1)
      }
      Store (WMI_SUCCESS, ERRO)
      Store(0x1,\STDT)  //CMFC Success
      Store (Local0, \BFDT)
      return (Local0)
    }

    Method(WTRG, 0x03,NotSerialized) {
    // ***************************************************************************
    // Method:
    //   RDRG
    //
    // Description:
    //   Read Register
    //
    // Entry:
    //   Arg0 : Register offset
    //   Arg1 : Length
    //   Arg2 : Data Buffer
    // ***************************************************************************
      Store (WMI_SUCCESS, ERRO)
      if (LLessEqual(Arg1,0)) {
        Store(0x0,\STDT)  //CMFC False
        Store (WMI_INVALID_PARAMETER, ERRO)
        return (ERRO)
      }

      Store (0, Local1)        // Write Data shift bits
      while(LGreater(Arg1,0)) {
        ShiftRight(Arg2, Multiply(Local1, RW_SIZE), DATA)
        switch(Arg0) {
          case (0) {
            Store (BUFF, DIO_CONTROLLER.INP0)
          }
          case (1) {
            Store (BUFF, DIO_CONTROLLER.INP1)
          }
          case (2) {
            Store (BUFF, DIO_CONTROLLER.OTP0)
          }
          case (3) {
            Store (BUFF, DIO_CONTROLLER.OTP1)
          }
          case (4) {
            Store (BUFF, DIO_CONTROLLER.INV0)
          }
          case (5) {
            Store (BUFF, DIO_CONTROLLER.INV1)
          }
          case (6) {
            Store (BUFF, DIO_CONTROLLER.CFP0)
          }
          case (7) {
            Store (BUFF, DIO_CONTROLLER.CFP1)
          }
          Default {
            Store(0x0,\STDT)  //CMFC False
            Store (WMI_INVALID_PARAMETER, ERRO)
            return (ERRO)
          }
        }
        Add (Local1, 1, Local1)
        Add (Arg0, Divide (RW_SIZE, 8), Arg0)
        Subtract (Arg1, Divide (RW_SIZE, 8), Arg1)
      }
      Store(0x1,\STDT)  //CMFC Success
      Store (WMI_SUCCESS, ERRO)
      return (ERRO)
    }

    Method(RPIN, 0x2, NotSerialized) {
    // ***************************************************************************
    // Method:
    //   RPIN
    //
    // Description:
    //   Read DIO pin
    //
    // Entry:
    //   Arg0 : Type (Singal Pin:0 ; Multiple Pin: 1)
    //   Arg1 : Pin number (Single type);  Pin Bit mask (Multiple type)
    //
    // Return:
    //   Read Data,
    //   0 :  Error

    // ***************************************************************************
      //
      // Check input parameters are valid
      //
      if (LEqual (Arg0, SINGLE_PIN)) {
        if (LGreaterEqual(Arg0, WMI_DIO_AMOUNT)) {
          Store(0x0,\STDT)  //CMFC False
          return (WMI_INVALID_PARAMETER)
        }
        ShiftLeft(1, Arg1, PINB)
      } else {
        Store (Arg1, PINB)
      }

      //
      // Read input regsiter
      //
      Divide (WMI_DIO_AMOUNT, RW_SIZE, Local0, Local1)
      if (LNotEqual (Local0,0)) {
        Add(Local1,1,Local1)
      }
      Store(RDRG(IN_REG_OFFSET, Local1),Local0)
      if (WMI_ERROR(ERRO)) {
        Store(0x0,\STDT)  //CMFC False
        return (ERRO)
      }

      //
      // return target pin value
      //
      And (Local0, PINB, Local0)
      if (LEqual (Arg0, SINGLE_PIN)) {
        if (LEqual(Local0,0)) {
          Store(0x01, SFNO)
          Store(0x00,BFDT)
          return (0x00)
        } else {
          Store(0x01, SFNO)
          Store(0x01,BFDT)
          return (0x01)
        }
      } else {
          Store(0x01, SFNO)
          Store(0x01,BFDT)
          return  (Local0)
      }
    }

    Method(WPIN, 0x3, NotSerialized) {
    // ***************************************************************************
    // Method:
    //   WPIN
    //
    // Description:
    //   Write DIO pin
    //
    // Entry:
    //   Arg0 : Type (Singal Pin:0 ; Multiple Pin: 1)
    //   Arg1 : Pin number (Single type); Pin Bit mask (Multiple type: Bits are pin numbers , H/L are In/Output setting)
    //   Arg2 : Setting ; Not reference in multiple type
    //
    // Output:
    //   0x00 : Success
    //   WMI_ERROR
    // ***************************************************************************
      //
      // Check input parameters are valid
      //
      if (LEqual (Arg0, SINGLE_PIN)) {
        if (LGreaterEqual(Arg1, WMI_DIO_AMOUNT)) {
          Store(0x0,\STDT)  //CMFC False
          return (WMI_INVALID_PARAMETER)
        }
        ShiftLeft(1, Arg1,PINB)
        ShiftLeft(Arg2, Arg1, PIND)
      } else {
        Store (DIO_BIT_MASK, PINB)
        Store (Arg1, PIND)
      }

      And (DIO_BIT_MASK, PINB, PINB)
      if (LEqual (PINB,Zero)) {
        Store(0x0,\STDT)  //CMFC False
        return (WMI_INVALID_PARAMETER)
      }

      //
      // Read output regsiter
      //
            //
      Divide (WMI_DIO_AMOUNT, RW_SIZE, Local0, Local1)
      if (LNotEqual (Local0,0)) {
        Add(Local1,1,Local1)
      }
      Store(RDRG(OUT_REG_OFFSET, Local1), Local0)
      if (WMI_ERROR(ERRO)) {
        Store(0x0,\STDT)  //CMFC False
        return (ERRO)
      }
      //
      // update output data
      //
      And (Local0, Not(PINB), Local0)
      Or (Local0, PIND, Local0)

      return (WTRG (OUT_REG_OFFSET,Local1 ,Local0))
    }

    Method(CPIN, 0x3, NotSerialized) {
    // ***************************************************************************
    // Method:
    //   CPIN
    //
    // Description:
    //   Config DIO pin
    //
    // Entry:
    //   Arg0 : Type (Singal Pin:0 ; Multiple Pin: 1)
    //   Arg1 : Pin number (Single type); Pin Bit mask (Multiple type: Bits are pin numbers , H/L are In/Output setting)
    //   Arg2 : Setting ; Not reference in multiple type
    //
    // Return:
    //   0x00 : Success
    //   0xFF : Error, input parameter error:
    //                  pin number greater than DIO define amount,
    //                  pin number is not in mask

    // ***************************************************************************
      //
      // Check input parameters are valid
      //
      if (LEqual (Arg0, SINGLE_PIN)) {
        if (LGreaterEqual(Arg0, WMI_DIO_AMOUNT)) {
          Store(0x0,\STDT)  //CMFC False
          return (WMI_INVALID_PARAMETER)
        }
        ShiftLeft(1, Arg1,PINB)
        ShiftLeft(Arg2, Arg1, PIND)
      } else {
        Store (DIO_BIT_MASK, PINB)
        Store (Arg1, PIND)
      }

      And (DIO_BIT_MASK, PINB, PINB)
      if (LEqual (PINB,Zero)) {
        Store(0x0,\STDT)  //CMFC False
        return (WMI_INVALID_PARAMETER)
      }
      //
      // Read config regsiter
      //
      Divide (WMI_DIO_AMOUNT, RW_SIZE, Local0, Local1)
      if (LNotEqual (Local0, 0)) {
        Add (Local1, 1, Local1)
      }

      Store (RDRG (CONFIG_REG_OFFSET, Local1), Local0)
      if (WMI_ERROR(ERRO)) {
        Store(0x0,\STDT)  //CMFC False
        return (ERRO)
      }
      //
      // Update config register
      //
      And (Local0, Not(PINB), Local0)
      Or (Local0, PIND, Local0)
      return (WTRG (CONFIG_REG_OFFSET, Local1, Local0))
    }

    Method(WMIF, 0x1, NotSerialized) {
    // ***************************************************************************
    // Method:
    //   WMIF
    //
    // Description:
    //   Get DIO information
    //
    // Entry:
    //   Arg0 : Information type :
    //            DIO_GetSupport
    //            DIO_GetAmount
    //            DIO_BIT_MASK
    // ***************************************************************************
      //
      // WMI DIO function check
      //
      Switch (Arg0) {
        Case (DIO_GetSupport) {
          Store(0x1,\STDT)  //CMFC Function Success
          Store(0x01,\BFDT) 
          Return(0x01)
        }

        Case (DIO_GetAmount) {
          Store(0x1,\STDT)  //CMFC Function Success
          Store(WMI_DIO_AMOUNT,\BFDT) 
          Return(WMI_DIO_AMOUNT)
        }

        Case (DIO_GetSettings) {
          Store(0x1,\STDT)  //CMFC Function Success
          Store(DIO_BIT_MASK,\BFDT) 
          Return(DIO_BIT_MASK)
        }

        Default {
          Store(0x8000,\STDT)  //CMFC Function Not Support
          Return(WMI_UNSUPPORTED)    // Not support
        }
      }
    }
  }
