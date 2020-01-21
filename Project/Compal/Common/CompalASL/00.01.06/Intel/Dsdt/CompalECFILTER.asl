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
 1.01	   16/06/22  Joy	  Remove EC RAM definition in CompalECFILTER.asl 
 				  Change PTVL to PCVL to sync up Compalecname.asl
 				  Remove #include "CompalCMFC.ASL" from CompalECFILTER.asl 
*/

//
// [COMMON ASL] EC functions using EDK II
//

//
// Please refer below for more information to set ECFILTER.
// -----------------------------------------------------------------
// If set INTEL_ECFILTER == "ENABLE" on OEMASL.asl,
//     you can only find Intel function after ASL compile.
// If set INTEL_ECFILTER == "DISABLE" on OEMASL.asl,
//     you can only find AMD function after ASL compile.
//
// Default setting - INTEL_ECFILTER == ENABLE
// -----------------------------------------------------------------
// If set CPU_THROTTLING_SUPPORT == "ENABLE" on OEMASL.asl 
// CPU throttling information will show in Thermal utility.
// If set CPU_THROTTLING_SUPPORT == "DISABLE" on OEMASL.asl
// CPU throttling information will not show in Thermal utility.
// 
// Default setting - CPU_THROTTLING_SUPPORT == DISABLE

#ifndef INTEL_ECFILTER
        #define INTEL_ECFILTER  ENABLE
#endif
#ifndef PMIO_REG10
        #define PMIO_REG10 0x1810 //[COM] 
#endif

#ifndef CPU_THROTTLING_SUPPORT
        #define CPU_THROTTLING_SUPPORT  DISABLE
#endif

OperationRegion(CCLK, SystemIO, PMIO_REG10 ,0x04)
Field(CCLK, DWordAcc, NoLock, Preserve)
{
	, 1,                          //reserved
	DUTY, 3,                      //Duty Cycle bits 3:1
	THEN, 1,                      //Throttling Enable bits 4
	, 3 ,                         //Reserved 7:5
    FTT, 1 ,                      //Force Thermal Throttling
    , 8 ,                         //Reserved 16:9
	TSTS, 1,                      //Throttling Enable status bit 17
}
Mutex(FAMX,0x0)

Method(FANG, 1)
{
	Acquire(FAMX, 0xFFFF)

	Store(Arg0, ERIB)
	Store(ERBD, Local0)

	Release (FAMX)

	Return (Local0)
}

Method(FANW, 2)
{
	Acquire(FAMX, 0xFFFF)

	Store(Arg0, ERIB)
	Store(Arg1, ERBD)

	Release (FAMX)

	Return (Arg1)
}

// Return ECFilter version
Method (TUVR, 1)
{
	Return(0x04)
}

//Current throttling value
Method(THRO, 1)
{
    #if (INTEL_ECFILTER == ENABLE)

        ITHR(Arg0)                              // For Intel Current throttling.

    #else

        ATHR(Arg0)                              // For AMD Current throttling.

    #endif

}

// Control CPU clock
Method(CLCK , 1)
{
    #if (INTEL_ECFILTER == ENABLE)

        If (LEqual(Arg0, 0))
        {
                Store(0, THEN)
                Store(0, FTT)
        }
	else
	{
		Store(arg0, DUTY)
		Store(1, THEN)
	}
        return(THEN)

    #else

        // AMD platform not support this interface.
        Return(0x00)

    #endif
}

//For EC Control Throttling (base on POWER consumption)
Method(PCLK)
{
    #if (INTEL_ECFILTER == ENABLE)

        IPCL()

    #else

        APCL()

    #endif
}


#if (INTEL_ECFILTER == ENABLE)

/*---------------------------------------------------------------------------
 # For Intel thermal function.
 *---------------------------------------------------------------------------*/
      //
      // Intel Current throttling.
      //
      Method(ITHR, 1)
      {
      	If (LEqual(Arg0, 0))
      	{
      	  #if (CPU_THROTTLING_SUPPORT == ENABLE)
             Return(THEN)                       //return throttling enabled bit                        
          #else       
             Return(0xff)                       //CPU throttling is not supported 
          #endif      		
      	}
      	ELSE
      	{
      		If (LEqual(Arg0, 1))
      		{ Return(DUTY) }                    //return duty cycle
      		ELSE
                {
      			If (LEqual(Arg0, 2))
      			{ Return(TTHR) }                //return themal throttling status
                        ELSE
                        { Return(0xff) }
      		}
      	}
      } // End of ITHR

      //
      // Intel thermal function to Control EC Throttling (base on POWER consumption).
      //
      Method(IPCL)
      {
              // To use P-State transition to replace throttling function.
              Store(PCVL, Local0)
              Store(Local0, \_PR.CPU0._PPC)

              // Notify OS Performance Present Capabilities Changed.
              PNOT()

      }

