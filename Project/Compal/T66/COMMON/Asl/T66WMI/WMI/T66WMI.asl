/*---------------------------------------------------------------------------
 *
 *	$Workfile:   T66WMI.ASL  $
 *
 *	$Header:   $
 *
 *	Copyright (c) 2016~ Compal Company
 *
 *	This program contains proprietary and confidential information.
 *	All rights reserved except as may be permitted by prior
 *	written consent.
 *
 *	Content:
 *
 *		R E V I S I O N    H I S T O R Y
 *	$Log:   $
 *
 *  Date        Who             Change
 *	----------  ------------    -------------------------------------------
 *-------------------------------------------------------------------------*/

// #define WMI_SW_SMI           0xCE
// #define WMI_OEM_SW_SMI       0xCF

//
// Create a Global MUTEX.
//

Mutex(MUTT,0)

Scope(\_SB) {
  Device(WMID)
  {
  //
  // PNP0C14 is the ID assigned by Microsoft to the WMI to ACPI mapper
  //
     Name(_HID, "PNP0C14")
     Name(_UID, "0x0")
  //
  // _WDG evaluates to a data structure that specifies the data blocks supported
  // by the ACPI device.
  //
     Name(_WDG, Buffer() {

  //
  // Method WMAB :GUID("{94AC939D-C89E-4CBB-A1B3-FE0B1569AC97}")
  //
         0x9D, 0x93, 0xAC, 0x94, 0x9E, 0xC8, 0xBB, 0x4C, 0xA1, 0xB3, 0xFE, 0x0B, 0x15, 0x69, 0xAC, 0x97,
         65, 66,        // Object ID (AB)
         1,             // Instance Count
         0x02,          // Flags WMIACPI_REGFLAG_METHOD
  //
  // Method WMAC :GUID("{E4648D4A-5C35-49EA-9C15-06F334926E16}")
  //
         0x4A, 0x8D, 0x64, 0xE4, 0x35, 0x5C, 0xEA, 0x49, 0x9C, 0x15, 0x06, 0xF3, 0x34, 0x92, 0x6E, 0x16,
         65, 67,        // Object ID (AC)
         1,             // Instance Count
         0x02,          // Flags WMIACPI_REGFLAG_METHOD
  //
  // Method WMAD :GUID("{39806AF8-77DC-48B7-B2D6-E530700A2480}")
  //
         0xF8, 0x6A, 0x80, 0x39, 0xDC, 0x77, 0xB7, 0x48, 0xB2, 0xD6, 0xE5, 0x30, 0x70, 0x0A, 0x24, 0x80,
         65, 68,        // Object ID (AD)
         1,             // Instance Count
         0x02,          // Flags WMIACPI_REGFLAG_METHOD
  //
  // This GUID for returning the MOF data
  //
         0x21, 0x12, 0x90, 0x05, 0x66, 0xd5, 0xd1, 0x11, 0xb2, 0xf0, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0x10,
         66, 65,        // Object ID (BA)
         1,             // Instance Count
         0x00,          // Flags
     })

     #include "T66WMI_Def.asl"

     External(\STDT, IntObj)
     External(\BFDT, IntObj)
     External(\SFNO, IntObj)

     #ifdef WMI_WMAB_SUPPORT
      #include "WMI/WMAB.asl"       // Method   : WMAB --> BIOS
     #endif

     #ifdef WMI_WMAC_SUPPORT
      #include "WMI/WMAC.asl"       // Method   : WMAC --> DIO
     #endif

     #ifdef WMI_WMAD_SUPPORT
      #include "WMI/WMAD.asl"       // Method   : WMAD --> CPU
     #endif

     #include "WMI/WQBA.asl"       // MOF data : WQAB
  }
}

