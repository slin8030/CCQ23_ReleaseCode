/** @file

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

Scope (IGPU_SCOPE)
{
//
// ACPI Control Methods and Notifications
// Rev. 0.30
//
// ATI GFX Interface: ATIF
//
//   This is the major entry point used by the driver for ATI specific functionality
//   in the System BIOS. This method provides multiple functions that can be invoked
//   by the display driver. The only required function is "Verify Interface". All other
//   functions are optional. The general format of this method is as follows.
//
// Arguments:
//
//   Arg0 (ACPI_INTEGER): Function code
//   Arg1 (ACPI_BUFFER): Parameter buffer, 256 bytes
//
// Output:
//
//   (ACPI_BUFFER): 256 bytes.
//
    Method (ATIF, 2, Serialized)
    {
//
// Function 0 (Verify Interface)
//
//   This function provides interface version and bit vectors indicating functions and
//   notifications supported by System BIOS. It is a required function, if any other 
//   ATIF functions or any ATIF notifications are supported by System BIOS.
//
// Arguments:
//
//   Arg0: Function code
//   Arg1: None
//
// Output:
//
//   Structure data containing the following info:
//
// ===================================================================================================
// ||       Field       || Format ||                        Description                             ||
// ===================================================================================================
// || Structure Size    || WORD   || Structure size in bytes including the Structure Size field.    ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Interface         || WORD   || 16-bit interface version. Must be set to 1.                    ||
// || Version           ||        ||                                                                ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Supported         || DWORD  || Bit 0: Display switch request is supported                     ||
// || Notifications     ||        || Bit 1: Expansion mode change request is supported              ||
// || Mask              ||        || Bit 2: Thermal state change request is supported               ||
// ||                   ||        || Bit 3: Forced power state change request is supported          ||
// ||                   ||        || Bit 4: System power source change request is supported         ||
// ||                   ||        || Bit 5: Display configuration change request is supported       ||
// ||                   ||        || Bit 6: PowerXpress graphics switch toggle request is supported ||
// ||                   ||        || Bit 7: Panel brightness change request is supported            ||
// ||                   ||        || Bit 8: Discrete GPU display connect/disconnect event is        ||
// ||                   ||        ||        supported.                                              ||
// ||                   ||        || Bits 31-9: Reserved (must be zero)                             ||
// ||                   ||        || If supported notifications mask indicates that a notification  ||
// ||                   ||        || is not supported, it will be ignored when function             ||
// ||                   ||        || "Get System BIOS Requests" is executed.                        ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Supported         || DWORD  || Bit vector providing supported functions information. Each bit ||
// || Functions Bit     ||        || marks support for one specific function of the ATIF method.    ||
// || Vector            ||        || Bit n, if set, indicates that Function n+1 is supported.       ||
// ===================================================================================================
//
        If (LEqual (Arg0, ATIF_VERIFY_INTERFACE))
        {
            Name (TMP0, Buffer(256) {0x00})
            CreateWordField  (TMP0, 0, F0SS)
            CreateWordField  (TMP0, 2, F0IV)
            CreateDwordField (TMP0, 4, F0SN)
            CreateDwordField (TMP0, 8, F0SF)

            Store (0x000C, F0SS) // Size of return structure
            Store (0x0001, F0IV) // Interface version
            Store (Zero, F0SN)
            Store (Zero, F0SF)

            Return (TMP0)
        }

        Return (Zero)
    } // ATIF method End#

//
// Switchable Graphics ACPI Control
// Methods and Notifications
// Rev. 0.27
//
// ATI PowerXpress (PX) Contrl Method: ATPX
//
//   This is PowerXpress specific Control Method used by integrated graphics or discrete
//   graphics driver on PowerXpress enabled platforms. The existence of this method indicates
//   support for PowerXpress. This method provides multiple functions that can be
//   invoked by the display driver. The general format of this method is as follows.
//
// Arguments:
//
//     Arg0 (ACPI_INTEGER): Function code
//     Arg1 (ACPI_BUFFER): Parameter buffer, 256 bytes
//
// Output:
//
//     (ACPI_BUFFER): 256 bytes.
//
// Arg0 parameter may define the following values (only lower 32bit used):
//
    Method (ATPX, 2, Serialized)
    {
//
// Function 0 (Verify PowerXpress Interface)
//
//   This function provides interface version and functions supported by PowerXpress System BIOS.
//   It is a required function, if any other ATPX functions are supported by System BIOS.
//
// Arguments:
//
//   Arg0: Function code
//   Arg1: None
//
// Output:
//
//   Structure data containing the following info:
//
// ===================================================================================================
// ||       Field       || Format ||                        Description                             ||
// ===================================================================================================
// || Structure Size    || WORD   || Structure size in bytes including the Structure Size field.    ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Interface         || WORD   || 16-bit interface version. Must be set to 1.                    ||
// || Version           ||        ||                                                                ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Supported         || DWORD  || Bit vector providing supported functions information. Each bit ||
// || Functions Bit     ||        || marks support for one specific function of the ATPX method.    ||
// || Vector            ||        || Bit n, if set, indicates that Function n+1 is supported.       ||
// ===================================================================================================
//
        If (LEqual (Arg0, PX_VERIFY_POWERXPRESS_INTERFACE))
        {
            Name (TMP1, Buffer(256) {0x00})
            CreateWordField  (TMP1, 0, F0SS)
            CreateWordField  (TMP1, 2, F0IV)
            CreateDwordField (TMP1, 4, F0SF)

            Store (0x08, F0SS)
            Store (One, F0IV)
            //
            // Support function 1 and 2
            //
            Store (0x00000003, F0SF)

            Return (TMP1)
        }
//
// Function 1 (Get PowerXpress Parameters)
//
//   This function retrieves various PowerXpress related platform parameters. It is assumed
//   that LCD1 display output signals are always multiplexed.
//   It is a required function if any display multiplexers are supported on a given platform.
//
// Arguments:
//
//   Arg0: Function code
//   Arg1: None
//
// Output:
//
//   Structure data containing the following info:
//
// ===================================================================================================
// ||       Field       || Format ||                        Description                             ||
// ===================================================================================================
// || Structure Size    || WORD   || Structure size in bytes including the Structure Size field.    ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Valid Flags Mask  || DWORD  || Indicates which of the "Flags" bits are valid.                 ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Flags             || DWORD  || Bits 6-0: Ignored.                                             ||
// ||                   ||        || Bit 7: Indicates that a "dynamic" PX scheme is supported.      ||
// ||                   ||        || Bit 8: Reserved.                                               ||
// ||                   ||        || Bit 9: Indicates that fixed scheme is not supported,           ||
// ||                   ||        ||        if set to one.                                          ||
// ||                   ||        || Bit 10: Indicates that full dGPU power off in "dynamic" scheme ||
// ||                   ||        ||         is supported, if set to one.                           ||
// ||                   ||        || Bit 11: Indicates that discrete graphics must be powered on    ||
// ||                   ||        ||         while a monitor is connected to discrete graphics      ||
// ||                   ||        ||         connector, if set to one.                              ||
// ||                   ||        || Bit 12: Indicates that discrete graphics can drive the display ||
// ||                   ||        ||         outputs(local dGPU displays are supported),            ||
// ||                   ||        ||         if set to one.                                         ||
// ||                   ||        || Bit 13: Indicates that long idle detection is disabled,        ||
// ||                   ||        ||         if set to one.                                         ||
// ||                   ||        || Bit 14: Indicates that Windows 8.1 "Hybrid Graphics" is        ||
// ||                   ||        ||         required (supported), if set to one.                   ||
// ||                   ||        || Bits 31-15: Reserved (must be zero).                           ||
// ===================================================================================================
//
        If (LEqual (Arg0, PX_GET_POWERXPRESS_PARAMETERS))
        {
            Name (TMP2, Buffer(256) {0x00})
            CreateWordField  (TMP2, 0, F1SS)
            CreateDwordField (TMP2, 2, F1VM)
            CreateDwordField (TMP2, 6, F1FG)

            Store (0x000A, F1SS)  //Structure size of return package
            Store (0x00004680, F1VM) // Mask used for valid bit fields
            Store (0x00004680, F1FG) // Actual PX parameters field

            Return (TMP2)
        }
//
// Function 2 (Power Control)
//
//   This function powers on/off the discrete graphics.
//   It is a required function.
//
// Arguments:
//
//   Arg0: Function code
//   Arg1: Structure data containing the following info:
//
// ===================================================================================================
// ||       Field       || Format ||                        Description                             ||
// ===================================================================================================
// || Structure Size    || WORD   || Structure size in bytes including the Structure Size field.    ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Power State       || BYTE   || Indicates which of the "Flags" bits are valid.                 ||
// ||-------------------||--------||----------------------------------------------------------------||
// || Flags             || DWORD  || Bit 0:                                                         ||
// ||                   ||        || 0 - Power off discrete graphics.                               ||
// ||                   ||        || 1 - Power on discrete graphics.                                ||
// ||                   ||        || Bits 7-1: Reserved (must be zero).                             ||
// ===================================================================================================
//
// Output:
//
//   None.
//
        If (LEqual (Arg0, PX_POWER_CONTROL))
        {
            CreateWordField (Arg1, 0, FN2S)
            CreateByteField (Arg1, 2, DGPR)

            If (LEqual (DGPR, Zero))  // Powers off discrete graphics
            {
                DGPU_SCOPE._OFF ()
            }
            If (LEqual (DGPR, One))  // Powers on discrete graphics
            {
                DGPU_SCOPE._ON ()
            }
            Return (Zero)
        }

        Return (Zero)
    } // ATPX method End#
//
// ATI PowerXpress (PX) get ROM Method: ATRM
//
//   This is PowerXpress specific Control Method used by the discrete graphics driver on
//   PowerXpress enabled platforms to get a runtime modified copy of the discrete graphics
//   device ROM data (Video BIOS). The ATRM method definition is identical to the standard
//   ACPI _ROM method except that the ATRM method is defined in the _VGA namespace of the
//   integrated graphics device.
//
//   This function is required unless another method of exposing non-POSTed Video BIOS
//   image to the driver is supported by System BIOS: copying Video BIOS image to Video
//   Memory after Video BIOS obtained run-time parameters from System BIOS or when there
//   are no run-time parameters (Video BIOS image has hard-coded platform dependent data)
//   and Video BIOS ROM image can be accessed directly by the driver.
//
// Arguments:
//
//     Arg0: Offset of the graphics device ROM data.
//     Arg1: Size of the buffer to fill in (up to 4K).
//
// Output:
//
//     Buffer of bytes.
//
    Method (ATRM, 2, Serialized)
    {
        Store (Arg0, Local0)
        Store (Arg1, Local1)

        Name (VROM, Buffer(Local1) {0x00}) // Create 4K buffer to return to DD

        If (LGreater (Local1, 0x1000))
        {
            Store (0x1000, Local1) // Return dummy buffer if asking for more than 4K
        }

        If (LGreater (Arg0, DGPU_SCOPE.RVBS))
        {
            Return (VROM) // Return dummy buffer if asking beyond VBIOS image
        }

        Add (Arg0, Arg1, Local2)

        If (LGreater (Local2, DGPU_SCOPE.RVBS)) // If requested BASE+LEN > VBIOS image size
        {
            Subtract (DGPU_SCOPE.RVBS, Local0, Local1) // Limit length to the final chunk of VBIOS image
        }

        Divide (Local0, 0x8000, Local3, Local4)  // (Dividend, Divisor, Remainder, Result)

        Switch (Local4)
        {
            Case (0)
            {
                Store (DGPU_SCOPE.VBS1, Local5)
            }
            Case (1)
            {
                Store (DGPU_SCOPE.VBS2, Local5)
            }
            Case (2)
            {
                Store (DGPU_SCOPE.VBS3, Local5)
            }
            Case (3)
            {
                Store (DGPU_SCOPE.VBS4, Local5)
            }
            Case (4)
            {
        	      Store (DGPU_SCOPE.VBS5, Local5)
            }
            Case (5)
            {
        	      Store (DGPU_SCOPE.VBS6, Local5)
            }
            Case (6)
            {
        	      Store (DGPU_SCOPE.VBS7, Local5)
            }
            Case (7)
            {
        	      Store (DGPU_SCOPE.VBS8, Local5)
            }
        }
        Multiply (0x8000, Local4, Local4)   // (Multiplicand, Multiplier, Result)
        Subtract (Local0, Local4, Local0) 
        
        Mid (Local5, Local0, Local1, VROM)

        Return (VROM)
    }
} // iGPU Scope End#