#else

/*---------------------------------------------------------------------------
 # For AMD thermal function.
 *---------------------------------------------------------------------------*/
      External(ASL_CPU0._PPC, IntObj)
      External(ASL_CPU1._PPC, IntObj)
      External(ASL_CPU2._PPC, IntObj)
      External(ASL_CPU3._PPC, IntObj)
      //
      // AMD Current throttling.
      //
      Method(ATHR, 1)
      {
      	If(LEqual(Arg0, 0))
      	{
      	  #if (CPU_THROTTLING_SUPPORT == ENABLE)
             If(LEqual(ASL_CPU0._PPC, 0))
             {
                Return(0x01)                        //Throttling(P-State) Enable
             }
             Else
             {
      		      Return(0x00)                      //Throttling(P-State) Disable
             }                        
          #else       
             Return(0xff)                           //CPU throttling is not supported 
          #endif      		                      
      	}
      	Else
      	{
      		If(LEqual(Arg0, 1))
      		{
      		        Return(ASL_CPU0._PPC)           //Return P-State Level
                      }
      		Else
                      {
      			If(LEqual(Arg0, 2))
      			{
      			        Return(TTHR)                //Return themal throttling status
                              }
                        	Else
                          	{
                          	        Return(0xff)
                              }
      		}
      	}

      } // End of ATHR

      //
      // AMD thermal function to Control EC Throttling (base on POWER consumption).
      //
      Method(APCL)
      {
              // AMD Platform can't use HTC to replace throttling.
              // It will cause system hang up when enable C1E at the same time.
              // So, we need to use P-State transition to replace throttling function.
              // The algorithm is as below.
              // 1. If EC control P-State level <= P-State number
              //    _PPC = PCVL
              // 2. If EC control P-State level >  P-State number
              //    _PPC = PPC0 or PPC1
              Store(PCVL, Local0)
              Store(PPC0, Local1)
              Store(PPC1, Local2)

              If(LLessEqual(Local0, Local1))          // Request state < P-State Number
              {
                      Store(Local0, ASL_CPU0._PPC)
                      If(LGreaterEqual(MPEN, 1))      // Check Multi-Processor?
                      {                               // Dual Core
                              Store(Local0, ASL_CPU1._PPC)
                              If(LGreaterEqual(MPEN, 2))              // Check Multi-Processor?
                              {                                       // Triple Core
                                      Store(Local0, ASL_CPU2._PPC)
                                      If(LGreaterEqual(MPEN, 3))      // Check Multi-Processor?
                                      {                               // Quad Core
                                              Store(Local0, ASL_CPU3._PPC)
                                      }
                              }
                      }
              }
              Else
              {
                      Store(Local1, ASL_CPU0._PPC)
                      If(LEqual(MPEN, 1))             // Check Multi-Processor?
                      {                               // Dual Core
                              Store(Local2, ASL_CPU1._PPC)
                              If(LGreaterEqual(MPEN, 2))              // Check Multi-Processor?
                              {                                       // Triple Core
                                      Store(Local2, ASL_CPU2._PPC)
                                      If(LGreaterEqual(MPEN, 3))      // Check Multi-Processor?
                                      {                               // Quad Core
                                              Store(Local2, ASL_CPU3._PPC)
                                      }
                              }
                      }
              }

              // Notify OS Performance Present Capabilities Changed.
              Notify(ASL_CPU0,0x80)                  // Notify CPU P state capabilities change
              If(LGreaterEqual(MPEN, 1))             // Check Multi-Processor?
              {                                      // Dual Core
                      Notify(ASL_CPU1,0x80)          // Notify CPU P state capabilities change
                      If(LGreaterEqual(MPEN, 2))             // Check Multi-Processor?
                      {                                      // Triple Core
                              Notify(ASL_CPU2,0x80)          // Notify CPU P state capabilities change
                              If(LGreaterEqual(MPEN, 3))     // Check Multi-Processor?
                              {                              // Quad Core
                                      Notify(ASL_CPU3,0x80)  // Notify CPU P state capabilities change
                              }
                      }
              }

      } // End of APCL method.

#endif

