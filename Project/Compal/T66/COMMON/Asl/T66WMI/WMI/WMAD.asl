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
External(\_PR.DTS1, FieldUnitObj)
External(\_PR.DTS2, FieldUnitObj)
External(\_PR.DTSF, FieldUnitObj)
External (\_SB.CSMI, MethodObj)
External(TMUD, FieldUnitObj)







  //
  // Evaluation of this method causes the event 0xA0 to be fired. Since it is
  // defined by the _WDG method it is callable via WMI. Arg0 has the instance
  // index and Arg1 has any input parameters.
     Method(WMAD, 3) {
     // ***************************************************************************
     // Method:
     //   WMAD
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
      //Buffer for WMAD
      Name(ADBF, Buffer(4){})
      if (LEqual (WMI_WMAD_SUPPORT, 0)) {
        // Not support , return 0x8000
        Store(0x8000,STDT)  //CMFC Function Not Support
        return (WMI_UNSUPPORTED)
      }

      Switch (Arg1) {

        Case (WMAD_FUNC_GetCPUDTS) {
        //  Store(20, \_PR.DTSF)                    // INIT_DTS_FUNCTION_AFTER_S3
          \_SB.CSMI(ThermalUtility_SW_SMI, Read_DTS_Temperature)        // Notify DTS SW SMI, Sub-function is null.
          Sleep(10)
          Store(0x1,STDT)  //CMFC Function Success
          If(LGreater(\_PR.DTS2, \_PR.DTS1)) {
            Store(\_PR.DTS2, Local0)
          } else {
            Store(\_PR.DTS1, Local0)
          }
          Return (TMUD)
        }

        Default {
          Store(0x8000,STDT)  //CMFC Function Not Support
          Return(WMI_UNSUPPORTED)    // Not support
        }
      }
    }

