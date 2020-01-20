/** @file
  ACPI DSDT table for USB Type C

@copyright
 Copyright (c) 2015-2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/

DefinitionBlock (
  "UsbC.aml",
  "SSDT",
  2,
  "Intel_",
  "UsbCTabl",
  0x1000
  )
{
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  //
  // CRB EC object
  //
  External(\_SB.PCI0.LPCB.H_EC.MGI0, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI1, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI2, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI3, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI4, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI5, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI6, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI7, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI8, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGI9, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGIA, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGIB, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGIC, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGID, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGIE, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGIF, IntObj)

  External(\_SB.PCI0.LPCB.H_EC.CTL0, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CTL1, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CTL2, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CTL3, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CTL4, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CTL5, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CTL6, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CTL7, IntObj)

  External(\_SB.PCI0.LPCB.H_EC.MGO0, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO1, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO2, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO3, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO4, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO5, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO6, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO7, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO8, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGO9, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGOA, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGOB, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGOC, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGOD, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGOE, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.MGOF, IntObj)

  External(\_SB.PCI0.LPCB.H_EC.CCI0, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CCI1, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CCI2, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.CCI3, IntObj)

  External(\_SB.PCI0.LPCB.H_EC.ECMD, MethodObj)
  External(\_SB.PCI0.LPCB.H_EC.ECRD, MethodObj)  // EC Read Method
  External(\_SB.PCI0.LPCB.H_EC.ECWT, MethodObj)  // EC Write Method
  External(\_SB.PCI0.LPCB.H_EC.ECWR, MethodObj)  // EC Write Method
   External(\_SB.PCI0.LPCB.H_EC.ECM1, MethodObj)
#else
  //
  // OEM EC object
  //
#endif
#endif
//[-end-161206-IB07400822-modify]//
  

  External(UBCB)
  External(P8XH, MethodObj)
  External(BID, IntObj)  // BoardId
  External(\_SB.PCI0.XHC.RHUB, DeviceObj)
  External(\_SB.PCI0.XHC.RHUB.TPLD, MethodObj)
  External(\_SB.PCI0.XHC.RHUB.TUPC, MethodObj)
  External(ETYC, IntObj)

  Scope (\_SB)
  {
    Device(UBTC)  // USB type C device
    {
      Name (_HID, EISAID("USBC000"))
      Name (_CID, EISAID("PNP0CA0"))
      Name (_UID, 0)
      Name (_DDN, "USB Type C")
      Name (_ADR, 0x0)

      Name (_DEP, Package ()  // _DEP: Dependencies
      {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
            //
            // CRB EC DEP
            //
            \_SB.PCI0.LPCB.H_EC
#else
            //
            // OEM EC DEP
            //
#endif
#endif
//[-end-161206-IB07400822-modify]//
      }) 
      
      Name (CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x00000000, 0x1000, USBR)
      })

      Device (CR01) // USB Type C Connector
      {
        Name (_ADR, 0x0)
        Method (_PLD)  {  Return ( \_SB.PCI0.XHC.RHUB.TPLD(1,1) )  }
      }

      Method(_CRS,0, Serialized)
      {
        CreateDWordField (CRS, \_SB.UBTC.USBR._BAS, CBAS)
        Store (UBCB, CBAS)
        Return(CRS)
      }

      Method(_STA,0)
      {
        If (LEqual (ETYC, 1)) {
          Return(0xF)
        }
        Return(0x0)
      }

      OperationRegion (USBC, SystemMemory, UBCB, 0x38)  // 56 bytes Opm Buffer
      Field(USBC,ByteAcc,Lock,Preserve)
      {
        VER1, 8,
        VER2, 8,
        RSV1, 8,
        RSV2, 8,
        Offset(4),
        CCI0, 8,  //  PPM->OPM CCI indicator
        CCI1, 8,
        CCI2, 8,
        CCI3, 8,

        CTL0, 8,  //  OPM->PPM Control message
        CTL1, 8,
        CTL2, 8,
        CTL3, 8,
        CTL4, 8,
        CTL5, 8,
        CTL6, 8,
        CTL7, 8,

        // USB Type C Mailbox Interface
        MGI0, 8,  //  PPM->OPM Message In
        MGI1, 8,
        MGI2, 8,
        MGI3, 8,
        MGI4, 8,
        MGI5, 8,
        MGI6, 8,
        MGI7, 8,
        MGI8, 8,
        MGI9, 8,
        MGIA, 8,
        MGIB, 8,
        MGIC, 8,
        MGID, 8,
        MGIE, 8,
        MGIF, 8,

        MGO0, 8,  //  OPM->PPM Message Out
        MGO1, 8,
        MGO2, 8,
        MGO3, 8,
        MGO4, 8,
        MGO5, 8,
        MGO6, 8,
        MGO7, 8,
        MGO8, 8,
        MGO9, 8,
        MGOA, 8,
        MGOB, 8,
        MGOC, 8,
        MGOD, 8,
        MGOE, 8,
        MGOF, 8,

      }  // end of Field

      Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj} )
      {
        // Compare passed in UUID with supported UUID.
        If (LEqual(Arg0, ToUUID ("6f8398c2-7ca4-11e4-ad36-631042b5008f")))  // UUID for USB type C
        {
          Switch (ToInteger(Arg2))  // Arg2:  0 for query, 1 for write and 2 for read
          {
            Case (0)
            {
              Return (Buffer() {0x07}) // 2 functions defined other than Query.
            }

            Case (1)  // OPM write to EC
            {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
              //
              // CRB EC code
              //
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO0, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO0 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO1, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO1 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO2, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO2 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO3, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO3 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO4, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO4 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO5, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO5 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO6, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO6 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO7, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO7 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO8, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO8 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGO9, RefOf(  \_SB.PCI0.LPCB.H_EC.MGO9 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGOA, RefOf(  \_SB.PCI0.LPCB.H_EC.MGOA ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGOB, RefOf(  \_SB.PCI0.LPCB.H_EC.MGOB ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGOC, RefOf(  \_SB.PCI0.LPCB.H_EC.MGOC ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGOD, RefOf(  \_SB.PCI0.LPCB.H_EC.MGOD ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGOE, RefOf(  \_SB.PCI0.LPCB.H_EC.MGOE ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( MGOF, RefOf(  \_SB.PCI0.LPCB.H_EC.MGOF ) )

              \_SB.PCI0.LPCB.H_EC.ECWR( CTL0, RefOf(  \_SB.PCI0.LPCB.H_EC.CTL0 ) )
              \_SB.PCI0.LPCB.H_EC.ECWR( CTL1, RefOf(  \_SB.PCI0.LPCB.H_EC.CTL1 ) )
              \_SB.PCI0.LPCB.H_EC.ECWR( CTL2, RefOf(  \_SB.PCI0.LPCB.H_EC.CTL2 ) )
              \_SB.PCI0.LPCB.H_EC.ECWR( CTL3, RefOf(  \_SB.PCI0.LPCB.H_EC.CTL3 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( CTL4, RefOf(  \_SB.PCI0.LPCB.H_EC.CTL4 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( CTL5, RefOf(  \_SB.PCI0.LPCB.H_EC.CTL5 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( CTL6, RefOf(  \_SB.PCI0.LPCB.H_EC.CTL6 ) )
              \_SB.PCI0.LPCB.H_EC.ECWT( CTL7, RefOf(  \_SB.PCI0.LPCB.H_EC.CTL7 ) )

              \_SB.PCI0.LPCB.H_EC.ECM1 (0xE0)
#else
              //
              // OEM EC code
              //
#endif
#endif
//[-end-161206-IB07400822-modify]//
              P8XH(0,0xE0)
            }

            Case (2)  // OPM read from EC
            {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
              //
              // CRB EC code
              //
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI0) ), MGI0 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI1) ), MGI1 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI2) ), MGI2 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI3) ), MGI3 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI4) ), MGI4 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI5) ), MGI5 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI6) ), MGI6 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI7) ), MGI7 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI8) ), MGI8 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGI9) ), MGI9 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGIA) ), MGIA )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGIB) ), MGIB )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGIC) ), MGIC )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGID) ), MGID )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGIE) ), MGIE )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.MGIF) ), MGIF )

              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.CCI0) ), CCI0 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.CCI1) ), CCI1 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.CCI2) ), CCI2 )
              Store(  \_SB.PCI0.LPCB.H_EC.ECRD( RefOf (\_SB.PCI0.LPCB.H_EC.CCI3) ), CCI3 )
#else
              //
              // OEM EC code
              //
#endif
#endif
//[-end-161206-IB07400822-modify]//
            }
          }  // End switch
        }   // End UUID check
        Return (0)
      }  // End _DSM Method
    }  // end of Device
  } // end \_SB scope
} // end SSDT
