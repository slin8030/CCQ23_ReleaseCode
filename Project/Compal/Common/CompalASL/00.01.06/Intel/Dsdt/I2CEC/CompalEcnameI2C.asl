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
 1.00                             Init version.
 1.01      16/08/01  Vanessa      Add new temperature of sensor definition in 90h/91h
 1.02      16/08/31  Thomas       Add a new flag for FFCC feature (F1h.6)

*/
//================================================================
// The following I2C_CMD_for WOA v0.2 define EC Namespace Region
// Need to use C080 and C081 to R/W Namespace.
//================================================================
// Namespace 2nd Battery
#define ECNAME_BMS0 0x30                        // 2nd Manufacture name byte 0                          ; 30h
#define ECNAME_BMS1 0x31                        // 2nd Manufacture name byte 1                          ; 31h
#define ECNAME_BMS2 0x32                        // 2nd Manufacture name byte 2                          ; 32h
#define ECNAME_BMS3 0x33                        // 2nd Manufacture name byte 3                          ; 33h
#define ECNAME_BMS4 0x34                        // 2nd Manufacture name byte 4                          ; 34h
#define ECNAME_BMS5 0x35                        // 2nd Manufacture name byte 5                          ; 35h
#define ECNAME_BMS6 0x36                        // 2nd Manufacture name byte 6                          ; 36h
#define ECNAME_BMS7 0x37                        // 2nd Manufacture name byte 7                          ; 37h
#define ECNAME_BMS8 0x38                        // 2nd Manufacture name byte 8                          ; 38h
#define ECNAME_BDS0 0x39                        // 2nd Device name byte 0                               ; 39h
#define ECNAME_BDS1 0x3A                        // 2nd Device name byte 1                               ; 3Ah
#define ECNAME_BDS2 0x3B                        // 2nd Device name byte 2                               ; 3Bh
#define ECNAME_BDS3 0x3C                        // 2nd Device name byte 3                               ; 3Ch
#define ECNAME_BDS4 0x3D                        // 2nd Device name byte 4                               ; 3Dh
#define ECNAME_BDS5 0x3E                        // 2nd Device name byte 5                               ; 3Eh
#define ECNAME_BDS6 0x3F                        // 2nd Device name byte 6                               ; 3Fh               
#define ECNAME_BAM1 0x40                        // 2nd Battery type (0=Nimh, 1=Li)                      ; 40h.0
                                                // 2nd Battery mode (0=mAh, 1=10mWh)                    ; 40h.1
                                                // 2nd Battery low                                      ; 40h.2
#define ECNAME_BST1 0x41                        // 2nd BatteryBattery Status                            ; 41h
                                                // Bit0 : Discharging
                                                // Bit1 : Charging
                                                // Bit2 : Discharg and Critical Low
                                                // Bit3-7 : Reserved
