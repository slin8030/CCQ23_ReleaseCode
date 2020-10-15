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
     Method(WMAE, 3) {
     // ***************************************************************************
     // Method:
     //   WMAE
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
       if (LEqual (WMI_WMAE_SUPPORT, 0)) {
         // Not support , return 0x8000
         Store(0x8000,\STDT)  //CMFC Function Not Support
         return (WMI_UNSUPPORTED)
       }

       Switch (Arg1) {
         Case (EC_ReadWriteCommend) {  
           Name (EDIB, Buffer(8) {0x00})      //EC Data In Buffer
           CreateField (EDIB, 0, 16, ECI0)    //Command 0x80xx read, 0x00xx write 
                                              //Need check I2C_CMD_for WOA_V1.0.xls
           CreateField (EDIB, 16, 40, ECI1)    //[0]= The Length of Input Data for EC Command
                                               //[1]= The Length of Return Data of EC Command
                                               //[2~]= Input Data array for EC Read Command
           Store(Arg2,EDIB)

           //Init CMFC 
          Store(0x01, SFNO)
          Store(0x01, BFDT)
          Stall(255)
		  
          \_SB.CSMI(WMI_SW_SMI,CMFC_VERSION_CTRL)
          Stall(255)
	
          Store(ECI0, SFNO)                 //Command
          Store(ECI1, BFDT)                 //Data

          Stall(255)
          \_SB.CSMI(WMI_SW_SMI,CMFC_EC_COMMUNICATE_INTERFACE)
          Stall(255)	
           Return(BFDT)     //Need to transfer to HEX
        }

		
        Default {
          Store(0x8000,\STDT)  //CMFC Function Not Support
          Return(WMI_UNSUPPORTED)    // Not support
        }

       }
     }

