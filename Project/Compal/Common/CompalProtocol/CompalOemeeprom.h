/*
 * (C) Copyright 2011-2012 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 */
/*
 Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
This software, and any portion thereof, is referred to herein as the "Software."

USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
In such a case, Compal will provide You with written notices of such claim, suit, or action.

DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef OEM_H_
#define OEM_H_



//--------------------------------------------------------------------------
//Define OEM Setting Variable.
//--------------------------------------------------------------------------

#define CompalVariableBank0_GUID \
  { \
    0xA86E3B62, 0x28F4, 0x11E4, {0xAB, 0x4C, 0xA6, 0xC5, 0xE4, 0xD2, 0x2F, 0xB7}\
  }
#define CompalVariableBank1_GUID \
  { \
    0xA86E3B0A, 0x28F4, 0x11E4, {0xAB, 0x4C, 0xA6, 0xC5, 0xE4, 0xD2, 0x2F, 0xB7}\
  }
#define CompalVariableBank2_GUID \
  { \
    0xA86E3B7B, 0x28F4, 0x11E4, {0xAB, 0x4C, 0xA6, 0xC5, 0xE4, 0xD2, 0x2F, 0xB7}\
  }
#define CompalVariableBank3_GUID \
  { \
    0xA86E3B39, 0x28F4, 0x11E4, {0xAB, 0x4C, 0xA6, 0xC5, 0xE4, 0xD2, 0x2F, 0xB7}\
  }
#define CompalVariableBank4_GUID \
  { \
    0xA86E3B23, 0x28F4, 0x11E4, {0xAB, 0x4C, 0xA6, 0xC5, 0xE4, 0xD2, 0x2F, 0xB7}\
  }
#define CompalVariableBank5_GUID \
  { \
    0xA86E3B56, 0x28F4, 0x11E4, {0xAB, 0x4C, 0xA6, 0xC5, 0xE4, 0xD2, 0x2F, 0xB7}\
  }
#define CompalVariableBank6_GUID \
  { \
    0xA86E3B12, 0x28F4, 0x11E4, {0xAB, 0x4C, 0xA6, 0xC5, 0xE4, 0xD2, 0x2F, 0xB7}\
  }
#define CompalVariableBank7_GUID \
  { \
    0xA86E3B94, 0x28F4, 0x11E4, {0xAB, 0x4C, 0xA6, 0xC5, 0xE4, 0xD2, 0x2F, 0xB7}\
  }
#define OEMSettingVariableName0   L"EepromData0"
#define OEMSettingVariableName1   L"EepromData1"
#define OEMSettingVariableName2   L"EepromData2"
#define OEMSettingVariableName3   L"EepromData3"
#define OEMSettingVariableName4   L"EepromData4"
#define OEMSettingVariableName5   L"EepromData5"
#define OEMSettingVariableName6   L"EepromData6"
#define OEMSettingVariableName7   L"EepromData7"

#pragma pack(1)

/*
 * Please keep the size of COMPAL_EEPROM_CONFIGURATION
 * at 2048-byte
 */
typedef struct {
UINT8    EEPROM[256];
}OEMDRIVER_SAMPLE_CONFIGURATION0;

typedef struct {
  UINT8    EEPROM[256];
}OEMDRIVER_SAMPLE_CONFIGURATION1;
typedef struct {

  UINT8    EEPROM[256];
}OEMDRIVER_SAMPLE_CONFIGURATION2;
typedef struct {

  UINT8    EEPROM[256];
}OEMDRIVER_SAMPLE_CONFIGURATION3;
typedef struct {

  UINT8    EEPROM[256];
}OEMDRIVER_SAMPLE_CONFIGURATION4;
typedef struct {

  UINT8    EEPROM[256];
}OEMDRIVER_SAMPLE_CONFIGURATION5;
typedef struct {

  UINT8    EEPROM[256];
}OEMDRIVER_SAMPLE_CONFIGURATION6;
typedef struct {

  UINT8    EEPROM[256];
}OEMDRIVER_SAMPLE_CONFIGURATION7;
#pragma pack()

//--------------------------------------------------------------------------
//Define InvenSense Setting Variable.
//--------------------------------------------------------------------------

#endif /* OEM_H_ */

