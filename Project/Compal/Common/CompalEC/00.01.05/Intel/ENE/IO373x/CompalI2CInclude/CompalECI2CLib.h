/*
 * (C) Copyright 2013-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2013-2020 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
 R E V I S I O N    H I S T O R Y
 
 Ver       Date       Who          Change           Support Chipset
 --------  --------   ------------ ---------------- -----------------------------------
 1.00      13/8/22    Jeter_Wang   Init version.     BTT/BSW/CHT
*/


#ifndef _Compal_EC_I2CLib_H_
#define _Compal_EC_I2CLib_H_

#include <Uefi.h>
#include <Library/IoLib.h>

//
// FIFO write workaround value.
//
#define FIFO_WRITE_DELAY    2

//
// MMIO Register Definitions
//
#define    R_IC_CON                          ( 0x00) // I2C Control  
#define     B_IC_RESTART_EN                  BIT5
#define     B_IC_SLAVE_DISABLE               BIT6
#define     V_SPEED_STANDARD                 0x02
#define     V_SPEED_FAST                     0x04
#define     V_SPEED_HIGH                     0x06
#define     B_MASTER_MODE                    BIT0

#define    R_IC_TAR                          ( 0x04) // I2C Target Address
#define     IC_TAR_10BITADDR_MASTER           BIT12

#define    R_IC_SAR                          ( 0x08) // I2C Slave Address
#define    R_IC_HS_MADDR                     ( 0x0C) // I2C HS MasterMode Code Address
#define    R_IC_DATA_CMD                     ( 0x10) // I2C Rx/Tx Data Buffer and Command

#define    B_READ_CMD                         BIT8    // 1 = read, 0 = write
#define    B_CMD_STOP                         BIT9    // 1 = STOP
#define    B_CMD_RESTART                      BIT10   // 1 = IC_RESTART_EN

#define    V_WRITE_CMD_MASK                  ( 0xFF)

#define    R_IC_SS_SCL_HCNT                  ( 0x14) // Standard Speed I2C Clock SCL High Count
#define    R_IC_SS_SCL_LCNT                  ( 0x18) // Standard Speed I2C Clock SCL Low Count
#define    R_IC_FS_SCL_HCNT                  ( 0x1C) // Full Speed I2C Clock SCL High Count
#define    R_IC_FS_SCL_LCNT                  ( 0x20) // Full Speed I2C Clock SCL Low Count
#define    R_IC_HS_SCL_HCNT                  ( 0x24) // High Speed I2C Clock SCL High Count
#define    R_IC_HS_SCL_LCNT                  ( 0x28) // High Speed I2C Clock SCL Low Count
#define    R_IC_INTR_STAT                    ( 0x2C) // I2C Inetrrupt Status
#define    R_IC_INTR_MASK                    ( 0x30) // I2C Interrupt Mask
#define     I2C_INTR_GEN_CALL                 BIT11  // General call received
#define     I2C_INTR_START_DET                BIT10
#define     I2C_INTR_STOP_DET                 BIT9
#define     I2C_INTR_ACTIVITY                 BIT8
#define     I2C_INTR_TX_ABRT                  BIT6   // Set on NACK
#define     I2C_INTR_TX_EMPTY                 BIT4
#define     I2C_INTR_TX_OVER                  BIT3
#define     I2C_INTR_RX_FULL                  BIT2   // Data bytes in RX FIFO over threshold
#define     I2C_INTR_RX_OVER                  BIT1
#define     I2C_INTR_RX_UNDER                 BIT0
#define    R_IC_RAW_INTR_STAT                ( 0x34) // I2C Raw Interrupt Status
#define    R_IC_RX_TL                        ( 0x38) // I2C Receive FIFO Threshold
#define    R_IC_TX_TL                        ( 0x3C) // I2C Transmit FIFO Threshold
#define    R_IC_CLR_INTR                     ( 0x40) // Clear Combined and Individual Interrupts
#define    R_IC_CLR_RX_UNDER                 ( 0x44) // Clear RX_UNDER Interrupt
#define    R_IC_CLR_RX_OVER                  ( 0x48) // Clear RX_OVERinterrupt
#define    R_IC_CLR_TX_OVER                  ( 0x4C) // Clear TX_OVER interrupt
#define    R_IC_CLR_RD_REQ                   ( 0x50) // Clear RD_REQ interrupt
#define    R_IC_CLR_TX_ABRT                  ( 0x54) // Clear TX_ABRT interrupt
#define    R_IC_CLR_RX_DONE                  ( 0x58) // Clear RX_DONE interrupt
#define    R_IC_CLR_ACTIVITY                 ( 0x5C) // Clear ACTIVITY interrupt
#define    R_IC_CLR_STOP_DET                 ( 0x60) // Clear STOP_DET interrupt
#define    R_IC_CLR_START_DET                ( 0x64) // Clear START_DET interrupt
#define    R_IC_CLR_GEN_CALL                 ( 0x68) // Clear GEN_CALL interrupt
#define    R_IC_ENABLE                       ( 0x6C) // I2C Enable
#define    R_IC_STATUS                       ( 0x70) // I2C Status

