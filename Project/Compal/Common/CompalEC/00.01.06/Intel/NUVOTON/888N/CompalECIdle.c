/*
 * (C) Copyright 2011-2014 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2014 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <CompalECLib.h>

#define EC_RAM_BASE_0xF554 0x10954
#define EC_RAM_BASE_0xFF10 0xFF10
#define EC_RAM_BASE_0xFF14 0xFF14

EFI_STATUS
CompalECWait (
    IN  BOOLEAN     EnableWrites
)
{
    //
    // If EC is in Idle mode, do nothing
    //
    if (EnableWrites == TRUE) {
        //
        // If EC in Wait Mode
        //

        //
        // Let EC into Wait
        //
        CompalECWaitMode();

        //
        // Check EC RAM 0xF554 = 33 , If no, keep loop
        //


        // Delay
        CompalAcpiStall(20000);
    } else {
        // Let EC wake up
        // Exit EC idle mode through set 0xFF10 = 0xFF

    }
    return EFI_SUCCESS;
}

EFI_STATUS
CompalECIdle (
    IN  BOOLEAN    EnableWrites
)
{
    //
    // Save Generic IO Decode 2
    //
    if (EnableWrites == TRUE) {
        //
        // If EC in Idle Mode
        //

        // Let EC into IDLE
        CompalECIdleMode();

        // Check EC RAM 0xF554 = 0xA5
        // Make sure EC in idle mode

    } else {
        // Exit 8051 reset

    }
    return EFI_SUCCESS;
}

VOID
CompalECIdleMode (
    VOID
)
{
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, EC_CMD_SYSTEM_NOTIFICATION_IDLE_MODE);
    CompalAcpiStall(5000);	//5ms
}

VOID
CompalECWaitMode (
    VOID
)
{
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, EC_CMD_SYSTEM_NOTIFICATION_WAIT_MODE);
}

EFI_STATUS
CompalEcSmmWait (
    IN  BOOLEAN     EnableWrites
)
{
    CompalECWait (EnableWrites);
    return EFI_SUCCESS;
}

EFI_STATUS
CompalEcSmmIdle (
    IN  BOOLEAN     EnableWrites
)
{
    CompalECIdle (EnableWrites);
    return EFI_SUCCESS;
}