#define ECNAME_BRC2 0x42                        // 2nd Batt Remaining Capacity                          ; 42h
#define ECNAME_BRC3 0x43                        // 2nd Batt Remaining Capacity                          ; 43h
#define ECNAME_BSN2 0x44                        // 2nd Batt Serial Number                               ; 44h
#define ECNAME_BSN3 0x45                        // 2nd Batt Serial Number                               ; 45h
#define ECNAME_BPV2 0x46                        // 2nd Batt Present Voltage                             ; 46h
#define ECNAME_BPV3 0x47                        // 2nd Batt Present Voltage                             ; 47h
#define ECNAME_BDV2 0x48                        // 2nd Batt Design Voltage                              ; 48h
#define ECNAME_BDV3 0x49                        // 2nd Batt Design Voltage                              ; 49h
#define ECNAME_BDC2 0x4A                        // 2nd Batt Design Capacity                             ; 4Ah
#define ECNAME_BDC3 0x4B                        // 2nd Batt Design Capacity                             ; 4Bh
#define ECNAME_BFC2 0x4C                        // 2nd Batt Last Full Charge Capacity                   ; 4Ch
#define ECNAME_BFC3 0x4D                        // 2nd Batt Last Full Charge Capacity                   ; 4Dh
#define ECNAME_GAS1 0x4E                        // Gasguage                                             ; 4Eh
#define ECNAME_BCC1 0x4F                        // Battery cycle counter                                ; 4Fh
#define ECNAME_BCT2 0x50                        // Battery current(Low byte)                            ; 50h
#define ECNAME_BCT3 0x51                        // Battery current(High byte)                           ; 51h
#define ECNAME_BAC2 0x52                        // 2nd Batt Average Current(Low byte)                   ; 52h
#define ECNAME_BAC3 0x53                        // 2nd Batt Average Current(High byte)                  ; 53h
#define ECNAME_BPC1 0x54                        // 2nd Batt Power consumption                           ; 54h
#define ECNAME_BVL1 0x55                        // 2nd Batt voltage                                     ; 55h
#define ECNAME_BTS1 0x56                        // 2nd Battery TS temperature                           ; 56h
#define ECNAME_AVT1 0x57                        // 2nd Battery Average temperature                      ; 57h
#define ECNAME_BCU2 0x58                        // 2nd Battery Charge Current(Low byte)                 ; 58h
#define ECNAME_BCU3 0x59                        // 2nd Battery Charge Current(High byte)                ; 59h
#define ECNAME_BSC1 0x5A                        // 2nd Battery current temperature sample count         ; 5Ah
#define ECNAME_BCT5 0x5B                        // 2nd Battery command index for read battery           ; 5Bh
                                                // through SMBus		
#define ECNAME_BCU6 0x5C                        // 2nd Battery count up to communicate battery          ; 5Ch
#define ECNAME_BCU7 0x5D                        // 2nd Battery count up if battery is arrived           ; 5Dh
                                                // over temperature	
#define ECNAME_BSS2 0x5E                        // 2nd Battery SMBus status(Low Byte)                   ; 5Eh
#define ECNAME_BSS3 0x5F                        // 2nd Battery SMBus status(High Byte)                  ; 5Fh
#define ECNAME_BOV1 0x60                        // 2nd Battery over voltage counter                     ; 60h
#define ECNAME_BCF1 0x61                        // 2nd communication fail counter                       ; 61h
#define ECNAME_BAD1 0x62                        // 2nd Battery voltage of ADC                           ; 62h
#define ECNAME_BCV8 0x63                        // 2nd cell1 voltage(Low byte)                          ; 63h
#define ECNAME_BCV9 0x64                        // 2nd cell1 voltage(High byte)                         ; 64h
#define ECNAME_BCVA 0x65                        // 2nd cell2 voltage(Low byte)                          ; 65h
#define ECNAME_BCVB 0x66                        // 2nd cell2 voltage(High byte)                         ; 66h
#define ECNAME_BCVC 0x67                        // 2nd cell3 voltage(Low byte)                          ; 67h
#define ECNAME_BCVD 0x68                        // 2nd cell3 voltage(High byte)                         ; 68h
#define ECNAME_BCVE 0x69                        // 2nd cell4 voltage(Low byte)                          ; 69h
#define ECNAME_BCVF 0x6A                        // 2nd cell4 voltage(High byte)                         ; 6Ah
#define ECNAME_BMA2 0x6B                        // 2nd manufacture Access(Low byte)                     ; 6Bh
#define ECNAME_BMA3 0x6C                        // 2nd manufacture Access(High byte)                    ; 6Ch												
#define ECNAME_GTT2 0x6D                        // 2nd Get Time to full(Minute)                         ; 6Dh
#define ECNAME_GTT3 0x6E                        // 2nd Get Time to full(Minute)                         ; 6Eh
#define ECNAME_BAS1 0x70                        // 2nd Battery exist                                    ; 70h.0
                                                // 2nd Battery full                                     ; 70h.1
                                                // 2nd Battery empty                                    ; 70h.2
                                                // 2nd Battery first in                                 ; 70h.3
                                                // 2nd Battery read complete                            ; 70h.6
#define ECNAME_BCN1 0x78                        // 2nd Battery Cell Number                              ; 78h
                                                //     2 cell                                           ; 78h.0
                                                //     4 cell                                           ; 78h.1
