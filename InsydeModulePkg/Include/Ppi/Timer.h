/** @file
  This file defines the Timer Architectural PPI.
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _ARCH_PPI_TIMER_H_
#define _ARCH_PPI_TIMER_H_

//
// Global ID for the Timer Architectural PPI
//
#define H2O_TIMER_ARCH_PPI_GUID \
  { 0xe61293c, 0xa5ad, 0x4a77, { 0xbb, 0xa4, 0x8b, 0x8d, 0xf0, 0x12, 0x1d, 0x4e } }

//
// Global ID for the Timer event
//
#define H2O_TIMER_EVENT_PPI_GUID \
  { 0x19324b9c, 0xe7f8, 0x44d9, {0x94, 0x75, 0xa6, 0x34, 0x7e, 0xce, 0x8c, 0x60 } }
  
//
// Declare forward reference for the Timer Architectural PPI
//
typedef struct _H2O_TIMER_ARCH_PPI   H2O_TIMER_ARCH_PPI;

/**
  This function of this type is called when a timer interrupt fires.
  @param[in]  Time   Time since the last timer interrupt in 100 ns units. This will
                     typically be TimerPeriod, but if a timer interrupt is missed, and the
                     H2O_TIMER_ARCH_PPI module can detect missed interrupts, then Time
                     will contain the actual amount of time since the last interrupt.

  @retval     None.

**/
typedef
VOID
(EFIAPI *H2O_TIMER_NOTIFY)(
  IN UINT64  Time
  );

/**
  This function registers the handler NotifyFunction so it is called every time 
  the timer interrupt fires.  It also passes the amount of time since the last 
  handler call to the NotifyFunction.  If NotifyFunction is NULL, then the 
  handler is unregistered.  If the handler is registered, then EFI_SUCCESS is 
  returned.  If the CPU does not support registering a timer interrupt handler, 
  then EFI_UNSUPPORTED is returned.  If an attempt is made to register a handler 
  when a handler is already registered, then EFI_ALREADY_STARTED is returned.  
  If an attempt is made to unregister a handler when a handler is not registered, 
  then EFI_INVALID_PARAMETER is returned.  If an error occurs attempting to 
  register the NotifyFunction with the timer interrupt, then EFI_DEVICE_ERROR 
  is returned.

  @param[in]  This              The H2O_TIMER_ARCH_PPI instance.
  @param[in]  NotifyFunction    The function to call when a timer interrupt fires. 
                                NULL will unregister the handler.

  @retval EFI_SUCCESS           The timer handler was registered.
  @retval EFI_UNSUPPORTED       The platform does not support timer interrupts.
  @retval EFI_ALREADY_STARTED   NotifyFunction is not NULL, and a handler is already
                                registered.
  @retval EFI_INVALID_PARAMETER NotifyFunction is NULL, and a handler was not
                                previously registered.
  @retval EFI_DEVICE_ERROR      The timer handler could not be registered.

**/
typedef 
EFI_STATUS
(EFIAPI *H2O_TIMER_REGISTER_HANDLER)(
  IN H2O_TIMER_ARCH_PPI         *This,
  IN H2O_TIMER_NOTIFY           NotifyFunction
);

