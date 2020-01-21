/*
 * (C) Copyright 2012-2020 Compal Electronics, Inc.
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
 1.01       14/9/16  Vanessa      Move SMI Cmd/Data Port define to common code from Prj tip.
 1.02       14/11/05 Vanessa	  Modify OPRegion size to WORD.
 1.03       16/03/02 Stan         Implement Compal touchpad feature.
*/

  //
  // Compal common NVS definition.
  // For common used, utility or tool need refernecne this name 'COMP'
  // And the offset should be fixed up.
  // 
  
  OperationRegion(COMP,SystemMemory,0xFFFF0000,0xAA55)  
  Field(COMP,AnyAcc,Lock,Preserve)
  {
    // The definitions below need to be matched CompalGlobalNvsArea.h 
    // and can be modified by Compal common code
    //
    //
    RSR0, 32,       //    (00~03) Reserved 4 Bytes
    BDDD, 8,        //    (04) Board ID
    CPFB, 8,        //    (05) CompalFastBootEnable 
    PBTI, 8,        //    (06) Panel Brightness Table Index
    BRLV, 8,        //    (07) Brightness Level   	  
    CAVR, 8,        //    (08) CMFC AP Version
    TJMA, 16,       //    (09,10) Cpu TjMax Type  
    CORE, 8,	    //    (11) CPU Core Number
    TPDF, 8,        //    (12) Compal touchpad id flag 
    CG13, 8,        //    (13) Compal GlobalNvStore Data 13
    CG14, 8,        //    (14) Compal GlobalNvStore Data 14
    CG15, 8,        //    (15) Compal GlobalNvStore Data 15
    CG16, 8,        //    (16) Compal GlobalNvStore Data 16
    CG17, 8,        //    (17) Compal GlobalNvStore Data 17
    CG18, 8,        //    (18) Compal GlobalNvStore Data 18
    CG19, 8,        //    (19) Compal GlobalNvStore Data 19             
    CG20, 8,        //    (20) Compal GlobalNvStore Data 20
    CG21, 8,        //    (21) Compal GlobalNvStore Data 21
    CG22, 8,        //    (22) Compal GlobalNvStore Data 22
    CG23, 8,        //    (23) Compal GlobalNvStore Data 23
    CG24, 8,        //    (24) Compal GlobalNvStore Data 24
    CG25, 8,        //    (25) Compal GlobalNvStore Data 25
    CG26, 8,        //    (26) Compal GlobalNvStore Data 26
    CG27, 8,        //    (27) Compal GlobalNvStore Data 27
    CG28, 8,        //    (28) Compal GlobalNvStore Data 28
    CG29, 8,        //    (29) Compal GlobalNvStore Data 29	
    CG30, 8,        //    (30) Compal GlobalNvStore Data 30
    CG31, 8,        //    (31) Compal GlobalNvStore Data 31             
    SFNO, 16,       //    (32,33) Sub Function No of SWSMI 0xCE
    STDT, 16,       //    (34,35) Status Data of SWSMI 0xCE
    BFDT, 1024,     //    (36~163) Input/Oput Buffer Data[128] of SWSMI 0xCE
    RSR1, 736,      //    (164~255) Reserved 92 bytes
    IDFD, 1024,     //    (256~383) EDID Full Data
    RSR2, 1024,     //    (384~511) Reserved 128 bytes
  }

  //
  // Intel chip SW SMI port B2/B3 definition.
  // For Compal common used, utility or tool.
  // 
  OperationRegion (SMIO, SystemIO, 0xB2, 0x02)
  Field (SMIO, ByteAcc, NoLock, Preserve) {
    SMIC, 8,
    SMID, 8
  }