#define    R_IC_SDA_HOLD                     ( 0x7C) // I2C IC_DEFAULT_SDA_HOLD//16bits

#define     STAT_MST_ACTIVITY                 BIT5   // Master FSM Activity Status.
#define     STAT_RFF                          BIT4   // RX FIFO is completely full
#define     STAT_RFNE                         BIT3   // RX FIFO is not empty
#define     STAT_TFE                          BIT2   // TX FIFO is completely empty
#define     STAT_TFNF                         BIT1   // TX FIFO is not full

#define    R_IC_TXFLR                        ( 0x74) // Transmit FIFO Level Register
#define    R_IC_RXFLR                        ( 0x78) // Receive FIFO Level Register
#define    R_IC_TX_ABRT_SOURCE               ( 0x80) // I2C Transmit Abort Status Register
#define    R_IC_SLV_DATA_NACK_ONLY           ( 0x84) // Generate SLV_DATA_NACK Register
#define    R_IC_DMA_CR                       ( 0x88) // DMA Control Register
#define    R_IC_DMA_TDLR                     ( 0x8C) // DMA Transmit Data Level
#define    R_IC_DMA_RDLR                     ( 0x90) // DMA Receive Data Level
#define    R_IC_SDA_SETUP                    ( 0x94) // I2C SDA Setup Register
#define    R_IC_ACK_GENERAL_CALL             ( 0x98) // I2C ACK General Call Register
#define    R_IC_ENABLE_STATUS                ( 0x9C) // I2C Enable Status Register
#define    R_IC_COMP_PARAM                   ( 0xF4) // Component Parameter Register
#define    R_IC_COMP_VERSION                 ( 0xF8) // Component Version ID
#define    R_IC_COMP_TYPE                    ( 0xFC) // Component Type

#define    R_IC_CLK_GATE                     ( 0xC0) // Clock Gate

#define I2C_SS_SCL_HCNT_VALUE_100M               0x1DD                
#define I2C_SS_SCL_LCNT_VALUE_100M               0x1E4             
#define I2C_FS_SCL_HCNT_VALUE_100M               0x54                
#define I2C_FS_SCL_LCNT_VALUE_100M               0x9a               
#define I2C_HS_SCL_HCNT_VALUE_100M               0x7                 
#define I2C_HS_SCL_LCNT_VALUE_100M               0xE           

#define     IC_TAR_10BITADDR_MASTER           BIT12
#define     FIFO_SIZE                         32
#define     R_IC_INTR_STAT                    ( 0x2C) // I2c Inetrrupt Status
#define     R_IC_INTR_MASK                    ( 0x30) // I2c Interrupt Mask
#define     I2C_INTR_GEN_CALL                 BIT11  // General call received
#define     I2C_INTR_START_DET                BIT10
#define     I2C_INTR_STOP_DET                 BIT9
#define     I2C_INTR_ACTIVITY                 BIT8
#define     I2C_INTR_TX_ABRT                  BIT6   // Set on NACK
#define     I2C_INTR_TX_EMPTY                 BIT4
#define     I2C_INTR_TX_OVER                  BIT3
#define     I2C_INTR_RX_FULL                  BIT2   // Data bytes in RX FIFO over threshold
#define     I2C_INTR_RX_OVER                  BIT1
#define     I2C_INTR_RX_UNDER                 BIT0

EFI_BOOT_SERVICES  *gBS;



EFI_STATUS CompalByteReadI2C_Serial(
  IN  UINT8 BusNo,
  IN  UINT8 SlaveAddress, 
  IN  UINTN ReadBytes,
  OUT UINT8 *ReadBuffer
);

EFI_STATUS CompalByteWriteI2C_Serial(
    IN  UINT8 BusNo, 
    IN  UINT8 SlaveAddress,
    IN  UINTN WriteBytes,
    IN  UINT8 *WriteBuffer,
    IN  UINT8 Mode    
);

#endif  // _Compal_EC_I2CLib_H_
