/*++

Copyright (c)  COMPAL Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
COMPAL Corporation.


Module Name:

  EdidDefinition.h

Abstract:

  Protocol for describing Oem Eeprom Feature

Revision History

--*/

#ifndef _EDID_H_
#define _EDID_H_

//
// EDID Offset Definition
//
#define EDID_HEADER_OFFSET                                 0x00   // EDID header "00 FF FF FF FF FF FF 00"
#define EDID_MANUFACTURE_NAME_OFFSET                       0x08   // EISA 3-Character ID
#define EDID_PRODUCT_CODE_OFFSET                           0x0A   // Vendor Sssigned Code
#define EDID_SERIAL_NUMBER_OFFSET                          0x0C   // 32-bit Serial Number
#define EDID_WEEK_OF_MANUFACTURE_OFFSET                    0x10   // Week of Manufacture
#define EDID_YEAR_OF_MANUFACTURE_OFFSET                    0x11   // Year of Manufacture. Add 1990 to the value for actual year
#define EDID_VERSION_OFFSET                                0x12   // EDID Structure Version
#define EDID_REVISION_OFFSET                               0x13   // EDID Structure Revision
#define EDID_VIDEO_INPUT_DEFINITION_OFFSET                 0x14   // See Spec. Table 3.6 
#define EDID_MAX_HORIZONTAL_IMAGE_SIZE_OFFSET              0x15   // Cm 
#define EDID_MAXV_ERTICAL_IMAGE_SIZE_OFFSET                0x16   // Cm
#define EDID_DISPLAY_TRANSFER_CHARACTERISTIC_OFFSET        0x17   // Display Gamma. Divide by 100, then add 1 for actual value. 
#define EDID_FEATURE_SUPPORT_OFFSET                        0x18   // See Spec. Table 3.11 
#define EDID_RED_GREEN_LOW_BITS_OFFSET                     0x19   // Rx1 Rx0 Ry1 Ry0 Gx1 Gx0 Gy1Gy0 
#define EDID_BLUE_WHITE_LOW_BITS_OFFSET                    0x1A   // Bx1 Bx0 By1 By0 Wx1 Wx0 Wy1 Wy0 
#define EDID_RED_X_OFFSET                                  0x1B   // Red-x Bits 9 - 2
#define EDID_RED_Y_OFFSET                                  0x1C   // Red-y Bits 9 - 2 
#define EDID_GREEN_X_OFFSET                                0x1D   // Green-x Bits 9 - 2 
#define EDID_GREEN_Y_OFFSET                                0x1E   // Green-y Bits 9 - 2 
#define EDID_BLUE_X_OFFSET                                 0x1F   // Blue-x Bits 9 - 2 
#define EDID_BLUE_Y_OFFSET                                 0x20   // Blue-y Bits 9 - 2
#define EDID_WHITE_X_OFFSET                                0x21   // White-x Bits 9 - 2 
#define EDID_WHITE_Y_OFFSET                                0x22   // White-x Bits 9 - 2 
#define EDID_ESTABLISHED_TIMINGS_1_OFFSET                  0x23   // See Spec. Section 3.8 
#define EDID_ESTABLISHED_TIMINGS_2_OFFSET                  0x24   // See Spec. Section 3.8 
#define EDID_MANUFACTURER_RESERVED_TIMINGS_OFFSET          0x25   // See Spec. Section 3.8 
#define EDID_STANDARD_TIMING_IDENTIFICATION_OFFSET         0x26   // See Spec. Section 3.9 
#define EDID_DETAILED_TIMING_DESCRIPTIONS_OFFSET           0x36   // See Detailed Timing Descriptions (18 Bytes) Offset Definition 
#define EDID_EXTENSION_FLAG_OFFSET                         0x7E   // Number of (optional) 128-byte EDID extension blocks to follow 
#define EDID_CHECKSUM_OFFSET                               0x7F   // The 1-byte sum of all 128 bytes in this EDID block shall equal zero

