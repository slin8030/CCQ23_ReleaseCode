/* (C) Copyright 2011-2020 Compal Electronics, Inc.
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
 1.00                             Init version.
 1.01      16/03/02  Stan         Add TPDF flag for Compal touchpad feature.
*/
#ifndef _COMPAL_GLOBAL_NVS_AREA_H_
#define _COMPAL_GLOBAL_NVS_AREA_H_

//
// COMPAL Global NVS Area definition
//
#pragma pack (1)
typedef struct {

  //
  // The definitions below need to be matched COMP definitions in CompalGlobalNvs.ASL
  // and can be modified by Compal Common code
  //
  UINT8       Reserved0[4];                   // 00~03
  UINT8       OemGlobalBoardID;               // 04
  UINT8       CompalFastBootEnable;           // 05
  UINT8       PanelBrightnessTableIndex;      // PBTI (06) Panel Brightness Table Index
  UINT8       BrightnessLevel;                // BRLV (07) Brightness Level
  UINT8       CMFCAPVersion;                  // CAVR (08) CMFC AP Version
  UINT16      CPUTjMax;                       // 09 10 CPU TjMax Type
  UINT8       CPUCoreNumber;                  // 11 CPU Core Number
  UINT8       TouchpadIdFlg;                  // 12 TouchPad ID
  UINT8       TouchPanelIdFlg;                // 13 Compal touchpanel id flag
  UINT8       TouchpadDevice;                 // 14 TouchpadDevice (Project.env settings:0x00, PS2:0x01, I2C:0x02)
  UINT8       STPMThermalUtilityData;         // 15
  UINT8       CompalGlobalNvsData16;          // 16
  UINT8       CompalGlobalNvsData17;          // 17
  UINT8       CompalGlobalNvsData18;          // 18
  UINT8       CompalGlobalNvsData19;          // 19
  UINT8       CompalGlobalNvsData20;          // 20
  UINT8       CompalGlobalNvsData21;          // 21
  UINT8       CompalGlobalNvsData22;          // 22
  UINT8       CompalGlobalNvsData23;          // 23
  UINT8       CompalGlobalNvsData24;          // 24
  UINT8       CompalGlobalNvsData25;          // 25
  UINT8       CompalGlobalNvsData26;          // 26
  UINT8       CompalGlobalNvsData27;          // 27
  UINT8       CompalGlobalNvsData28;          // 28
  UINT8       CompalGlobalNvsData29;          // 29
  UINT8       CompalGlobalNvsData30;          // 30
  UINT8       CompalGlobalNvsData31;          // 31
  UINT16      SubFunctionNo;                  // 32 33 Sub Function No of SWSMI 0xCE
  UINT16      StatusData;                     // 34 35 Status Data of SWSMI 0xCE
  UINT8       BufferData[128];                // 36~163 Input/Oput Buffer Data[128] of SWSMI 0xCE
  UINT8       Reserved1[92];                  // 164~255
  UINT8       EDIDFullData[128];              // 256~383 EDID Full Data
  UINT8       Reserved2[128];                 // 384~511
} COMPAL_GLOBAL_NVS_AREA;
#pragma pack ()

#endif
