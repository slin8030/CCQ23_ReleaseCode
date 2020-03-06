/*
 * (C) Copyright 2016 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2016 Compal Electronics, Inc.. All rights reserved.

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
  --------  --------  --------     ----------------------------------------------------
  1.00                Daiven         Init version and add VEEPROM data to SMBIOS.

*/
#include "T66ConfigDxe.h"
#include "UpdateSmbios.h"

/**
  Creates and returns a notification event and registers that event with all the protocol
  instances specified by ProtocolGuid.

  This function causes the notification function to be executed for every protocol of type
  ProtocolGuid instance that exists in the system when this function is invoked. In addition,
  every time a protocol of type ProtocolGuid instance is installed or reinstalled, the notification
  function is also executed. This function returns the notification event that was created.
  If ProtocolGuid is NULL, then ASSERT().
  If NotifyTpl is not a legal TPL value, then ASSERT().
  If NotifyFunction is NULL, then ASSERT().
  If Registration is NULL, then ASSERT().
  If Event is NULL, then ASSERT().


  @param[in]  ProtocolGuid    Supplies GUID of the protocol upon whose installation the event is fired.
  @param[in]  NotifyTpl       Supplies the task priority level of the event notifications.
  @param[in]  NotifyFunction  Supplies the function to notify when the event is signaled.
  @param[in]  NotifyContext   The context parameter to pass to NotifyFunction.
  @param[out] Registration    A pointer to a memory location to receive the registration value.
                              This value is passed to LocateHandle() to obtain new handles that
                              have been added that support the ProtocolGuid-specified protocol.
  @param[out] Event           A pointer to a memory location to receive the registration value.

  @return EFI_SUCCESS: The notification event that was created.
          EFI_INVALID_PARAMETERerror: ProtocolGuid/NotifyFunction/Registration invalid parameter NULL
          other: Create or register notify error
**/
EFI_STATUS
EFIAPI
OemCreateProtocolNotifyEvent (
  IN  EFI_GUID           *ProtocolGuid,
  IN  EFI_TPL             NotifyTpl,
  IN  EFI_EVENT_NOTIFY    NotifyFunction,
  IN  VOID               *NotifyContext,  OPTIONAL
  OUT VOID              **Registration,
  OUT EFI_EVENT          *Event
  )
{
  EFI_STATUS  Status;

  if ((ProtocolGuid == NULL) || (NotifyFunction == NULL) || (Registration == NULL) || (Event == NULL)) {
    DEBUG ((EFI_D_ERROR, "DxeOemConfig CreateProtocolNotifyEvent input parameter error\n"));
    Status = EFI_INVALID_PARAMETER;
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  //
  // Create the event
  //

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  NotifyTpl,
                  NotifyFunction,
                  NotifyContext,
                  Event
                  );
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "DxeOemConfig Creat Event Error, Status = %r\n", Status));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  //
  // Register for protocol notifications on this event
  //

  Status = gBS->RegisterProtocolNotify (
                  ProtocolGuid,
                  *Event,
                  Registration
                  );

  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "DxeOemConfig Register error, Status = %r\n", Status));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  return Status;
}

/**
  The user Entry Point for Oem config platform setting. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
T66ConfigEntryPoint(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  // Update Smbios data
  //
  Status = UpdateSmbios (ImageHandle, SystemTable);

  return Status;
}

