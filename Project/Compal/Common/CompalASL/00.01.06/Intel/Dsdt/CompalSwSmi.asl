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
 1.00                Justy        Init version for Compal SW SMI.
 1.01      16/08/22  Vanessa      Support CPU turbo On/Off SW SMI 0xDA

*/
//////////////////////////////////////////////////////////////////////////////////////
// Compal SW SMI number list                                                        //
//////////////////////////////////////////////////////////////////////////////////////
#define CMFC_SW_SMI                 0xCE
#define CMFC_OEM_SW_SMI             0xCF
#define STPM_Thermal_Utility_SW_SMI	0xD9
  #define Read_DTS_Temperature            0x01
#define CPU_TurboMode_SW_SMI        0xDA
  #define CPU_Turbo_On                    0x00
  #define CPU_Turbo_Off                   0x01

Mutex(CFMX,0x0)

Scope(\_SB){
  //
  // Intel chip SW SMI port B2/B3 definition.
  // For Compal common used, utility or tool.
  // 
  OperationRegion (SMIO, SystemIO, 0xB2, 0x02)
  Field (SMIO, ByteAcc, NoLock, Preserve) {
    SMIC, 8,
    SMID, 8
  }

  Method(CSMI, 2)
  {
    Acquire(CFMX, 0xFFFF)
    Store(Arg1, SMID)
    Store(Arg0, SMIC)
	Release(CFMX)
  }

// sample
//        \_SB.CSMI(CPU_TurboMode_SW_SMI, CPU_Turbo_Off)     // SMI Service 0xDA, Sub function 01

}

