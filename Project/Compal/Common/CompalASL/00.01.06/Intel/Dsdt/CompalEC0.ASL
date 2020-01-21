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
 1.00                Vanessa      Init version.
 1.01      16/07/05  Vanessa      Move CompalCMFC.asl from CompalECFilter.asl
*/

//
// [COMMON ASL] EC functions using EDK II
//

// ECOK - Return TRUE if EC Namespace accessible.
Method(ECOK,0)
{
        If (LEqual(COMMON_ASL_EC_PATH.OKEC, 0x01))
        {
                Return (1)                      //return true
        }
        else
        {
                Return (0)                      //return false
        }
}

Device(EC0)
{
        Name(_HID,  EISAID("PNP0C09"))  // PNP ID

        Name(_GPE, EC_SCI)      // EC SCI is connected to GPE.EC_SCI

        #if (ECRAM_MEMMAP == ENABLE)

                // If ECRAM memory I/O function enable, don't 
                // waiting for driver ready.

                Name(OKEC, 0x01)        

        #else
                Name(OKEC, Zero)

        #endif

        // EC resources
        Name(_CRS, ResourceTemplate()
        {
                IO(Decode16,0x62,0x62,0x1,0x1)
                IO(Decode16,0x66,0x66,0x1,0x1)
        })

        /* ------------------------------------------------------------------------*/
        // Name: _REG - This method is called by drivers to register installation
        //              and removal.  For example; OS calls _REG(3,1) to indicate
        //              to the ACPI BIOS that the Embedded Controller Driver is
        //              present and functional.
        //
        //              ARG0: 0=Memory
        //                    1=I/O
        //                    2=PCI Config
        //                    3=Embedded Controller
        //                    4=SMBus
        //
        //              ARG1: 0=Handler Not Connected
        //                    1=Handler Connected
        //
        Method(_REG, 2)
        {
                If (LEqual(Arg0, 0x03))
                {
                        Store(Arg1, OKEC)
                        COMMON_ASL_LPC_PATH.OSTP()
                        COMMON_ASL_LPC_PATH.ECFG(1) //Set ACPI EC driver ready flag

                        // ---------------------------------------------------------------------
                        // If you need add the features or workaround, please define a method
                        // with path which names EC_REG_HOOKS in OEMASL.asl when _REG is run.
                        //
                        // For example,
                        //
                        // In OEMASL.ASL -
                        // #define EC_REG_HOOKS   COMMON_ASL_LPC_PATH.OSTP(Arg0)  // Notice OS type to EC.
                        //
                        // Create a new method in one of OEM ASL files -
                        // Method(OSTP, 1) {....}
                        //
                        #ifdef EC_REG_HOOKS
                            EC_REG_HOOKS
                        #endif

                }
        } // end of _REG

        // EC RAM fields
        #include "Compalecname.ASL"             // EC Name Space
        #include "CompalECFILTER.ASL"
        #include "CompalQuery.ASL"              // Compal common query event.         
        #include "CompalCMFC.ASL"               // Compal CMFC function code.         

}// END device EC0
