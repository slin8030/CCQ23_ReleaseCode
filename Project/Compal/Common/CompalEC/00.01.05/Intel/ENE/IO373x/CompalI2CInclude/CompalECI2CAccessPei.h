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

#ifndef COMPAL_ECI2CACCESS_PEI_H
#define COMPAL_ECI2CACCESS_PEI_H

///
/// Memory Mapped PCI Access macros
//
#define DEFAULT_PCI_BUS_NUMBER_PCH              0

#define PCI_DEVICE_NUMBER_PCH_LPC               31
#define PCI_FUNCTION_NUMBER_PCH_LPC             0

#define R_PCH_LPC_ACPI_BASE                     0x40                // ABASE, 16bit
#define R_PCH_LPC_ACPI_BASEADR                  0x400               // ABASE, 16bit
#define B_PCH_LPC_ACPI_BASE_EN                  BIT1                // Enable Bit
#define B_PCH_LPC_ACPI_BASE_BAR                 0x0000FF80          // Base Address, 128 Bytes
#define V_PCH_ACPI_PM1_TMR_MAX_VAL              0x1000000           // The timer is 24 bit overflow
#define B_PCH_ACPI_PM1_TMR_VAL                  0xFFFFFF            // The timer value mask

#define R_PCH_ACPI_PM1_TMR                      0x08                // Power Management 1 Timer
#define V_PCH_ACPI_PM1_TMR_FREQUENCY            3579545             // Timer Frequency


#define PchLpcPciCfg8(Register) I2CLibPeiMmioRead8 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, Register))

#define PCIEX_BASE_ADDRESS  0xE0000000
#define PCI_EXPRESS_BASE_ADDRESS ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)

#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )
#endif      //COMPAL_ECI2CACCESS_PEI_H