//
// Detailed Timing Descriptions (18 Bytes) Offset Definition
//
#define EDID_DTD1_PIXEL_CLOCK_OFFSET                       0x36   // DTD1 Pixel clock / 10,000
#define EDID_DTD1_HORIZONTAL_ACTIVE_OFFSET                 0x38   // DTD1 Pixels, lower 8 bits
#define EDID_DTD1_HORIZONTAL_BLANKING_OFFSET               0x39   // DTD1 Pixels, lower 8 bits 
#define EDID_DTD1_HORIZONTAL_ACTIVE_BLANKING_OFFSET        0x3A   // DTD1 Upper 4 bits: Active, Lower 4 bits: Blanking
#define EDID_DTD1_Vertical_Active_OFFSET                   0x3B   // DTD1 Lines, lower 8 bits 
#define EDID_DTD1_VERTICAL_BLANKING_OFFSET                 0x3C   // DTD1 Lines, lower 8 bits
#define EDID_DTD1_VERTICAL_ACTIVE_BLANKING_OFFSET          0x3D   // DTD1 Upper 4 bits: Active, Lower 4 bits: Blanking 
#define EDID_DTD1_HORIZONTAL_SYNC_OFFSET_OFFSET            0x3E   // DTD1 Pixels , from blanking starts, lower 8 bits
#define EDID_DTD1_HORIZONTAL_SYNC_PULSE_WIDTH_OFFSET       0x3F   // DTD1 Pixels, lower 8 bits 
#define EDID_DTD1_VERTICAL_SYNC_OFFSET                     0x40   // DTD1 Upper 4 bits: Sync Offset, Lower 4 bits: Sync Pulse Width 
#define EDID_DTD1_HORIZONTAL_VERTICAL_SYNC_OFFSET          0x41   // DTD1 Each 2 bits: H-Sync Offset, H-Sync Pulse Width, V-Sync Offset, V-Sync Pulse Width
#define EDID_DTD1_HORIZONTAL_IMAGE_SIZE_OFFSET             0x42   // DTD1 mm, lower 8 bits
#define EDID_DTD1_VERTICAL_IMAGE_SIZE_OFFSET               0x43   // DTD1 mm, lower 8 bits 
#define EDID_DTD1_HORIZONTAL_VERTICAL_IMAGE_SIZE_OFFSET    0x44   // DTD1 Upper 4 bits: Horizontal, Lower 4 bits: Vertical 
#define EDID_DTD1_HORIZONTAL_BORDER_OFFSET                 0x45   // DTD1 Pixels, see Section 3.12 
#define EDID_DTD1_VERTICAL_BORDER_OFFSET                   0x46   // DTD1 Lines, see Section 3.12
#define EDID_DTD1_FLAGS_OFFSET                             0x47   // DTD1 See Spec. Table 3.16, 3.17 for definition
#define EDID_DTD2_PIXEL_CLOCK_OFFSET                       0x48   // DTD2 Pixel clock / 10,000
#define EDID_DTD2_HORIZONTAL_ACTIVE_OFFSET                 0x4A   // DTD2 Pixels, lower 8 bits
#define EDID_DTD2_HORIZONTAL_BLANKING_OFFSET               0x4B   // DTD2 Pixels, lower 8 bits 
#define EDID_DTD2_HORIZONTAL_ACTIVE_BLANKING_OFFSET        0x4C   // DTD2 Upper 4 bits: Active, Lower 4 bits: Blanking
#define EDID_DTD2_Vertical_Active_OFFSET                   0x4D   // DTD2 Lines, lower 8 bits 
#define EDID_DTD2_VERTICAL_BLANKING_OFFSET                 0x4E   // DTD2 Lines, lower 8 bits
#define EDID_DTD2_VERTICAL_ACTIVE_BLANKING_OFFSET          0x4F   // DTD2 Upper 4 bits: Active, Lower 4 bits: Blanking 
#define EDID_DTD2_HORIZONTAL_SYNC_OFFSET_OFFSET            0x50   // DTD2 Pixels , from blanking starts, lower 8 bits
#define EDID_DTD2_HORIZONTAL_SYNC_PULSE_WIDTH_OFFSET       0x51   // DTD2 Pixels, lower 8 bits 
#define EDID_DTD2_VERTICAL_SYNC_OFFSET                     0x52   // DTD2 Upper 4 bits: Sync Offset, Lower 4 bits: Sync Pulse Width 
#define EDID_DTD2_HORIZONTAL_VERTICAL_SYNC_OFFSET          0x53   // DTD2 Each 2 bits: H-Sync Offset, H-Sync Pulse Width, V-Sync Offset, V-Sync Pulse Width
#define EDID_DTD2_HORIZONTAL_IMAGE_SIZE_OFFSET             0x54   // DTD2 mm, lower 8 bits
#define EDID_DTD2_VERTICAL_IMAGE_SIZE_OFFSET               0x55   // DTD2 mm, lower 8 bits 
#define EDID_DTD2_HORIZONTAL_VERTICAL_IMAGE_SIZE_OFFSET    0x56   // DTD2 Upper 4 bits: Horizontal, Lower 4 bits: Vertical 
#define EDID_DTD2_HORIZONTAL_BORDER_OFFSET                 0x57   // DTD2 Pixels, see Section 3.12 
#define EDID_DTD2_VERTICAL_BORDER_OFFSET                   0x58   // DTD2 Lines, see Section 3.12
#define EDID_DTD2_FLAGS_OFFSET                             0x59   // DTD2 See Spec. Table 3.16, 3.17 for definition
#define EDID_DTD3_PIXEL_CLOCK_OFFSET                       0x5A   // DTD3 Pixel clock / 10,000
#define EDID_DTD3_HORIZONTAL_ACTIVE_OFFSET                 0x5C   // DTD3 Pixels, lower 8 bits
#define EDID_DTD3_HORIZONTAL_BLANKING_OFFSET               0x5D   // DTD3 Pixels, lower 8 bits 
#define EDID_DTD3_HORIZONTAL_ACTIVE_BLANKING_OFFSET        0x5E   // DTD3 Upper 4 bits: Active, Lower 4 bits: Blanking
#define EDID_DTD3_Vertical_Active_OFFSET                   0x5F   // DTD3 Lines, lower 8 bits 
#define EDID_DTD3_VERTICAL_BLANKING_OFFSET                 0x60   // DTD3 Lines, lower 8 bits
#define EDID_DTD3_VERTICAL_ACTIVE_BLANKING_OFFSET          0x61   // DTD3 Upper 4 bits: Active, Lower 4 bits: Blanking 
#define EDID_DTD3_HORIZONTAL_SYNC_OFFSET_OFFSET            0x62   // DTD3 Pixels , from blanking starts, lower 8 bits
#define EDID_DTD3_HORIZONTAL_SYNC_PULSE_WIDTH_OFFSET       0x63   // DTD3 Pixels, lower 8 bits 
#define EDID_DTD3_VERTICAL_SYNC_OFFSET                     0x64   // DTD3 Upper 4 bits: Sync Offset, Lower 4 bits: Sync Pulse Width 
#define EDID_DTD3_HORIZONTAL_VERTICAL_SYNC_OFFSET          0x65   // DTD3 Each 2 bits: H-Sync Offset, H-Sync Pulse Width, V-Sync Offset, V-Sync Pulse Width
#define EDID_DTD3_HORIZONTAL_IMAGE_SIZE_OFFSET             0x66   // DTD3 mm, lower 8 bits
#define EDID_DTD3_VERTICAL_IMAGE_SIZE_OFFSET               0x67   // DTD3 mm, lower 8 bits 
#define EDID_DTD3_HORIZONTAL_VERTICAL_IMAGE_SIZE_OFFSET    0x68   // DTD3 Upper 4 bits: Horizontal, Lower 4 bits: Vertical 
#define EDID_DTD3_HORIZONTAL_BORDER_OFFSET                 0x69   // DTD3 Pixels, see Section 3.12 
#define EDID_DTD3_VERTICAL_BORDER_OFFSET                   0x6A   // DTD3 Lines, see Section 3.12
#define EDID_DTD3_FLAGS_OFFSET                             0x6B   // DTD3 See Spec. Table 3.16, 3.17 for definition
#define EDID_DTD4_PIXEL_CLOCK_OFFSET                       0x6C   // DTD4 Pixel clock / 10,000
#define EDID_DTD4_HORIZONTAL_ACTIVE_OFFSET                 0x6E   // DTD4 Pixels, lower 8 bits
#define EDID_DTD4_HORIZONTAL_BLANKING_OFFSET               0x6F   // DTD4 Pixels, lower 8 bits 
#define EDID_DTD4_HORIZONTAL_ACTIVE_BLANKING_OFFSET        0x70   // DTD4 Upper 4 bits: Active, Lower 4 bits: Blanking
#define EDID_DTD4_Vertical_Active_OFFSET                   0x71   // DTD4 Lines, lower 8 bits 
#define EDID_DTD4_VERTICAL_BLANKING_OFFSET                 0x72   // DTD4 Lines, lower 8 bits
#define EDID_DTD4_VERTICAL_ACTIVE_BLANKING_OFFSET          0x73   // DTD4 Upper 4 bits: Active, Lower 4 bits: Blanking 
#define EDID_DTD4_HORIZONTAL_SYNC_OFFSET_OFFSET            0x74   // DTD4 Pixels , from blanking starts, lower 8 bits
#define EDID_DTD4_HORIZONTAL_SYNC_PULSE_WIDTH_OFFSET       0x75   // DTD4 Pixels, lower 8 bits 
#define EDID_DTD4_VERTICAL_SYNC_OFFSET                     0x76   // DTD4 Upper 4 bits: Sync Offset, Lower 4 bits: Sync Pulse Width 
#define EDID_DTD4_HORIZONTAL_VERTICAL_SYNC_OFFSET          0x77   // DTD4 Each 2 bits: H-Sync Offset, H-Sync Pulse Width, V-Sync Offset, V-Sync Pulse Width
#define EDID_DTD4_HORIZONTAL_IMAGE_SIZE_OFFSET             0x78   // DTD4 mm, lower 8 bits
#define EDID_DTD4_VERTICAL_IMAGE_SIZE_OFFSET               0x79   // DTD4 mm, lower 8 bits 
#define EDID_DTD4_HORIZONTAL_VERTICAL_IMAGE_SIZE_OFFSET    0x7A   // DTD4 Upper 4 bits: Horizontal, Lower 4 bits: Vertical 
#define EDID_DTD4_HORIZONTAL_BORDER_OFFSET                 0x7B   // DTD4 Pixels, see Section 3.12 
#define EDID_DTD4_VERTICAL_BORDER_OFFSET                   0x7C   // DTD4 Lines, see Section 3.12
#define EDID_DTD4_FLAGS_OFFSET                             0x7D   // DTD4 See Spec. Table 3.16, 3.17 for definition

#endif
