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
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
*/

#ifndef _COMPAL_FWRECOVERY_ELTORITO_H_
#define _COMPAL_FWRECOVERY_ELTORITO_H_

#pragma pack(1)
//
// CDROM_VOLUME_DESCRIPTOR.Types
//
#define CDVOL_TYPE_STANDARD 0x0
#define CDVOL_TYPE_CODED    0x1
#define CDVOL_TYPE_END      0xFF

//
// CDROM_VOLUME_DESCRIPTOR.Id
//
#define CDVOL_ID  "CD001"

//
// CDROM_VOLUME_DESCRIPTOR.SystemId
//
#define CDVOL_ELTORITO_ID "EL TORITO SPECIFICATION"

//
// Indicator types
//
#define ELTORITO_ID_CATALOG               0x01
#define ELTORITO_ID_SECTION_BOOTABLE      0x88
#define ELTORITO_ID_SECTION_NOT_BOOTABLE  0x00
#define ELTORITO_ID_SECTION_HEADER        0x90
#define ELTORITO_ID_SECTION_HEADER_FINAL  0x91

//
// ELTORITO_CATALOG.Boot.MediaTypes
//
#define ELTORITO_NO_EMULATION 0x00
#define ELTORITO_12_DISKETTE  0x01
#define ELTORITO_14_DISKETTE  0x02
#define ELTORITO_28_DISKETTE  0x03
#define ELTORITO_HARD_DISK    0x04

//
// El Torito Volume Descriptor
// Note that the CDROM_VOLUME_DESCRIPTOR does not match the ISO-9660
// desctiptor.  For some reason descriptor used by El Torito is
// different, but they start the same.   The El Torito descriptor
// is left shifted 1 byte starting with the SystemId.  (Note this
// causes the field to get unaligned)
//
typedef struct {
    UINT8   Type;
    CHAR8   Id[5];  // CD001
    UINT8   Version;
    CHAR8   SystemId[26];
    CHAR8   Unused[38];
    UINT8   EltCatalog[4];
    CHAR8   Unused2[5];
    UINT32  VolSpaceSize[2];
} CDROM_VOLUME_DESCRIPTOR;

//
// Catalog Entry
//
typedef union {
    struct {
        CHAR8 Reserved[0x20];
    } Unknown;

    //
    // Catalog validation entry (Catalog header)
    //
    struct {
        UINT8   Indicator;
        UINT8   PlatformId;
        UINT16  Reserved;
        CHAR8   ManufacId[24];
        UINT16  Checksum;
        UINT16  Id55AA;
    } Catalog;

    //
    // Initial/Default Entry or Section Entry
    //
    struct {
        UINT8   Indicator;
        UINT8   MediaType : 4;
        UINT8   Reserved1 : 4;
        UINT16  LoadSegment;
        UINT8   SystemType;
        UINT8   Reserved2;
        UINT16  SectorCount;
        UINT32  Lba;
    } Boot;

    //
    // Section Header Entry
    //
    struct {
        UINT8   Indicator;
        UINT8   PlatformId;
        UINT16  SectionEntries;
        CHAR8   Id[28];
    } Section;

} ELTORITO_CATALOG;

#pragma pack()

#endif
