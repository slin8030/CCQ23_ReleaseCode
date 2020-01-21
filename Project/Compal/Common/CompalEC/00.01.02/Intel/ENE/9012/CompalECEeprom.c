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

#include <CompalECLib.h>

UINT8
CompalECEepromReadByte (
    IN    UINT16    DataIndex
)
{
    UINT8     Byte=0;

	CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_BANK_NUM,(UINT8)(DataIndex >> 8));
    Byte = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_READ,(UINT8)DataIndex);
    return Byte;
}


VOID
CompalECEepromWriteByte (
    IN    UINT16    EepromOffset,
    IN    UINT8     WriteData
)
{
    UINT8     Byte;
    UINT8     RetryCount;

    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_BANK_NUM,(UINT8)(EepromOffset >> 8));

    for (RetryCount =0; RetryCount < EC_CMD_EEPROM_RETRYCOUNT; RetryCount++) {
        CompalECWriteCmdTwoByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_WRITE,(UINT8)EepromOffset,WriteData);
        Byte = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_READ,(UINT8)EepromOffset);
        if ( Byte == WriteData ) {
            break;
        }
    }
}

UINT8
CompalECGetEepromSlaveAdd (
    VOID
)
{
    UINT8    Eeprom_Smbus_Slave_Addr;

    Eeprom_Smbus_Slave_Addr = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT, EC_GET_SLAVE_ADDR_CMD, EC_GET_SLAVE_ADDR_DATA);
    return Eeprom_Smbus_Slave_Addr;
}
