/** @file
  

Copyright (c) 2004 - 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _EFI_VKT_KBD_FONT_H_
#define _EFI_VKT_KBD_FONT_H_



// BitMap 00
UINT32 Font_30h [] = {
  0x03FC00,
  0x03FC00,
  0x1C1F80,
  0x1C1F80,
  0x1C1F80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1F0380,
  0x1F0380,
  0x1F0380,
  0x03FC00,
  0x03FC00,
  0x03FC00,
};

UINT32 Font_31h [] = { 
  0x007E00,
  0x007E00,
  0x01FE00,
  0x01FE00,
  0x01FE00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x0FFFF0,
  0x0FFFF0,
  0x0FFFF0
};

UINT32 Font_32h [] = { 
  0x1FFF80,
  0x1FFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x001FE0,
  0x001FE0,
  0x001FE0,
  0x03FF80,
  0x03FF80,
  0x03FF80,
  0x1FFC00,
  0x1FFC00,
  0xFF0000,
  0xFF0000,
  0xFF0000,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0	
};

UINT32 Font_33h [] = { 
  0x1FFFE0,
  0x1FFFE0,
  0x001F80,
  0x001F80,
  0x001F80,
  0x00FC00,
  0x00FC00,
  0x00FC00,
  0x03FF80,
  0x03FF80,
  0x03FF80,
  0x0003E0,
  0x0003E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80
};

UINT32 Font_34h [] = {
  0x00FF80,
  0x00FF80,
  0x03FF80,
  0x03FF80,
  0x03FF80,
  0x1F1F80,
  0x1F1F80,
  0x1F1F80,
  0xFC1F80,
  0xFC1F80,
  0xFC1F80,
  0xFFFFE0,
  0xFFFFE0,
  0x001F80,
  0x001F80,
  0x001F80,
  0x001F80,
  0x001F80,
  0x001F80
};

UINT32 Font_35h [] = {
  0xFFFF80,
  0xFFFF80,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFFFF80,
  0xFFFF80,
  0xFFFF80,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80
};

UINT32 Font_36h [] = {
  0x03FF80,
  0x03FF80,
  0x1F0000,
  0x1F0000,
  0x1F0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFFFF80,
  0xFFFF80,
  0xFFFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80
};

UINT32 Font_37h [] = {
  0xFFFFE0,
  0xFFFFE0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x001F80,
  0x001F80,
  0x001F80,
  0x00FC00,
  0x00FC00,
  0x00FC00,
  0x03E000,
  0x03E000,
  0x03E000,
  0x03E000,
  0x03E000,
  0x03E000,
  0x03E000,
  0x03E000
};

UINT32 Font_38h [] = {
  0x1FFC00,
  0x1FFC00,
  0xFC0380,
  0xFC0380,
  0xFC0380,
  0xFF0380,
  0xFF0380,
  0xFF0380,
  0x1FFC00,
  0x1FFC00,
  0x1FFC00,
  0xE0FFE0,
  0xE0FFE0,
  0xE003E0,
  0xE003E0,
  0xE003E0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80
};

UINT32 Font_39h [] = {
  0x1FFF80,
  0x1FFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1FFFE0,
  0x1FFFE0,
  0x1FFFE0,
  0x0003E0,
  0x0003E0,
  0x001F80,
  0x001F80,
  0x001F80,
  0x1FFC00,
  0x1FFC00,
  0x1FFC00
};                              

// ":"
UINT32 Font_3Ah [] = {
  0x00F800,
  0x00F800,
  0x00F800,
  0x00F800,
  0x00F800,
  0x00F800,
  0x000000,
  0x000000,
  0x000000,
  0x00F800,
  0x00F800,
  0x00F800,
  0x00F800,
  0x00F800
};

// ";"
UINT32 Font_3Bh [] = {
  0x001F00,
  0x001F00,
  0x001F00,
  0x001F00,
  0x001F00,
  0x001F00,
  0x000000,
  0x000000,
  0x000000,
  0x001F00,
  0x001F00,
  0x001F00,
  0x001F00,
  0x001F00,
  0x00F800,
  0x00F800,
  0x00F800
};

// "<"
UINT32 Font_3Ch [] = {
  0x000E00,
  0x000E00,
  0x007000,
  0x007000,
  0x007000,
  0x018000,
  0x018000,
  0x018000,
  0x0E0000,
  0x0E0000,
  0x0E0000,
  0x018000,
  0x018000,
  0x007000,
  0x007000,
  0x007000,
  0x000E00,
  0x000E00,
  0x000E00
};

// "="
UINT32 Font_3Dh [] = {
  0x000000,
  0x000000,
  0x000000,
  0x0FFFC0,
  0x0FFFC0,
  0x0FFFC0,
  0x000000,
  0x000000,
  0x000000,
  0x0FFFC0,
  0x0FFFC0,
  0x0FFFC0,
  0x000000,
  0x000000,
  0x000000
};

// ">"
UINT32 Font_3Eh [] = {
  0x0C0000,
  0x0C0000,
  0x038000,
  0x038000,
  0x038000,
  0x007000,
  0x007000,
  0x007000,
  0x000E00,
  0x000E00,
  0x000E00,
  0x007000,
  0x007000,
  0x038000,
  0x038000,
  0x038000,
  0x0C0000,
  0x0C0000,
  0x0C0000
};

// "?"
UINT32 Font_3Fh [] = {
  0x1FFF80,
  0x1FFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x001F80,
  0x001F80,
  0x001F80,
  0x03FC00,
  0x03FC00,
  0x000000,
  0x000000,
  0x000000,
  0x03FC00,
  0x03FC00,
  0x03FC00
};

// "@"
UINT32 Font_40h [] = {
  0x03FF80,
  0x03FF80,
  0x1C0060,
  0x1C0060,
  0x1C0060,
  0x000060,
  0x000060,
  0x000060,
  0x1FE060,
  0x1FE060,
  0x1FE060,
  0xE0E060,
  0xE0E060,
  0xE0E060,
  0xE0E060,
  0xE0E060,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80
};

// "A"
UINT32 Font_41h [] = {
  0x03FC00,
  0x03FC00,
  0x1F1F80,
  0x1F1F80,
  0x1F1F80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFFFFE0,
  0xFFFFE0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0
};

// "B"
UINT32 Font_42h [] = {
  0xFFFF80,
  0xFFFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFFFF80,
  0xFFFF80,
  0xFFFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFFFF80,
  0xFFFF80,
  0xFFFF80
};

// "C"
UINT32 Font_43h [] = {
  0x03FF80,
  0x03FF80,
  0x1F03E0,
  0x1F03E0,
  0x1F03E0,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0x1F03E0,
  0x1F03E0,
  0x1F03E0,
  0x03FF80,
  0x03FF80,
  0x03FF80
};

// "D"
UINT32 Font_44h [] = {
  0xFFFC00,
  0xFFFC00,
  0xFC1F80,
  0xFC1F80,
  0xFC1F80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC1F80,
  0xFC1F80,
  0xFC1F80,
  0xFFFC00,
  0xFFFC00,
  0xFFFC00
};

// "E"
UINT32 Font_45h [] = {
  0x0FFFF0,
  0x0FFFF0,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0FFFC0,
  0x0FFFC0,
  0x0FFFC0,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0FFFF0,
  0x0FFFF0,
  0x0FFFF0
};

// "F"
UINT32 Font_46h [] = {
  0xFFFFE0,
  0xFFFFE0,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFFFF80,
  0xFFFF80,
  0xFFFF80,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000	
};

// "G"
UINT32 Font_47h [] = {
	0x03FFE0,
  0x03FFE0,
  0x1F0000,
  0x1F0000,
  0x1F0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC1FE0,
  0xFC1FE0,
  0xFC1FE0,
  0xFC03E0,
  0xFC03E0,
  0x1F03E0,
  0x1F03E0,
  0x1F03E0,
  0x03FFE0,
  0x03FFE0,
  0x03FFE0
};


// "H"
UINT32 Font_48h [] = {
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0
};

// "I"
UINT32 Font_49h [] = {
  0x0FFFF0,
  0x0FFFF0,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x0FFFF0,
  0x0FFFF0,
  0x0FFFF0
};

// "J"
UINT32 Font_4Ah [] = {
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0x0003E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80
};

// "K"
UINT32 Font_4Bh [] = {
  0xFC03E0,
  0xFC03E0,
  0xFC1F80,
  0xFC1F80,
  0xFC1F80,
  0xFCFC00,
  0xFCFC00,
  0xFCFC00,
  0xFFE000,
  0xFFE000,
  0xFFE000,
  0xFFFC00,
  0xFFFC00,
  0xFCFF80,
  0xFCFF80,
  0xFCFF80,
  0xFC1FE0,
  0xFC1FE0,
  0xFC1FE0
};

// "L"
UINT32 Font_4Ch [] = {
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0FFFF0,
  0x0FFFF0,
  0x0FFFF0
};

// "M"
UINT32 Font_4Dh [] = {
  0xFC03E0,
  0xFC03E0,
  0xFF1FE0,
  0xFF1FE0,
  0xFF1FE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFCE3E0,
  0xFCE3E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0
};

// "N"
UINT32 Font_4Eh [] = {
  0xFC03E0,
  0xFC03E0,
  0xFF03E0,
  0xFF03E0,
  0xFF03E0,
  0xFFE3E0,
  0xFFE3E0,
  0xFFE3E0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFCFFE0,
  0xFCFFE0,
  0xFC1FE0,
  0xFC1FE0,
  0xFC1FE0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0
};

// "O"
UINT32 Font_4Fh [] = {
  0x1FFF80,
  0x1FFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80
};

// "P"
UINT32 Font_50h [] = {
  0xFFFF80,
  0xFFFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFFFF80,
  0xFFFF80,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0xFC0000
};

// "Q"
UINT32 Font_51h [] = {
  0x1FFF80,
  0x1FFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFCFFE0,
  0xFCFFE0,
  0xFC1F80,
  0xFC1F80,
  0xFC1F80,
  0x1FFC60,
  0x1FFC60,
  0x1FFC60
};

// "R"
UINT32 Font_52h [] = {
  0xFFFF80,
  0xFFFF80,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC1FE0,
  0xFC1FE0,
  0xFC1FE0,
  0xFFFC00,
  0xFFFC00,
  0xFCFF80,
  0xFCFF80,
  0xFCFF80,
  0xFC1FE0,
  0xFC1FE0,
  0xFC1FE0
};

// "S"
UINT32 Font_53h [] = {
  0x1FFC00,
  0x1FFC00,
  0xFC1F80,
  0xFC1F80,
  0xFC1F80,
  0xFC0000,
  0xFC0000,
  0xFC0000,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80,
  0x0003E0,
  0x0003E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80	
};

// "T"
UINT32 Font_54h [] = {
  0x0FFFFF,
  0x0FFFFF,
  0x0FFFFF,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00
};

// "U"
UINT32 Font_55h [] = {
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80
};

// "V"
UINT32 Font_56h [] = {
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFF1FE0,
  0xFF1FE0,
  0xFF1FE0,
  0x1FFF80,
  0x1FFF80,
  0x03FC00,
  0x03FC00,
  0x03FC00,
  0x00E000,
  0x00E000,
  0x00E000
};

// "W"
UINT32 Font_57h [] = {
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0,
  0xFCE3E0,
  0xFCE3E0,
  0xFCE3E0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0,
  0xFF1FE0,
  0xFF1FE0,
  0xFF1FE0,
  0x1C0380,
  0x1C0380,
  0x1C0380
};

// "X"
UINT32 Font_58h [] = {
  0xFC03E0,
  0xFC03E0,
  0xFF1FE0,
  0xFF1FE0,
  0xFF1FE0,
  0x1FFF80,
  0x1FFF80,
  0x1FFF80,
  0x03FC00,
  0x03FC00,
  0x03FC00,
  0x1FFF80,
  0x1FFF80,
  0xFF1FE0,
  0xFF1FE0,
  0xFF1FE0,
  0xFC03E0,
  0xFC03E0,
  0xFC03E0
};

// "Y"
UINT32 Font_59h [] = {
  0x0F81F0,
  0x0F81F0,
  0x0F81F0,
  0x0F81F0,
  0x0F81F0,
  0x0F81F0,
  0x0F81F0,
  0x0F81F0,
  0x01FFC0,
  0x01FFC0,
  0x01FFC0,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00
};

// "Z"
UINT32 Font_5Ah [] = {
  0xFFFFE0,
  0xFFFFE0,
  0x001FE0,
  0x001FE0,
  0x001FE0,
  0x00FF80,
  0x00FF80,
  0x00FF80,
  0x03FC00,
  0x03FC00,
  0x03FC00,
  0x1FE000,
  0x1FE000,
  0xFF0000,
  0xFF0000,
  0xFF0000,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0
};

// "["
UINT32 Font_5Bh [] = {
  0x0FFE00,
  0x0FFE00,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0F8000,
  0x0FFE00,
  0x0FFE00,
  0x0FFE00
};

// "\"
UINT32 Font_5Ch [] = {
  0xE00000,
  0xE00000,
  0x1C0000,
  0x1C0000,
  0x1C0000,
  0x030000,
  0x030000,
  0x030000,
  0x00E000,
  0x00E000,
  0x00E000,
  0x001C00,
  0x001C00,
  0x000380,
  0x000380,
  0x000380,
  0x000060,
  0x000060,
  0x000060
};

// "]"
UINT32 Font_5Dh [] = {
  0x0FFE00,
  0x0FFE00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x007E00,
  0x0FFE00,
  0x0FFE00,
  0x0FFE00
};

// "^"
UINT32 Font_5Eh [] = {
  0x00E000,
  0x00E000,
  0x031C00,
  0x031C00,
  0x031C00,
  0x1C0380,
  0x1C0380,
  0x1C0380,
  0xE00060,
  0xE00060,
  0xE00060
};

// "_"
UINT32 Font_5Fh [] = {
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0x000000,
  0xFFFFE0,
  0xFFFFE0,
  0xFFFFE0
};
#endif
