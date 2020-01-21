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
*/

#ifndef _COMPAL_EEPROM_DXE_PROTOCOL_H_
#define _COMPAL_EEPROM_DXE_PROTOCOL_H_

#define COMPAL_EEPROM_DXE_PROTOCOL_GUID  \
  { \
    0x76f54ec, 0xc230, 0x4fec, 0xbd, 0xf, 0x2f, 0xc7, 0xd3, 0x98, 0xe3, 0xa2  \
  }

typedef struct _COMPAL_EEPROM_DXE_PROTOCOL COMPAL_EEPROM_DXE_PROTOCOL;

//
// Member functions
//
typedef
EFI_STATUS
(EFIAPI *COMPAL_EEPROM_DXE_READ_PROTOCOL) (
  IN  COMPAL_EEPROM_DXE_PROTOCOL     *This,
  IN  UINT16                         OemEepromOffset,
  IN  UINT16                         OemEepromSize,
  IN  UINT8                          OemEepromReadPolicy,
  OUT UINT8                          *OemEepromDataBuffer
);

typedef
EFI_STATUS
(EFIAPI *COMPAL_EEPROM_DXE_WRITE_PROTOCOL) (
  IN  COMPAL_EEPROM_DXE_PROTOCOL     *This,
  IN  UINT16                         OemEepromOffset,
  IN  UINT16                         OemEepromSize,
  IN  UINT8                          *OemEepromDataBuffer
);

//
// Protocol Definition
//
typedef struct _COMPAL_EEPROM_DXE_PROTOCOL {
  UINT32                            CompalEepromDxeBaseAddr;
  COMPAL_EEPROM_DXE_READ_PROTOCOL   CompalEepromDxeRead;
  COMPAL_EEPROM_DXE_WRITE_PROTOCOL  CompalEepromDxeWrite;
} COMPAL_EEPROM_DXE_PROTOCOL;

extern EFI_GUID gCompalEEPROMDxeProtocolGuid;

#endif