#define ECNAME_BYC2 0x79                        // 2nd Battery Cycle counter(Low byte)                  ; 79h
#define ECNAME_BYC3 0x7A                        // 2nd Battery Cycle counter(High byte)                 ; 7Ah												
#define ECNAME_BFF2 0x7B                        // 2nd Batt Last FCC caught by OS (Low Byte)            ; 7Bh 
#define ECNAME_BFF3 0x7C                        // 2nd Batt Last FCC caught by OS (High Byte)           ; 7Ch
#define ECNAME_BTP2 0x7D                        // Next battery trip point (Low Byte)                   ; 7Dh
#define ECNAME_BTP3 0x7E                        // Next battery trip point (Low Byte)                   ; 7Eh
#define ECNAME_ERR1 0x7F                        // MaxError                                             ; 7Fh

// Namespace normal

#define ECNAME_STP1 0x90                        // Temperature of sensor1                               ; 90h
#define ECNAME_STP2 0x91                        // Temperature of sensor2                               ; 91h

#define ECNAME_ECOK 0xA1                        // EC Namespace accessible (EC will always return 0xA5) ; A1h
#define ECNAME_BOL0 0xA2                        // Batt 1st(Bit0 0=OffLine, 1=OnLine)                   ; A2h.0
                                                // Batt 2nd(Bit0 0=OffLine, 1=OnLine)                   ; A2h.0
#define ECNAME_LWAK 0xA7                        // Lan wake(Bit0 0=don't keep, 1=keep power             ; A3h.0
#define ECNAME_UWAK 0xA7                        // Usb wake(Bit0 0=don't keep, 1=keep power             ; A3h.1
#define ECNAME_SYSS 0xA4                        // Bit0:4 Reserved                                      ; A4h.3,4,5,6,7
                                                // HP                                                   ; A4h.3
                                                // Dock                                                 ; A4h.4
                                                // P-Sensor status ( 1: Active, 0: Inactive )           ; A4h.5
                                                // LID flag (Bit6 0=un-pressded, 1=pressed)             ; A4h.6
                                                // AC Adapter (Bit7 0=OffLine, 1=OnLine)                ; A4h.7
#define ECNAME_S3ST 0xA6                        // System entry S3 State                                ; A5h.0
#define ECNAME_S3RM 0xA6                        // System resume from S3 State                          ; A5h.1
#define ECNAME_S4ST 0xA6                        // System entry S4 State                                ; A5h.2
#define ECNAME_S4RM 0xA6                        // System resume from S4 State                          ; A5h.3
#define ECNAME_S5ST 0xA6                        // System entry S5 State                                ; A5h.4
#define ECNAME_S5RM 0xA6                        // System resume from S5 State                          ; A5h.5
#define ECNAME_CSST 0xA6                        // System entry CS State                                ; A5h.6
#define ECNAME_CSRM 0xA6                        // System resume from S5 State                          ; A5h.7
#define ECNAME_PCVL 0xAE                        // Throttling Level                                     ; AEh.0-5

												
// Namespace 1st Battery												
#define ECNAME_BMF0 0xB0                        // Manufacture name byte 0                              ; B0h
#define ECNAME_BMF1 0xB1                        // Manufacture name byte 1                              ; B1h
#define ECNAME_BMF2 0xB2                        // Manufacture name byte 2                              ; B2h
#define ECNAME_BMF3 0xB3                        // Manufacture name byte 3                              ; B3h
#define ECNAME_BMF4 0xB4                        // Manufacture name byte 4                              ; B4h
#define ECNAME_BMF5 0xB5                        // Manufacture name byte 5                              ; B5h
#define ECNAME_BMF6 0xB6                        // Manufacture name byte 6                              ; B6h
#define ECNAME_BMF7 0xB7                        // Manufacture name byte 7                              ; B7h
#define ECNAME_BMF8 0xB8                        // Manufacture name byte 8                              ; B8h
#define ECNAME_BDN0 0xB9                        // Device name byte 0                                   ; B9h
#define ECNAME_BDN1 0xBA                        // Device name byte 1                                   ; BAh
#define ECNAME_BDN2 0xBB                        // Device name byte 2                                   ; BBh
#define ECNAME_BDN3 0xBC                        // Device name byte 3                                   ; BCh
#define ECNAME_BDN4 0xBD                        // Device name byte 4                                   ; BDh
#define ECNAME_BDN5 0xBE                        // Device name byte 5                                   ; BEh
#define ECNAME_BDN6 0xBF                        // Device name byte 6                                   ; BFh
#define ECNAME_BAM0 0xC0                        // Battery type (0=Nimh, 1=Li)                          ; C0h.0
                                                // Battery mode (0=mAh, 1=10mWh)                        ; C0h.1
                                                // Battery low                                          ; C0h.2