/**
  This function adjusts the period of timer interrupts to the value specified 
  by TimerPeriod.  If the timer period is updated, then the selected timer 
  period is stored in EFI_TIMER.TimerPeriod, and EFI_SUCCESS is returned.  If 
  the timer hardware is not programmable, then EFI_UNSUPPORTED is returned.  
  If an error occurs while attempting to update the timer period, then the 
  timer hardware will be put back in its state prior to this call, and 
  EFI_DEVICE_ERROR is returned.  If TimerPeriod is 0, then the timer interrupt 
  is disabled.  This is not the same as disabling the CPU's interrupts.  
  Instead, it must either turn off the timer hardware, or it must adjust the 
  interrupt controller so that a CPU interrupt is not generated when the timer 
  interrupt fires. 

  @param[in] This             The H2O_TIMER_ARCH_PPI instance.
  @param[in] TimerPeriod      The rate to program the timer interrupt in 100 nS units. If
                              the timer hardware is not programmable, then EFI_UNSUPPORTED is
                              returned. If the timer is programmable, then the timer period
                              will be rounded up to the nearest timer period that is supported
                              by the timer hardware. If TimerPeriod is set to 0, then the
                              timer interrupts will be disabled.

  @retval    EFI_SUCCESS           The timer period was changed.
  @retval    EFI_UNSUPPORTED       The platform cannot change the period of the timer interrupt.
  @retval    EFI_DEVICE_ERROR      The timer period could not be changed due to a device error.

**/
typedef 
EFI_STATUS
(EFIAPI *H2O_TIMER_SET_TIMER_PERIOD)(
  IN H2O_TIMER_ARCH_PPI         *This,
  IN UINT64                     TimerPeriod
  );

/**
  This function retrieves the period of timer interrupts in 100 ns units, 
  returns that value in TimerPeriod, and returns EFI_SUCCESS.  If TimerPeriod 
  is NULL, then EFI_INVALID_PARAMETER is returned.  If a TimerPeriod of 0 is 
  returned, then the timer is currently disabled.

  @param[in]   This             The H2O_TIMER_ARCH_PPI instance.
  @param[out]  TimerPeriod      A pointer to the timer period to retrieve in 100 ns units. If
                                0 is returned, then the timer is currently disabled.

  @retval      EFI_SUCCESS           The timer period was returned in TimerPeriod.
  @retval      EFI_INVALID_PARAMETER TimerPeriod is NULL.

**/
typedef 
EFI_STATUS
(EFIAPI *H2O_TIMER_GET_TIMER_PERIOD)(
  IN  H2O_TIMER_ARCH_PPI          *This,
  OUT UINT64                      *TimerPeriod
  );

/**
  This function generates a soft timer interrupt. If the platform does not support soft 
  timer interrupts, then EFI_UNSUPPORTED is returned. Otherwise, EFI_SUCCESS is returned. 
  If a handler has been registered through the H2O_TIMER_ARCH_PPI.RegisterHandler() 
  service, then a soft timer interrupt will be generated. If the timer interrupt is 
  enabled when this service is called, then the registered handler will be invoked. The 
  registered handler should not be able to distinguish a hardware-generated timer 
  interrupt from a software-generated timer interrupt.

  @param[in]  This                  The H2O_TIMER_ARCH_PPI instance.

  @retval     EFI_SUCCESS           The soft timer interrupt was generated.
  @retval     EFI_UNSUPPORTEDT      The platform does not support the generation of soft timer interrupts.

**/
typedef 
EFI_STATUS
(EFIAPI *H2O_TIMER_GENERATE_SOFT_INTERRUPT)(
  IN H2O_TIMER_ARCH_PPI    *This
  );


//
// This protocol provides the services to initialize a periodic timer 
// interrupt, and to register a handler that is called each time the timer
// interrupt fires.  It may also provide a service to adjust the rate of the
// periodic timer interrupt.  When a timer interrupt occurs, the handler is 
// passed the amount of time that has passed since the previous timer 
// interrupt.
//
struct _H2O_TIMER_ARCH_PPI {
  H2O_TIMER_REGISTER_HANDLER          RegisterHandler;
  H2O_TIMER_SET_TIMER_PERIOD          SetTimerPeriod;
  H2O_TIMER_GET_TIMER_PERIOD          GetTimerPeriod;
  H2O_TIMER_GENERATE_SOFT_INTERRUPT   GenerateSoftInterrupt;
};

extern EFI_GUID gH2OTimerArchPpiGuid;
extern EFI_GUID gH2OTimerEventPpiGuid;

#endif
