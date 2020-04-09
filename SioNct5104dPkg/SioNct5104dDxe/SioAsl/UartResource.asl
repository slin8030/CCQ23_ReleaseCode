/** @file

ASL code for SIO

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

  //*****************************************************
  //  Method _PRS:  Return Possible Resource Settings
  //*****************************************************
  Method(_PRS,0) {   // Possible Resource    
  
    Name(BUF0,ResourceTemplate() {
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3F8,0x3F8,0x01,0x08)
       IRQNoFlags() {4}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3F8,0x3F8,0x01,0x08)
       IRQNoFlags() {3}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3F8,0x3F8,0x01,0x08)
       IRQNoFlags() {5}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3F8,0x3F8,0x01,0x08)
       IRQNoFlags() {7}
       }

      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x388,0x388,0x01,0x08)
       IRQNoFlags() {6}
       }

      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3E8,0x3E8,0x01,0x08)
       IRQNoFlags() {4}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3E8,0x3E8,0x01,0x08)
       IRQNoFlags() {3}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3E8,0x3E8,0x01,0x08)
       IRQNoFlags() {5}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3E8,0x3E8,0x01,0x08)
       IRQNoFlags() {7}
       }

      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x3E8,0x3E8,0x01,0x08)
       IRQNoFlags() {6}
       }
      
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S         
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2F8,0x2F8,0x01,0x08)
       IRQNoFlags() {4}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2F8,0x2F8,0x01,0x08)
       IRQNoFlags() {3}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2F8,0x2F8,0x01,0x08)
       IRQNoFlags() {5}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2F8,0x2F8,0x01,0x08)
       IRQNoFlags() {7}
       }

      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2F8,0x2F8,0x01,0x08)
       IRQNoFlags() {6}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2E8,0x2E8,0x01,0x08)
       IRQNoFlags() {4}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2E8,0x2E8,0x01,0x08)
       IRQNoFlags() {3}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2E8,0x2E8,0x01,0x08)
       IRQNoFlags() {5}
       }
  
      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2E8,0x2E8,0x01,0x08)
       IRQNoFlags() {7}
       }         

      // Good configuration for Legacy O/S and sub-optional for ACPI O/S
      StartDependentFn(0,2)
      {
       IO(Decode16,0x2E8,0x2E8,0x01,0x08)
       IRQNoFlags() {6}
       }         
      
      EndDependentFn()
      })
    
    Return(BUF0)  // Return Buf0
  }  // End _PRS