#define ECNAME_BST0 0xC1                        // 1st BatteryBattery Status                            ; C1h
                                                // Bit0 : Discharging
                                                // Bit1 : Charging
                                                // Bit2 : Discharg and Critical Low
                                                // Bit3-7 : Reserved
#define ECNAME_BRC0 0xC2                        // 1st Batt Remaining Capacity                          ; C2h
#define ECNAME_BRC1 0xC3                        // 1st Batt Remaining Capacity                          ; C3h
#define ECNAME_BSN0 0xC4                        // 1st Batt Serial Number                               ; C4h
#define ECNAME_BSN1 0xC5                        // 1st Batt Serial Number                               ; C5h
#define ECNAME_BPV0 0xC6                        // 1st Batt Present Voltage                             ; C6h
#define ECNAME_BPV1 0xC7                        // 1st Batt Present Voltage                             ; C7h
#define ECNAME_BDV0 0xC8                        // 1st Batt Design Voltage                              ; C8h
#define ECNAME_BDV1 0xC9                        // 1st Batt Design Voltage                              ; C9h
#define ECNAME_BDC0 0xCA                        // 1st Batt Design Capacity                             ; CAh
#define ECNAME_BDC1 0xCB                        // 1st Batt Design Capacity                             ; CBh
#define ECNAME_BFC0 0xCC                        // 1st Batt Last Full Charge Capacity                   ; CCh
#define ECNAME_BFC1 0xCD                        // 1st Batt Last Full Charge Capacity                   ; CDh
#define ECNAME_GAS0 0xCE                        // Gasguage                                             ; CEh
#define ECNAME_BCC0 0xCF                        // Battery cycle counter                                ; CFh
#define ECNAME_BCT0 0xD0                        // Battery current                                      ; D0h
#define ECNAME_BCT1 0xD1                        // Battery current                                      ; D1h
#define ECNAME_BAC0 0xD2                        // 1st Batt Average Current                             ; D2h
#define ECNAME_BAC1 0xD3                        // 1st Batt Average Current                             ; D3h
#define ECNAME_BPC0 0xD4                        // Battery power consumption                            ; D4h
#define ECNAME_BVL0 0xD5                        // Battery voltage                                      ; D5h
#define ECNAME_BTS0 0xD6                        // Battery TS temperature                               ; D6h
#define ECNAME_AVT0 0xD7                        // Battery average temperature                          ; D7h
#define ECNAME_BCU0 0xD8                        // Battery Charge Current(Low byte)                     ; D8h
#define ECNAME_BCU1 0xD9                        // Battery Charge Current(High byte)                    ; D9h
#define ECNAME_BSC0 0xDA                        // Battery current temperature sample count             ; DAh
#define ECNAME_BCT4 0xDB                        // Battery command index for read battery               ; DBh
                                                // through SMBus		
#define ECNAME_BCU4 0xDC                        // Battery count up to communicate battery              ; DCh
#define ECNAME_BCU5 0xDD                        // Battery count up if battery is arrived               ; DDh
                                                // over temperature	
