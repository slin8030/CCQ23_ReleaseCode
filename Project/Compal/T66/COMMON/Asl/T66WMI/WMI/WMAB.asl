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



  //
  // Evaluation of this method causes the event 0xA0 to be fired. Since it is
  // defined by the _WDG method it is callable via WMI. Arg0 has the instance
  // index and Arg1 has any input parameters.
     Method(WMAB, 3) {
     // ***************************************************************************
     // Method:
     //   WMAB
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
      if (LEqual (WMI_WMAB_SUPPORT, 0)) {
        // Not support , return 0x8000
        Store(0x8000,\STDT)  //CMFC Function Not Support
        return (WMI_UNSUPPORTED)
      }


      Switch (Arg1) {
        Case (Reload_BIOS_Default_Value) {
          //Init CMFC 
          Store(0x01, SFNO)
          Store(0x01, BFDT)
          Stall(255)
		  
          \_SB.CSMI(WMI_SW_SMI,CMFC_VERSION_CTRL)
          Stall(255)
		  
          Store(0x00, SFNO)
          Store(0x00, BFDT)
          Stall(255)
          \_SB.CSMI(WMI_SW_SMI,CMFC_RELOAD_BIOS_DEFUAL_TVALUE)
          Stall(255)		  
		  
          return (CTWE(STDT))
        }

        Case (SetPxeBootFirst) {
          //Init CMFC
          Store(0x01, SFNO)
          Store(0x01, BFDT)
          Stall(255)
          \_SB.CSMI(WMI_SW_SMI,CMFC_VERSION_CTRL)
    
          Store(0x01, SFNO)
          Store(03,BFDT)

          Stall(255)
          \_SB.CSMI(WMI_SW_SMI,CMFC_BOOT_DEVICE_SEQUENCE)
          Stall(255)		   		  
		  
          return (CTWE(STDT))
        }
		
        Default {
          Store(0x8000,\STDT)  //CMFC Function Not Support
          Return(WMI_UNSUPPORTED)    // Not support
        }
      }
    }

