/** @file
  This file defines the CPU Architectural PPI.
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

#ifndef _H2O_CPU_ARCH_PPI_H_
#define _H2O_CPU_ARCH_PPI_H_

#include <Protocol/Cpu.h>

#define H2O_CPU_ARCH_PPI_GUID \
  { \
    0xa0435fa4, 0x2387, 0x4b31, { 0x86, 0x79, 0x98, 0xc9, 0x32, 0xc7, 0xdb, 0xbb } \
  }

typedef struct _H2O_CPU_ARCH_PPI  H2O_CPU_ARCH_PPI;

/**
  H2O_CPU_INTERRUPT_HANDLER that is called when a processor interrupt occurs.

  @param[in]  InterruptType    Defines the type of interrupt or exception that
                               occurred on the processor.This parameter is processor architecture specific.
  @param[in]  SystemContext    A pointer to the processor context when
                               the interrupt occurred on the processor.

  @return     None

**/
typedef
VOID
(EFIAPI *H2O_CPU_INTERRUPT_HANDLER)(
  IN CONST  EFI_EXCEPTION_TYPE  InterruptType,
  IN CONST  EFI_SYSTEM_CONTEXT  SystemContext
  );

/**
  This function flushes the range of addresses from Start to Start+Length 
  from the processor's data cache. If Start is not aligned to a cache line 
  boundary, then the bytes before Start to the preceding cache line boundary 
  are also flushed. If Start+Length is not aligned to a cache line boundary, 
  then the bytes past Start+Length to the end of the next cache line boundary 
  are also flushed. The FlushType of EfiCpuFlushTypeWriteBackInvalidate must be 
  supported. If the data cache is fully coherent with all DMA operations, then 
  this function can just return EFI_SUCCESS. If the processor does not support 
  flushing a range of the data cache, then the entire data cache can be flushed.

  @param[in]  This              The H2O_CPU_ARCH_PPI instance.
  @param[in]  Start             The beginning physical address to flush from the processor's data
                                cache.
  @param[in]  Length            The number of bytes to flush from the processor's data cache. This
                                function may flush more bytes than Length specifies depending upon
                                the granularity of the flush operation that the processor supports.
  @param[in]  FlushType         Specifies the type of flush operation to perform.

  @retval     EFI_SUCCESS       The address range from Start to Start+Length was flushed from
                                the processor's data cache.
  @retval     EFI_UNSUPPORTEDT  The processor does not support the cache flush type specified
                                by FlushType.
  @retval     EFI_DEVICE_ERROR  The address range from Start to Start+Length could not be flushed
                                from the processor's data cache.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CPU_FLUSH_DATA_CACHE)(
  IN H2O_CPU_ARCH_PPI                   *This,
  IN EFI_PHYSICAL_ADDRESS               Start,
  IN UINT64                             Length,
  IN EFI_CPU_FLUSH_TYPE                 FlushType
  );


/**
  This function enables interrupt processing by the processor. 

  @param[in]  This                  The H2O_CPU_ARCH_PPI instance.

  @retval     EFI_SUCCESS           Interrupts are enabled on the processor.
  @retval     EFI_DEVICE_ERROR      Interrupts could not be enabled on the processor.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CPU_ENABLE_INTERRUPT)(
  IN H2O_CPU_ARCH_PPI              *This
  );


/**
  This function disables interrupt processing by the processor.

  @param[in]  This                  The H2O_CPU_ARCH_PPI instance.

  @retval     EFI_SUCCESS           Interrupts are disabled on the processor.
  @retval     EFI_DEVICE_ERROR      Interrupts could not be disabled on the processor.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CPU_DISABLE_INTERRUPT)(
  IN H2O_CPU_ARCH_PPI              *This
  );


/**
  This function retrieves the processor's current interrupt state a returns it in 
  State. If interrupts are currently enabled, then TRUE is returned. If interrupts 
  are currently disabled, then FALSE is returned.

  @param[in]  This                  The H2O_CPU_ARCH_PPI instance.
  @param[out] State                 A pointer to the processor's current interrupt state. Set to TRUE if
                                    interrupts are enabled and FALSE if interrupts are disabled.

  @retval     EFI_SUCCESS           The processor's current interrupt state was returned in State.
  @retval     EFI_INVALID_PARAMETER State is NULL.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CPU_GET_INTERRUPT_STATE)(
  IN  H2O_CPU_ARCH_PPI             *This,
  OUT BOOLEAN                      *State
  );


/**
  This function generates an INIT on the processor. If this function succeeds, then the
  processor will be reset, and control will not be returned to the caller. If InitType is 
  not supported by this processor, or the processor cannot programmatically generate an 
  INIT without help from external hardware, then EFI_UNSUPPORTED is returned. If an error 
  occurs attempting to generate an INIT, then EFI_DEVICE_ERROR is returned.

  @param[in]  This             The H2O_CPU_ARCH_PPI instance.
  @param[in]  InitType         The type of processor INIT to perform.

  @retval     EFI_SUCCESS           The processor INIT was performed. This return code should never be seen.
  @retval     EFI_UNSUPPORTED       The processor INIT operation specified by InitType is not supported
                                    by this processor.
  @retval     EFI_DEVICE_ERROR      The processor INIT failed.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CPU_INIT)(
  IN H2O_CPU_ARCH_PPI              *This,
  IN EFI_CPU_INIT_TYPE             InitType
  );


/**
  This function registers and enables the handler specified by InterruptHandler for a processor 
  interrupt or exception type specified by InterruptType. If InterruptHandler is NULL, then the 
  handler for the processor interrupt or exception type specified by InterruptType is uninstalled. 
  The installed handler is called once for each processor interrupt or exception.

  @param[in]  This             The H2O_CPU_ARCH_PPI instance.
  @param[in]  InterruptType    A pointer to the processor's current interrupt state. Set to TRUE if interrupts
                               are enabled and FALSE if interrupts are disabled.
  @param[in]  InterruptHandler A pointer to a function of type EFI_CPU_INTERRUPT_HANDLER that is called
                               when a processor interrupt occurs. If this parameter is NULL, then the handler
                               will be uninstalled.

  @retval     EFI_SUCCESS           The handler for the processor interrupt was successfully installed or uninstalled.
  @retval     EFI_ALREADY_STARTED   InterruptHandler is not NULL, and a handler for InterruptType was
                                    previously installed.
  @retval     EFI_INVALID_PARAMETER InterruptHandler is NULL, and a handler for InterruptType was not
                                    previously installed.
  @retval     EFI_UNSUPPORTED       The interrupt specified by InterruptType is not supported.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CPU_REGISTER_INTERRUPT_HANDLER)(
  IN H2O_CPU_ARCH_PPI                   *This,
  IN EFI_EXCEPTION_TYPE                 InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER          InterruptHandler
  );


/**
  This function reads the processor timer specified by TimerIndex and returns it in TimerValue.

  @param[in]   This             The H2O_CPU_ARCH_PPI instance.
  @param[in]   TimerIndex       Specifies which processor timer is to be returned in TimerValue. This parameter
                                must be between 0 and NumberOfTimers-1.
  @param[out]  TimerValue       Pointer to the returned timer value.
  @param[out]  TimerPeriod      A pointer to the amount of time that passes in femtoseconds for each increment
                                of TimerValue. If TimerValue does not increment at a predictable rate, then 0 is
                                returned. This parameter is optional and may be NULL.

  @retval      EFI_SUCCESS           The processor timer value specified by TimerIndex was returned in TimerValue.
  @retval      EFI_DEVICE_ERROR      An error occurred attempting to read one of the processor's timers.
  @retval      EFI_INVALID_PARAMETER TimerValue is NULL or TimerIndex is not valid.
  @retval      EFI_UNSUPPORTED       The processor does not have any readable timers.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CPU_GET_TIMER_VALUE)(
  IN H2O_CPU_ARCH_PPI                   *This,
  IN UINT32                             TimerIndex,
  OUT UINT64                            *TimerValue,
  OUT UINT64                            *TimerPeriod OPTIONAL
  );


/**
  This function modifies the attributes for the memory region specified by BaseAddress and
  Length from their current attributes to the attributes specified by Attributes.

  @param[in]  This              The H2O_CPU_ARCH_PPI instance.
  @param[in]  BaseAddress       The physical address that is the start address of a memory region.
  @param[in]  Length            The size in bytes of the memory region.
  @param[in]  Attributes        The bit mask of attributes to set for the memory region.

  @retval     EFI_SUCCESS           The attributes were set for the memory region.
  @retval     EFI_ACCESS_DENIED     The attributes for the memory resource range specified by
                                    BaseAddress and Length cannot be modified.
  @retval     EFI_INVALID_PARAMETER Length is zero.
                                    Attributes specified an illegal combination of attributes that
                                    cannot be set together.
  @retval     EFI_OUT_OF_RESOURCES  There are not enough system resources to modify the attributes of
                                    the memory resource range.
  @retval     EFI_UNSUPPORTED       The processor does not support one or more bytes of the memory
                                    resource range specified by BaseAddress and Length.
                                    The bit mask of attributes is not support for the memory resource
                                    range specified by BaseAddress and Length.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CPU_SET_MEMORY_ATTRIBUTES)(
  IN  H2O_CPU_ARCH_PPI                  *This,
  IN  EFI_PHYSICAL_ADDRESS              BaseAddress,
  IN  UINT64                            Length,
  IN  UINT64                            Attributes
  );


//
// The H2O_CPU_ARCH_PPI is used to abstract processor-specific functions. 
// This includes flushing caches, enabling and disabling interrupts, hooking interrupt
// vectors and exception vectors, reading internal processor timers, resetting the processor, and
// determining the processor frequency.
//
struct _H2O_CPU_ARCH_PPI {
  H2O_CPU_FLUSH_DATA_CACHE            FlushDataCache;
  H2O_CPU_ENABLE_INTERRUPT            EnableInterrupt;
  H2O_CPU_DISABLE_INTERRUPT           DisableInterrupt;
  H2O_CPU_GET_INTERRUPT_STATE         GetInterruptState;
  H2O_CPU_INIT                        Init;
  H2O_CPU_REGISTER_INTERRUPT_HANDLER  RegisterInterruptHandler;
  H2O_CPU_GET_TIMER_VALUE             GetTimerValue;
  H2O_CPU_SET_MEMORY_ATTRIBUTES       SetMemoryAttributes;
  //
  // The number of timers that are available in a processor. The value in this 
  // field is a constant that must not be modified after the CPU Architectural 
  // PPI is installed. All consumers must treat this as a read-only field.
  //
  UINT32                              NumberOfTimers;
  //
  // The size, in bytes, of the alignment required for DMA buffer allocations. 
  // This is typically the size of the largest data cache line in the platform. 
  // The value in this field is a constant that must not be modified after the 
  // CPU Architectural PPI is installed. All consumers must treat this as 
  // a read-only field.  
  //
  UINT32                              DmaBufferAlignment;
};

extern EFI_GUID gH2OCpuArchPpiGuid;

#endif