#define ECNAME_BSS0 0xDE                        // Battery SMBus status(Low Byte)                       ; DEh
#define ECNAME_BSS1 0xDF                        // Battery SMBus status(High Byte)                      ; DFh
#define ECNAME_BOV0 0xE0                        // Battery over voltage counter                         ; E0h
#define ECNAME_BCF0 0xE1                        // communication fail counter                           ; E1h
#define ECNAME_BAD0 0xE2                        // Battery voltage of ADC                               ; E2h
#define ECNAME_BCV0 0xE3                        // cell1 voltage(Low byte)                              ; E3h
#define ECNAME_BCV1 0xE4                        // cell1 voltage(High byte)                             ; E4h
#define ECNAME_BCV2 0xE5                        // cell2 voltage(Low byte)                              ; E5h
#define ECNAME_BCV3 0xE6                        // cell2 voltage(High byte)                             ; E6h
#define ECNAME_BCV4 0xE7                        // cell3 voltage(Low byte)                              ; E7h
#define ECNAME_BCV5 0xE8                        // cell3 voltage(High byte)                             ; E8h
#define ECNAME_BCV6 0xE9                        // cell4 voltage(Low byte)                              ; E9h
#define ECNAME_BCV7 0xEA                        // cell4 voltage(High byte)                             ; EAh
#define ECNAME_BMA0 0xEB                        // manufacture Access(Low byte)                         ; EBh
#define ECNAME_BMA1 0xEC                        // manufacture Access(High byte)                        ; ECh
#define ECNAME_GTT0 0xED                        // Get Time to full(Minute)                             ; EDh
#define ECNAME_GTT1 0xEE                        // Get Time to full(Minute)                             ; EEh
#define ECNAME_BAS0 0xF0                        // Battery exist                                        ; F0h.0
                                                // Battery full                                         ; F0h.1
                                                // Battery empty                                        ; F0h.2
                                                // Battery first in                                     ; F0h.3
                                                // Battery read complete                                ; F0h.6
#define ECNAME_ORRF 0xF1                        // Battery FFCC                                         ; F1h.6        
#define ECNAME_BCN0 0xF8                        // 1st Battery Cell Number                              ; F8h
                                                //     2 cell                                           ; F8h.0
                                                //     4 cell                                           ; F8h.1
#define ECNAME_BYC0 0xF9                        // 1ST Battery Cycle counter(Low byte)                  ; F9h
#define ECNAME_BYC1 0xFA                        // 1ST Battery Cycle counter(High byte)                 ; FAh												
#define ECNAME_BFF0 0xFB                        // 1st Batt Last FCC caught by OS (Low Byte)            ; FBh 
#define ECNAME_BFF1 0xFC                        // 1st Batt Last FCC caught by OS (High Byte)           ; FCh
#define ECNAME_BTP0 0xFD                        // Next battery trip point (Low Byte)                   ; FDh
#define ECNAME_BTP1 0xFE                        // Next battery trip point (Low Byte)                   ; FEh
#define ECNAME_ERR0 0xFF                        // MaxError                                             ; FFh

Scope(COMMON_ASL_EC_PATH){
        Name(ENE3, ResourceTemplate()
        {
          I2CSerialBus ( IO373x_ADDR,, IO373x_I2C_SPEED,, IO373x_ResourceSource,,,,)
        })

  // This AVBL is redefine on batteryUlpmc.asl
  // If we want to disable Intel ULPMC code, it will enable AVBL function as below.  
  // Track status of I2C OpRegion availability for this controller
  Name(AVBL, 0)
  Method(_REG,2) {
    If (Lequal(Arg0, 9)) {
      Store(Arg1, ^AVBL)
      }
  }
		
    OperationRegion(ERAM, GenericSerialBus, 0x0, 0x100)
	  Field(ERAM, BufferAcc, NoLock, Preserve)
 	 {
       Connection(ENE3),
       Offset(0x01),
       AccessAs(BufferAcc, AttribBytes(0x06)),          // Use the GenericSerialBus Read/Write Bytes protocol  
       I2CR, 8,     // Read EC data Command
       AccessAs(BufferAcc, AttribBytes(0x05)),          // Use the GenericSerialBus Read/Write Bytes protocol  
       I2CW, 8,     // Write EC data Command
  }
}// End Scope(\_SB.I2C1)
