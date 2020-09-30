/*---------------------------------------------------------------------------
 *
 *	$Workfile:   T66WMI.ASL  $
 *
 *	$Header:   $
 *
 *	Copyright (c) 2016~ Compal Company
 *
 *	This program contains proprietary and confidential information.
 *	All rights reserved except as may be permitted by prior
 *	written consent.
 *
 *	Content:
 *
 *		R E V I S I O N    H I S T O R Y
 *	$Log:   $
 *
 *  Date        Who             Change
 *	----------  ------------    -------------------------------------------
 *-------------------------------------------------------------------------*/
// #define DIOP    \_SB.PCI0.I2C2  // please define DIOP in OemASL.asl

External(DIOP, DeviceObj)
External(DIOP.WMIF, MethodObj)
External(DIOP.RPIN, MethodObj)
External(DIOP.CPIN, MethodObj)
External(DIOP.WPIN, MethodObj)

//********-[ DIO function list ]-*****************
// #define DIO_GetSupport         0x01
// #define DIO_GetAmount          0x02
// #define DIO_GetSettings        0x03
// #define DIO_SetDirection       0x04
// #define DIO_SetDirectionmany   0x05
// #define DIO_GetValue           0x06
// #define DIO_GetValuemany       0x07
// #define DIO_SetValue           0x08
// #define DIO_SetValuemany       0x09
//**************************************************

  //
  // Evaluation of this method causes the event 0xA0 to be fired. Since it is
  // defined by the _WDG method it is callable via WMI. Arg0 has the instance
  // index and Arg1 has any input parameters.
    Method(WMAC, 3) {
    // ***************************************************************************
    // Method:
    //   WMAC
    //
    // Description:
    //
    // Entry:
    //   Arg0 : instance name
    //   Arg1 : Function ID
    //   Arg2 : Input parameter 32bit
    // ****************************************************************************
      Store(0,\STDT)  //Clear STDT
      Store(0,\BFDT)  //Clear BFDT 
    //Arg2 data structure
      Name (ACPM, Buffer(4) {0x00})
      CreateByteField (ACPM, 0, PINN )   // Pin number
      CreateBitField  (ACPM, 8, PSET)   // Pin Setting.
      CreateWordField (ACPM, 2, RESE )   // Reseve

      if (LEqual (WMI_WMAC_SUPPORT, 0)) {
        // Not support , return 0x8000
        Store(0x8000,\STDT)  //CMFC Function Not Support
        return (WMI_UNSUPPORTED)
      }

      Store(Arg2, ACPM)
      Switch (Arg1) {
        Case (Package () {
          DIO_GetSupport,
          DIO_GetAmount,
          DIO_GetSettings}) {
          Return (DIOP.WMIF(Arg1))
        }

        Case (DIO_SetDirection) {
          //PINN: Pin Number
          //PSET: Pin Setting, Direction: 0: Output, 1: Input
          Return (DIOP.CPIN(SINGLE_PIN,PINN,PSET))
        }

        Case (DIO_SetValue) {
          //PINN: Pin Number
          //PSET: Pin Setting, Direction: 0: Low, 1: High
          Return (DIOP.WPIN(SINGLE_PIN,PINN,PSET))
        }

        Case (DIO_GetValue) {
          //PINN: Pin Number
          Return (DIOP.RPIN(SINGLE_PIN,PINN))
        }

        Case (DIO_SetDirectionmany) {
          //Bits are pin numbers, 0: Output, 1: Input
          Return (DIOP.CPIN(MULTIPLE_PIN, Arg2, 0))
        }

        Case (DIO_SetValuemany) {
          //Bits are pin numbers, Direction: 0: Low, 1: High
          Return (DIOP.WPIN(MULTIPLE_PIN, Arg2, 0))
        }
          //Bits are pin numbers,

        Case (DIO_GetValuemany) {
          //Pin: Number, Direction: 0: Low, 1: High
          Return (DIOP.RPIN(MULTIPLE_PIN, Arg2))
        }

        Default {
          Store(0x8000,\STDT)  //CMFC Function Not Support
          Return(WMI_UNSUPPORTED)    // Not support
        }
      }
    }

