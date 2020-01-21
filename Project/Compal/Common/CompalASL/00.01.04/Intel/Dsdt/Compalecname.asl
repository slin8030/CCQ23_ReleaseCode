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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal’s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     -----------------------------------------------------------------------------
 1.00                Vanessa      Init version.
 1.01      14/11/05  Vanessa	  Updated OSTY for Window 10.
 1.02      15/06/15  Vanessa      Code Adjust for project used to define level name at CompalECNAME_PROJECT.ASL.
 1.03      15/11/10  Ivan         Add new EC namespace defintion offset 88h(SMBUS Bus Number Selection).
*/

//
// [COMMON ASL] EC RAM using EDK II
//

#include "OemASL.asl"
#include "CompalECMMAP.ASL"
/*
 EC RAM Range   EC Memory Map           Size         Description
 -------------  ----------------------  --------     ----------------------------------------------------
                0xFF000000-0xFF0007FF   0x800        For EC kernal used.
 0x00-0x59      0xFF000800-0xFF000859   0x5A         For Compal project used.
 0x5A-0xFF      0xFF00085A-0xFF0008FF   0xA5         For Compal common used.
                0xFF000900-                          Reserved.
*/
// EC RAM fields
OperationRegion(ERAM,EmbeddedControl, 0, 0xFF)
Field(ERAM, ByteAcc, Lock, Preserve)
{
#include "CompalECNAME_PROJECT.ASL"
        Offset(0x5A),
#if (ECRAM_MEMMAP == DISABLE)
        AASD, 8,        // ISCT sleep duration for EC wake up   ; 5Ah
#else
            , 8,        // Reserve                              ; 5Ah
#endif
            , 8,        // Reserve                              ; 5Bh
            , 8,        // Reserve                              ; 5Ch
        ENIB, 16,       // Ext_NameSpace_Index                  ; 5Dh
                        // Ext_NameSpace_Bank                   ; 5Eh
        ENDD, 8,        // Ext_NameSpace_Data                   ; 5Fh

        Offset(0x60),
        SMPR, 8,        // SMBus protocol register              ; 60h
        SMST, 8,        // SMBus status register                ; 61h
        SMAD, 8,        // SMBus address register               ; 62h
        SMCM, 8,        // SMBus command register               ; 63h
        SMD0, 0x100,    // SMBus data regs (32)                 ; 64h - 83h
        BCNT, 8,        // SMBus Block Count                    ; 84h
        SMAA, 24,       // SMBus Alarm Address                  ; 85h - 87h
        SMBN, 8,        // SMBUS Bus Number Selection           ; 88h

#if (ECRAM_MEMMAP == DISABLE)
        Offset(0x90),
        BMFN, 72,       // Battery Manufacture name             ; 90h - 98h
        BATD, 56,       // Battery Device name                  ; 99h - 9fh

        Offset(0xA0),
        AATL, 1,        // ISCT battery temperature limitation  ; A0h.0
        AACL, 1,        // ISCT battery capacity limitation     ; A0h.1
        AAST, 1,        // ISCT S0_ISCT status                  ; A0h.2
        AARW, 1,        // ISCT RTC wake status (ISCT 1.0 only) ; A0h.3
        AAEN, 1,        // ISCT support enable                  ; A0h.4
            , 1,        // Reserve                              ; A0h.5
        WKRN, 1,        // Wake Up Reason                       ; A0h.6-7
                        // 01: EC wake
                        // 10: WLAN/WWAN
            
        Offset(0xA1),
            , 1,        // Reserve                              ; A1h.0
        VIDO, 1,        // Video Out Button (1=Pressed)         ; A1h.1
        TOUP, 1,        // Touch Pad Button (0=On, 1=Off)       ; A1h.2

        // OS Shutdown Temp2
        Offset(0xA2),
        ODTS, 8,        // OS Shutdown Temp2 (DTS)              ; A2h

        // SYS_STATUS
        Offset(0xA3),
        OSTY, 4,        // OSTYPE :   0000- XP                  ; A3h.0-3
                        //            0001- Vista
                        //            0010- Linux / Android
                        //            0011- Win7
                        //            0100- Win8
                        //            0101- Win 8.1  
			//            0110- Win 10
            , 2,        // Reserve                              ; A3h.4-5
        ECRD, 1,        // ACPI EC driver ready (0=Not ready,1=Ready) ; A3h.6    
        ADPT, 1,        // AC Adapter (0=OffLine, 1=OnLine)     ; A3h.7

        // WAKEUP_ENABLE
        Offset(0xA4),
        PWAK, 1,        // PME Wake Enable(0=Disable, 1=Enable) ; A4h.0
        MWAK, 1,        // Modem Wake Enable(0/1=Disable/Enable); A4h.1
        LWAK, 1,        // LAN Wake Enable (0=Disable, 1=Enable); A4h.2
        RWAK, 1,        // RTC Wake Enable(0=DIsable,1=Enable)  ; A4h.3
        WWAK, 1,        // WLAN wake Enable (0=Disable,1=Enable); A4h.4
        UWAK, 1,        // USB WAKE(0=Disable, 1=Enable)        ; A4h.5
        KWAK, 1,        // Keyboard WAKE(0=Disable,1=Enable)    ; A4h.6
        TWAK, 1,        // TouchPad WAKE(0=Disable,1=Enable)    ; A4h.7

        // AC_OFF Source
        Offset(0xA5),
        CCAC, 1,        // Charge Current (1=AC OFF)            ; A5h.0
        AOAC, 1,        // Adapter OVP (1=AC OFF)               ; A5h.1
        BLAC, 1,        // Batt learning (1=AC OFF)             ; A5h.2
        PSRC, 1,        // Command (1=AC OFF)                   ; A5h.3
        BOAC, 1,        // Batt OVP (1=AC OFF)                  ; A5h.4
        LCAC, 1,        // Leak Current (1=AC OFF)              ; A5h.5
        AAAC, 1,        // Air Adapter (1=AC OFF)               ; A5h.6
        ACAC, 1,        // AC Off (1=AC OFF)                    ; A5h.7

        Offset(0xA6),
        S3ST, 1,        // System entry S3 State                : A6.0
        S3RM, 1,        // System resume from S3 State          : A6.1
        S4ST, 1,        // System entry S4 State                : A6.2
        S4RM, 1,        // System resume from S4 State          : A6.3
        S5ST, 1,        // System entry S5 State                : A6.4
        S5RM, 1,        // System resume from S5 State          : A6.5
        CSST, 1,        // System entry CS State                ; A6.6
        CSRM, 1,        // System resume from CS State          ; A6.7
        // Thermal function
        Offset(0xA7),
        OSTT, 8,        // OS Throttling Temp                   ; A7h
        OSST, 8,        // OS Shutdown Temp                     ; A8h
        THLT, 8,        // Throttling Temp                      ; A9h
        TCNL, 8,        // Tcontrol Temp                        ; AAh
        MODE, 1,        // Mode(0=Local, 1=Remote)              ; ABh.0
            , 2,        // Reserve                              ; ABh.1-2
        INIT, 1,        // INITOK(0/1=Controlled by OS/EC)      ; ABh.3
        FAN1, 1,        // FAN1 Active                          ; ABh.4
        FAN2, 1,        // FAN2 Active                          ; ABh.5
        FANT, 1,        // FAN Speed Time OK                    ; ABh.6
        SKNM, 1,        // Skin Mode (0/1=Skin Address 90/92)   ; ABh.7
        SDTM, 8,        // Shutdown Thermal Temperature         ; ACh

        FSSN, 4,        // Fan Speed Step Number                ; ADh.0-3
                        // 00 : Fan Off
                        // 01 : Fan On Speed 1
                        // 10 : Fan On Speed 2
                        // 11 : Fan On Speed 3
        FANU, 4,        // Machine Fan's number                 ; ADh.4-7

        PCVL, 6,        // Throttling Level                     ; AEh.0-5
                        // 000000 : No throttling
                        // 000001 : Clock throttling 12.5%
                        // 000010 : Clock throttling 25%
                        // 000011 : Clock throttling 37.5%
                        // 000100 : Clock throttling 50%
                        // 000101 : Clock throttling 62.5%
                        // 000110 : Clock throttling 75%
                        // 000111 : Clock throttling 87.5%
                        // 001000 : Clock throttling 100%
        SWTO, 1,        // SW Throttling (1=Active)             ; AEh.6
        TTHR, 1,        // HW (THRM#) Throttling (1=Active)     ; AEh.7

        TTHM, 1,        // TS_THERMAL(1:Throttling for thermal) ; AFh.0
        THTL, 1,        // Turbo_Mode( 1= Turbo support )       ; AFh.1
        CTDP, 1,        // ULV CPU power function support       ; AFh.2
        NPST, 5,        // Number of P-State level              ; AFh.3-7

        CTMP, 8,        // Current CPU Temperature              ; B0h
        CTML, 8,        // CPU local temperature                ; B1h
        SKTA, 8,        // Skin Temperature A                   ; B2h
        SKTB, 8,        // GPU Temperature                      ; B3h
        SKTC, 8,        // Skin Temperature C                   ; B4h
            , 8,        // Reserved                             ; B5h
        NTMP, 8,        // North Bridge Diode Temp              ; B6h

        // System Event & Status
        Offset(0xB7),
        APLE, 1,        // Airplane mode enable                 ; B7h.0
            , 1,        // Reserved                             ; B7h.1
            , 1,        // Reserved                             ; B7h.2
            , 1,        // Reserved                             ; B7h.3
        CTHL, 4,        // Clock throttling level               ; B7h.4-7
                        // 0 0 0 0  : Throttling 50%
	                      // 0 0 0 1  : Reserved
	                      // 0 0 1 0  : Throttling 12.50%
	                      // 0 0 1 1  : Throttling 18.75%
	                      // 0 1 0 0  : Throttling 25.0%
	                      // 0 1 0 1  : Throttling 31.25%
	                      // 0 1 1 0  : Throttling 37.5%
	                      // 0 1 1 1  : Throttling 43.75%
	                      // 1 0 0 0  : Throttling 50.0%
	                      // 1 0 0 1  : Throttling 56.25%
	                      // 1 0 1 0  : Throttling 62.5%
	                      // 1 0 1 1  : Throttling 68.75%
	                      // 1 1 0 0  : Throttling 75.0%
	                      // 1 1 0 1  : Throttling 81.25%
	                      // 1 1 1 0  : Throttling 87.5%
	                      // 1 1 1 1  : Throttling 93.75%

            , 1,        // Reserved                             ; B8h.0
        LIDF, 1,        // LID flag (1=Closed, 0=Opened)        ; B8h.1
        PMEE, 1,        // PME event (0=off, 1=On)              ; B8h.2
        PWBE, 1,        // Power button event (0=off, 1=On)     ; B8h.3
        RNGE, 1,        // Ring in event (0=off, 1=On)          ; B8h.4
        BTWE, 1,        // Bluetooth wake event (0=off, 1=On)   ; B8h.5
            , 2,        // Reserved                             ; B8h.6-7

        // Device function
        Offset(0xB9),
        BRTS, 8,        // Brightness Value                     ; B9h

        S35M, 1,        // S3.5 HotKey test mode                ; BAh.0
        S35S, 1,        // S3.5 function status                 ; BAh.1
            , 2,        // Reserved                             ; BAh.2-3
        FFEN, 1,        // IRST support bit (1= Support)        ; BAh.4
        FFST, 1,        // IRST status flag                     ; BAh.5
            , 2,        // Reserved                             ; BAh.6-7

        WLAT, 1,        // Wireless LAN (0=Inactive, 1=Active)  ; BBh.0
        BTAT, 1,        // BlueTooth (0=Inactive, 1=Active)     ; BBh.1
        WLEX, 1,        // Wireless LAN (0=Not Exist, 1=Exist)  ; BBh.2
        BTEX, 1,        // BlueTooth (0=Not Exist, 1=Exist)     ; BBh.3
        KLSW, 1,        // Kill Switch (0=Off, 1=On)            ; BBh.4
        WLOK, 1,        // Wireless LAN Initialize OK           ; BBh.5
        AT3G, 1,        // 3G (0=non-active, 1=active)          ; BBh.6
        EX3G, 1,        // 3G (0=Not Exist, 1=Exist)            ; BBh.7

        PJID, 8,        // Project ID                           ; BCh

        // For Thermal Utility
        CPUJ, 3,        // CPU Type                             ; BDh.0-2
                        // 00, Tj85,
                        // 01, Tj90,
                        // 02, Tj100,
                        // 03, Tj105,
                        // 04 - 07, Reserved
        CPNM, 3,        // CPU Core number                      ; BDh.3-5
                        // 00, Single Core
                        // 01, Dual Core
                        // 02, Quad Core
                        // 03 - 07, Reserved
        GATY, 2,        // VGA Type                             ; BDh.6-7
                        // 00, UMA
                        // 01. Discrete
                        // 02 - 03, Reserved

        // SmartSelector_State0
        Offset(0xBE),
        BOL0, 1,        // Batt0 (0=OffLine, 1=OnLine)          ; BEh.0
        BOL1, 1,        // Batt1 (0=OffLine, 1=OnLine)          ; BEh.1
            , 2,        // Reserved                             ; BEh.2-3
        BCC0, 1,        // Batt0 be charging (1=Charging)       ; BEh.4
        BCC1, 1,        // Batt1 be charging (1=Charging)       ; BEh.5
            , 2,        // Reserved                             ; BEh.6-7

        // SmartSelector_State1
        BPU0, 1,        // Batt0 (1=PowerUp)                    ; BFh.0
        BPU1, 1,        // Batt1 (1=PowerUp)                    ; BFh.1
            , 2,        // Reserved                             ; BFh.2-3
        BOS0, 1,        // Batt0 (1=OnSMBUS)                    ; BFh.4
        BOS1, 1,        // Batt1 (1=OnSMBUS)                    ; BFh.5
            , 2,        // Reserved                             ; BFh.6-7

        Offset(0xC0),
        BTY0, 1,        // 1st Batt Type (0=NiMh, 1=LiIon)      ; C0h.0
        BAM0, 1,        // Battery mode (0=mW, 1=mA)            ; C0h.1
        BAL0, 1,        // Low Battery                          ; C0h.2
            , 1,        // Reserved                             ; C0h.3
        BMF0, 3,        // 1st Battery Manufacturer             ; C0h.4-6
                        // 001 : Sanyo
                        // 010 : Sony
                        // 100 : Pansonic
                        // 101 : CPT
            , 1,        // Reserved                             ; C0h.7

        BST0, 8,        // 1st Battery Status                   ; C1h
                        // Bit0 : Discharging
                        // Bit1 : Charging
                        // Bit2 : Discharg and Critical Low
                        // Bit3-7 : Reserved

        BRC0, 16,       // 1st Batt Remaining Capacity          ; C2h, C3h
        BSN0, 16,       // 1st Batt Serial Number               ; C4h, C5h
        BPV0, 16,       // 1st Batt Present Voltage             ; C6h, C7h
        BDV0, 16,       // 1st Batt Design Voltage              ; C8h, C9h
        BDC0, 16,       // 1st Batt Design Capacity             ; CAh, CBh
        BFC0, 16,       // 1st Batt Last Full Charge Capacity   ; CCh, CDh
        GAU0, 8,        // 1st Batt Gasgauge                    ; CEh
        CYC0, 8,        // 1st Batt Cycle Counter               ; CFh
        BPC0, 16,       // 1st Batt Current                     ; D0h, D1h
        BAC0, 16,       // 1st Batt Average Current             ; D2h, D3h
        BTW0, 8,        // 1st Batt Comsuption                  ; D4h
        BVL0, 8,        // 1st Batt Battery Volt                ; D5h
        BTM0, 8,        // 1st Batt Battery Temp                ; D6h
        BAT0, 8,        // 1st Batt Average Temp (Degree C)     ; D7h
        BCG0, 16,       // 1st Batt charge current              ; D8h, D9h
        BCT0, 8,        // 1st Batt Current Temp Semple counter ; DAh
        BCI0, 8,        // 1st BATT CMD Index for read BATT(SMB); DBh
        BCM0, 8,        // Count up to Communicate 1st BATT     ; DCh
        BOT0, 8,        // Count up if 1st BATT over Temp       ; DDh
        BSSB, 16,       // 1st BATT Battery Status SMB	        ; DEh, DFh

        BOV0, 8,        // 1st BATT Over Voltage Count          ; E0h
        BCF0, 8,        // 1st BATT Communication Fail Counter  ; E1h
        BAD0, 8,        // Battery Voltage of ADC               ; E2h
        BCV1, 16,       // Cell Voltage 1 (mV)                  ; E3h, E4h
        BCV2, 16,       // Cell Voltage 2 (mV)                  ; E5h, E6h
        BCV3, 16,       // Cell Voltage 3 (mV)                  ; E7h, E8h
        BCV4, 16,       // Cell Voltage 4 (mV)                  ; E9h, EAh

        Offset(0xED),
        BFCB, 16,       // 1st Batt Last FCC caught by OS       ; EDh, EEh

        Offset(0xF1),
            , 6,        // Reserved for EC
        ORRF, 1,        // OS RC redirection flag(Fix FCC issue); F1h.6

        Offset(0xF4),
        BMD0, 16,       // Manufacture Date                     ; F4h, F5h
                        // 1st Batt Day	                        ; BIT[4:0] (Day)
                        // 1st Batt Month                       ; BIT[9:5] (Month)
                        // 1st Batt Year                        ; BIT[15:10] (Year)
        BACV, 16,       // Charging Voltage                     ; F6h, F7h
        BDN0, 8,        // 1st Battery Cell Number              ; F8h
        BTPB, 16,       // Next battery trip point              ; F9h, FAh

#endif

            , 8         // Last byte for Reserved.              ; Reserved last byte for ASL code complier.

}


