/** @file
  When user enable PxeToLan, install one protocol to indicate Native driver load or not

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _TELNET_PROTOCOL_H_
#define _TELNET_PROTOCOL_H_

#define TELNET_SERVICE_BINDING_PROTOCOL_GUID \
  { 0xb81b2418, 0xca13, 0x4f5b, {0xa9, 0xca, 0xa9, 0xf5, 0x02, 0x31, 0x44, 0xd9} }

#define TELNET_PROTOCOL_GUID \
  { 0x65e1ceee, 0xced0, 0x4ffd, {0x93, 0x6d, 0x2d, 0x1a, 0x6b, 0xa3, 0xb0, 0xcf} }

#define MAX_TELNET_CONNECTIONS          5

typedef struct {
  BOOLEAN           UseDhcp;
  UINT16            PortNumber;
  EFI_IPv4_ADDRESS  LocalIp;
  EFI_IPv4_ADDRESS  SubnetMask;
  EFI_IPv4_ADDRESS  Gateway;
  EFI_IPv4_ADDRESS  AdminIp;
} TELNET_CONFIG_DATA;

/*
///
/// TELNET_START_TOKEN
///
typedef struct {
  /// 
  /// The Event to signal after the connection is established and Status field is updated 
  /// by the Telnet Protocol driver. The type of Event must be 
  /// EVENT_NOTIFY_SIGNAL, and its Task Priority Level (TPL) must be lower than or 
  /// equal to TPL_CALLBACK. If it is set to NULL, this function will not return  until the 
  /// function completes.
  /// 
  EFI_EVENT                            Event;
  //
  // The new created connection instance.
  //
  VOID                                *TelConn;
  ///
  /// The variable to receive the result of the completed operation.
  /// 
  EFI_STATUS                           Status;
} TELNET_START_TOKEN;
*/

typedef enum {
  TELNET_CONNECTION_OPEN,
  TELNET_CONNECTION_CLOSE,
} TELNET_CALLBACK_ACTION;


typedef struct _TELNET_PROTOCOL TELNET_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *TELNET_CALLBACK) (
  IN TELNET_PROTOCOL      *This,
  IN VOID                 *Connection,
  IN UINTN                Action,
  IN VOID                 *TerminalInfo
  );

typedef
EFI_STATUS
(EFIAPI *TELNET_CONFIGURE) (
  IN TELNET_PROTOCOL      *This,
  IN TELNET_CONFIG_DATA   *Config
  );

typedef
EFI_STATUS
(EFIAPI *TELNET_START) (
  IN  TELNET_PROTOCOL      *This,
  IN  TELNET_CALLBACK      Callback
  );
    
typedef
EFI_STATUS
(EFIAPI *TELNET_READ) (
  IN  TELNET_PROTOCOL      *This,
  IN  VOID                 *Connection, 
  IN OUT UINTN             *BufferSize,
  IN OUT VOID              *Buffer
  );
  
typedef
EFI_STATUS
(EFIAPI *TELNET_WRITE) (
  IN  TELNET_PROTOCOL      *This,
  IN  VOID                 *Connection,
  IN OUT UINT32            BufferSize,
  IN OUT VOID              *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *TELNET_RESET) (
  IN  TELNET_PROTOCOL      *This,
  IN  VOID                 *Connection
  );

typedef
BOOLEAN
(EFIAPI *TELNET_BUF_IS_EMPTY) (
  IN  TELNET_PROTOCOL      *This,
  IN  VOID                 *Connection
  );


typedef
EFI_HANDLE
(EFIAPI *TELNET_GET_TCP_HANDLE) (
  IN  TELNET_PROTOCOL      *This,
  IN  VOID                 *Connection
  );

struct _TELNET_PROTOCOL {
  TELNET_CONFIGURE        Configure;
  TELNET_START            Start;
  TELNET_READ             Read;
  TELNET_WRITE            Write;
  TELNET_RESET            Reset;
  TELNET_BUF_IS_EMPTY     BufIsEmpty;
  TELNET_GET_TCP_HANDLE   GetTcpHandle;
};

extern EFI_GUID gTelnetServiceBindingProtocolGuid;
extern EFI_GUID gTelnetProtocolGuid;

#endif //_TELNET_PROTOCOL_H_
