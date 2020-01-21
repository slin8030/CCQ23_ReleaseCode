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

#include "CompalBRLVDxe.h"

UINT16
PercentTo256Base (
  IN UINT8 Percent100
)
{
	UINT16 Percent256;

	if ( Percent100<100 )
	{
		Percent256 = (UINT16) Percent100;
		Percent256 = (Percent256 * 256) / 100;
	}
	else Percent256 = 0xFF;

	return (Percent256);
}

EFI_STATUS
CompalBRLVDxeEntry (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
  EFI_STATUS                              Status = EFI_SUCCESS;
  UINTN                                   Index;
  IGD_OPREGION_PROTOCOL                   *IgdOpRegion;
  VESA_BIOS_EXTENSIONS_EDID_DATA_BLOCK    *EdidDataBlock = NULL;
  EFI_EDID_DISCOVERED_PROTOCOL            *EdidData;

  Status = gBS->LocateProtocol (
               &gIgdOpRegionProtocolGuid, 
               NULL, 
               (VOID **)&IgdOpRegion
               );

  Status = gBS->LocateProtocol (
               &gEfiEdidDiscoveredProtocolGuid, 
               NULL, 
               (VOID **) &EdidData
               );

  if (!EFI_ERROR (Status)) {
      EdidDataBlock = (VESA_BIOS_EXTENSIONS_EDID_DATA_BLOCK *) &EdidData->Edid[0];
  }

  for(Index = 0; Index < (sizeof CompalBRLVPanelIDTABLE) / sizeof(CompalBRLVPANEL_TABLE); Index++) {
   if( EdidDataBlock->ProductCode == CompalBRLVPanelIDTABLE[Index].CompalBRLVPanelID) {
    // adjust panel brightness
    IgdOpRegion->OpRegion->MBox3.BCLM[0] = WORD_FIELD_VALID_BIT | 0x0000 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[0]);  // Set 0%
    IgdOpRegion->OpRegion->MBox3.BCLM[1] = WORD_FIELD_VALID_BIT | 0x0A00 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[1]);  // Set 10%
    IgdOpRegion->OpRegion->MBox3.BCLM[2] = WORD_FIELD_VALID_BIT | 0x1400 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[2]);  // Set 20%
    IgdOpRegion->OpRegion->MBox3.BCLM[3] = WORD_FIELD_VALID_BIT | 0x1E00 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[3]);  // Set 30%
    IgdOpRegion->OpRegion->MBox3.BCLM[4] = WORD_FIELD_VALID_BIT | 0x2800 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[4]);  // Set 40%
    IgdOpRegion->OpRegion->MBox3.BCLM[5] = WORD_FIELD_VALID_BIT | 0x3200 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[5]);  // Set 50%
    IgdOpRegion->OpRegion->MBox3.BCLM[6] = WORD_FIELD_VALID_BIT | 0x3C00 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[6]);  // Set 60%
    IgdOpRegion->OpRegion->MBox3.BCLM[7] = WORD_FIELD_VALID_BIT | 0x4600 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[7]);  // Set 70%
    IgdOpRegion->OpRegion->MBox3.BCLM[8] = WORD_FIELD_VALID_BIT | 0x5000 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[8]);  // Set 80%
    IgdOpRegion->OpRegion->MBox3.BCLM[9] = WORD_FIELD_VALID_BIT | 0x5A00 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[9]);  // Set 90%
    IgdOpRegion->OpRegion->MBox3.BCLM[10]= WORD_FIELD_VALID_BIT | 0x6400 | (UINT8) PercentTo256Base(CompalBRLVPanelIDTABLE[Index].CompalBRLVBrightnessLevel[10]); // Set 100%
   }
  }

  return EFI_SUCCESS;
}
