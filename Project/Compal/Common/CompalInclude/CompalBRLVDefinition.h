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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
*/

#define CompalBRLV_PanelID_ORB 0
//#define CompalBRLV_PanelID_A31 1
//#define CompalBRLV_PanelID_A32 2
//#define CompalBRLV_PanelID_ABO 3
//#define CompalBRLV_PanelID_A51 4
//#define CompalBRLV_PanelID_C38 5

static CompalBRLVPANEL_TABLE CompalBRLVPanelIDTABLE[] = {
#ifdef CompalBRLV_PanelID_ORB
        {0x1462, 10,20, 30, 40, 50, 60, 75, 81, 88, 94, 100},    // CMO N141I6
        {0x22EC, 0,  8, 16, 24, 34, 44, 54, 65, 76, 87, 100},    // AUO B156XTN02.2(0A)
#endif

#ifdef CompalBRLV_PanelID_A32
        {0x0644, 5,  9, 13, 17, 24, 32, 41, 50, 59, 72, 100},    // BOE HB140WX1-301
        {0x1B3D, 5,  9, 13, 17, 23, 31, 38, 47, 55, 66, 100},    // AUO B140HTN01.B
        {0x4E42, 5,  9, 13, 17, 22, 30, 37, 45, 54, 66, 100},    // SDC LTN140AT35-H01
        {0x323C, 5,  9, 13, 17, 24, 33, 43, 53, 62, 76, 100},    // AUO B140XTN03.2
        {0x0640, 5,  9, 13, 17, 24, 33, 43, 53, 62, 76, 100},    // BOE HB140FH1-401
        {0x1494, 5,  9, 13, 17, 24, 33, 43, 51, 60, 72, 100},    // INX N140BGE-E43
        {0x14A7, 5,  9, 13, 17, 23, 33, 41, 50, 60, 72, 100},    // INX N140HGE-EAA
        {0x0455, 5,  9, 13, 17, 22, 30, 38, 47, 55, 67, 100},    // LGD LP140WH8-TPC1  LGD LP140WH8-TPC2
        {0x3143, 5,  9, 13, 18, 26, 36, 46, 56, 66, 80, 100},    // LTN140HL05-301
        {0x14B1, 5,  9, 13, 17, 21, 29, 37, 45, 53, 65, 100},    // N140HCE-EAA
        {0x0638, 5,  9, 13, 17, 25, 34, 44, 53, 63, 74, 100},    // NV140FHM-N41
        {0x04B2, 5, 10, 13, 19, 27, 37, 47, 57, 67, 81, 100},    // LP140WF6-SPD1
        {0x0640, 5,  9, 13, 17, 24, 33, 43, 53, 62, 76, 100},    // HB140FH1-401
        {0x1B3D, 5,  9, 13, 17, 23, 31, 38, 47, 55, 66, 100},    // B140HTN01.B

        {0x0614, 7, 11, 15, 20, 29, 40, 50, 61, 68, 87, 100},    // NT156WHM-N22
        {0x063B, 7, 11, 15, 20, 29, 40, 50, 61, 72, 87, 100},    // NT156WHM-N12
        {0x4E51, 6, 10, 14, 18, 25, 35, 45, 55, 63, 79, 100},    // LTN156AT39-H01
        {0x4C51, 6, 10, 14, 18, 25, 35, 45, 55, 63, 79, 100},    // LTN156AT39-301
        {0x46EC, 6, 10, 14, 18, 23, 31, 40, 48, 55, 63, 100},    // B156XTN04.6
        {0x45EC, 4,  8, 12, 16, 22, 31, 39, 48, 54, 68, 100},    // B156XTN04.5
        {0x15C9, 6, 10, 14, 18, 26, 35, 45, 53, 62, 78, 100},    // N156BGE-E32
        {0x15BE, 4,  8, 12, 16, 23, 32, 41, 50, 56, 71, 100},    // N156BGE-E31
        {0x15CA, 6, 10, 14, 18, 26, 34, 45, 55, 61, 77, 100},    // N156BGE-E42
        {0x15BF, 4,  8, 12, 16, 23, 31, 40, 48, 55, 69, 100},    // N156BGE-E41
        {0x0465, 5,  9, 13, 18, 26, 36, 46, 55, 64, 79, 100},    // LP156WHB-TPD3/TPD2
        {0x0493, 6, 10, 14, 19, 26, 36, 46, 57, 67, 82, 100},    // LP156WHB-TPC1
        {0x0468, 6, 10, 14, 19, 26, 36, 46, 57, 67, 82, 100},    // LP156WHB-TPC2
        {0x38ED, 6, 10, 14, 18, 23, 32, 41, 49, 57, 72, 100},    // B156HTN03.6 4A
        {0x062B, 6, 10, 14, 18, 26, 35, 45, 54, 63, 80, 100},    // HB156FH1-401
        {0x15C4, 6, 10, 14, 18, 24, 33, 43, 52, 60, 78, 100},    // N156HGE-EAB C1
        {0x141C, 5,  9, 13, 17, 21, 29, 37, 45, 52, 75, 100},    // LQ156Z1JW03B
        {0x044F, 6, 10, 14, 19, 26, 36, 46, 57, 66, 82, 100},    // LGD LP156WF4-SPL1
        {0x04B3, 6, 10, 14, 19, 26, 36, 46, 57, 67, 80, 100},    // LGD LP156WF4-SPL2
        {0x5444, 6, 10, 14, 19, 25, 36, 45, 56, 66, 79, 100},    // SDC 156HL07-301
        {0x0679, 6, 10, 14, 19, 27, 37, 47, 57, 66, 82, 100},    // NV156FHM-N41 
        {0x5544, 6, 10, 14, 20, 26, 37, 46, 57, 67, 78, 100},    // SDC 156HL07-001

        {0x3854, 5,  9, 13, 17, 21, 26, 30, 34, 40, 57, 100},    // LTN173HL01-301
        {0x046E, 5,  9, 13, 17, 21, 25, 29, 35, 41, 57, 100},    // LP173WF4-SPF2
        {0x109D, 6, 10, 14, 18, 22, 26, 30, 35, 40, 57, 100},    // B173HAN01.0
        {0x1738, 5,  9, 13, 17, 21, 25, 34, 42, 45, 69, 100},    // INX N173HCE-E31

        {0x272D, 5,  6,  9, 12, 18, 24, 31, 37, 44, 65, 100},    // AUO B133HAN02.7
        {0x415A, 5,  6,  8, 11, 17, 22, 29, 35, 42, 63, 100},    // SDC LTN133YL06-H01
        {0x1367, 5,  6,  7, 10, 15, 19, 25, 31, 39, 58,  99},    // INX N133HCE
        {0x4259, 5,  7,  9, 13, 19, 24, 32, 40, 50, 71, 100},    // SDC LTN133L09
#endif

#ifdef CompalBRLV_PanelID_ABO
	// ABO LED 10~12 size panel
	{0x2052, 10, 17, 26, 35, 42, 52, 65, 74, 83, 91, 100},   // B116AW02 V0
	{0x325C,  8, 17, 25, 34, 43, 53, 67, 77, 85, 92, 100},   // B116XW03 V2, B116XW03 V2 (1B)
	{0x1113, 10, 20, 30, 40, 50, 60, 75, 81, 88, 94, 100},   // N116BGE-L41
	{0x1119, 10, 20, 30, 40, 50, 60, 75, 81, 88, 94, 100},   // N116BGE-L42
	{0x1118, 10, 20, 30, 40, 50, 60, 75, 81, 88, 94, 100},   // N116BGE-L42
	{0x405C,  8, 17, 25, 34, 43, 53, 67, 77, 85, 92, 100},   // B116XTN04.0 (0A)

	// ABO LED 14 size panel
	{0x303C,  9, 18, 27, 36, 45, 54, 68, 76, 84, 92, 100},   // B140XW03 V0
	{0x1457, 10, 20, 30, 40, 50, 60, 75, 81, 88, 94, 100},   // N140B6-L24
	{0x001E, 10, 20, 30, 40, 50, 60, 75, 81, 88, 94, 100},   // BT140GW03 V.2
	{0x322C, 10, 20, 30, 40, 50, 60, 75, 81, 88, 94, 100},   // B133XW03 V2
	{0x332C, 10, 20, 30, 40, 50, 60, 75, 81, 88, 94, 100},   // B133XW03 V3
	{0x122C,  8, 16, 24, 32, 40, 48, 60, 70, 80, 90, 100},   // B133XTN01.2
	{0x243C, 10, 17, 26, 34, 43, 52, 64, 73, 82, 91, 100},   // B140XTN02.4
 
	// ABO LED 15 size panel
	{0x22EC,  9, 17, 26, 35, 43, 52, 66, 75, 83, 92, 100},   // B156XW02 V2 (4A), B156XW02 V2 (3A), B156XTN02.2(0A)
	{0x1680, 10, 18, 27, 33, 41, 51, 61, 71, 81, 91, 100},   // N156B6-L06
	{0xCF01, 10, 17, 26, 34, 42, 53, 65, 74, 83, 92, 100},   // LP156WH2-TLE1
	{0x4141,  8, 16, 24, 31, 39, 48, 60, 70, 80, 90, 100},   // LTN156AT02-A01, LTN156AT02-A02, LTN156AT02-A03, LTN156AT07-A01
	{0x0005,  8, 16, 24, 32, 40, 52, 64, 73, 82, 91, 100},   // BT156GW01 V2
	{0x31EC,  9, 19, 28, 35, 47, 57, 67, 75, 83, 91, 100},   // B156XW03 V1
	{0x1592, 10, 18, 27, 36, 45, 55, 68, 76, 84, 92, 100},   // N156B6-L0B
	{0x40EC, 10, 19, 28, 37, 47, 57, 68, 76, 84, 92, 100},   // B156XW04 V0
	{0x2297, 10, 20, 30, 40, 50, 60, 73, 80, 87, 94, 100},   // LP156WH3-TLL1
	{0x23EC,  8, 16, 24, 34, 44, 54, 65, 74, 82, 91, 100},   // B156XW02 V3, B156XTN02.3 (0A)
	{0x0210, 10, 20, 30, 40, 50, 60, 73, 80, 87, 94, 100},   // LP156WH3-TLA2
	{0x0250,  8, 16, 24, 32, 40, 50, 62, 71, 80, 89, 100},   // LP156WH2-TLEA
	{0x14C7,  8, 16, 25, 33, 42, 53, 65, 74, 83, 92, 100},   // CLAA156WB11A
	{0x0593,  8, 17, 26, 35, 44, 55, 67, 75, 83, 91, 100},   // HT156WXNB-500, HT156WXB-500
	{0x494A,  9, 17, 25, 34, 42, 51, 64, 73, 82, 91, 100},   // LTN156AT02-A11
	{0x26EC,  8, 17, 26, 36, 45, 55, 68, 76, 83, 91, 100},   // B156XW02 V6
	{0x030A, 10, 19, 28, 38, 47, 57, 71, 78, 85, 93, 100},   // LP156WH3-TLAA
	{0x45EC, 10, 19, 28, 37, 47, 56, 70, 78, 85, 93, 100},   // B156XW04 V5 (0A)
	{0x02DC,  9, 17, 27, 35, 44, 54, 68, 76, 84, 92, 100},   // LP156WH4-TLA1
	{0x394A,  7, 14, 21, 29, 36, 43, 57, 68, 79, 90, 100},   // LTN156AT24-A01
	{0x035A, 10, 20, 30, 40, 50, 60, 70, 77, 84, 92, 100},   // LP156WH5-TJA1, LP156WH6-TJA1
	{0x10EC,  8, 16, 24, 34, 44, 54, 65, 73, 81, 89, 100},   // B156XTN01.0
	{0x15B4, 10, 18, 27, 36, 45, 55, 68, 76, 84, 92, 100},   // N156BGE-E21
	{0x0351,  9, 17, 27, 35, 44, 54, 68, 76, 84, 92, 100},   // LP156WH4-TPA1
	{0x038E, 10, 19, 28, 37, 47, 56, 71, 78, 85, 93, 100},   // LP156WH4-TLN2
	{0x0323,  7, 20, 33, 39, 49, 58, 67, 75, 84, 92, 100},   // LP156WF4-SPB1
	{0x039D, 10, 20, 30, 40, 50, 60, 70, 77, 84, 92, 100},   // LP156WH6-TJA2
	{0x0395, 10, 19, 28, 37, 47, 56, 71, 78, 85, 93, 100},   // LP156WH4-TLP2
	{0x15A3, 10, 18, 27, 36, 45, 55, 68, 76, 84, 92, 100},   // N156BGE-L11
	{0x12ED,  6, 17, 28, 33, 41, 51, 61, 71, 81, 91, 100},   // B156HAN01.2(0A)
	{0x03D4,  5, 15, 25, 30, 38, 48, 58, 68, 78, 89, 100},   // LP156WF4-SPD1
	{0x1400,  5, 17, 28, 34, 42, 52, 61, 71, 80, 90, 100},   // LQ156T1JW03
	{0x31ED,  8, 16, 20, 28, 34, 42, 55, 66, 77, 88, 100},   // B156HTN03.1
	{0x15C0,  7, 13, 20, 27, 34, 40, 50, 62, 75, 87, 100},   // N156HGE-EA1
	{0x48EC, 10, 15, 23, 31, 39, 47, 59, 71, 80, 90, 100},   // B156XW04 V8(2A)
	{0x47EC, 10, 15, 23, 31, 39, 47, 59, 71, 80, 90, 100},   // B156XW04 V7(2A)
	{0x03F8,  9, 18, 27, 36, 45, 54, 67, 75, 83, 91, 100},   // LP156WH3-TPS1
	{0x15BE,  9, 18, 26, 31, 44, 53, 68, 78, 88, 96, 100},   // N156BGE-E41
	{0x15BF,  9, 18, 26, 31, 44, 53, 68, 78, 88, 96, 100},   // N156BGE-E31

	// ABO LED 17 size panel
	{0x139E,  8, 15, 24, 31, 40, 48, 60, 70, 80, 90, 100},   // B173RW01 V3 (5A)
	{0x1711, 10, 18, 27, 36, 45, 55, 68, 76, 84, 92, 100},   // N173O6-L02
	{0x3051, 10, 19, 28, 38, 47, 58, 69, 77, 85, 93, 100},   // LTN173KT01-A01
	{0x0289,  8, 16, 24, 32, 38, 47, 59, 69, 79, 89, 100},   // LP173WD1-TLA3
	{0x129E,  8, 15, 24, 32, 40, 48, 60, 70, 80, 90, 100},   // B173RW01 V2 (5A)
	{0x1721, 10, 18, 27, 36, 45, 55, 68, 76, 84, 92, 100},   // N173FGE-L21
	{0x0340,  8, 17, 24, 33, 41, 49, 62, 72, 81, 90, 100},   // LP173WD1-TLA4

	// ABO CCFL 15 size panel
	{0x3945, 25, 32, 39, 46, 55, 64, 73, 80, 87, 95, 100},   // LTN154AT01-001 WxGA, G
	                                                         // LTN154AT01-A01 WXGA, AG
	{0x10EC, 25, 32, 39, 46, 55, 64, 73, 80, 87, 95, 100},   // B156XW01-V0, B156XW01-V.0
	{0x12EC, 25, 32, 39, 46, 55, 64, 73, 80, 87, 95, 100},   // B156XW01-V2
	{0x1558, 25, 32, 39, 46, 55, 64, 73, 80, 87, 95, 100},   // N156B3-L02
	{0x6301, 30, 36, 42, 48, 55, 64, 73, 80, 87, 95, 100},   // LP156WH1-TLA1
	{0x01C2, 30, 36, 42, 48, 55, 64, 73, 80, 87, 95, 100},   // LP156WH1-TLA3
	{0x14B5, 25, 32, 39, 46, 55, 64, 73, 80, 87, 95, 100},   // CLAA156WA11A
	{0xD801, 30, 36, 42, 48, 55, 64, 73, 80, 87, 95, 100},   // LP156WH1-TLC1
	{0x1599, 25, 32, 39, 46, 55, 64, 73, 80, 87, 95, 100},   // N156B3-L0B
#endif

};
