//
// This file contains 'Framework Code' and is licensed as such
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.
//
/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SmmIo.c
    
Abstract:

  SMM I/O access utility implementation file, for Ia32

--*/

//
// Include files
//
#include "Library/SmmIoLib.h"

UINT8
SmmIoRead8 (
  IN  UINT16    Address
  )
/*++

Routine Description:

  Do a one byte IO read

Arguments:

  Address - IO address to read
  
Returns: 
  
  Data read

--*/  
{
  UINT8   Buffer;
  
  ASSERT (mSmst);

  mSmst->SmmIo.Io.Read (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT8, 
                  Address, 
                  1, 
                  &Buffer
                  );
  return Buffer;
}

VOID
SmmIoWrite8 (
  IN  UINT16    Address,
  IN  UINT8     Data
  )
/*++

Routine Description:

  Do a one byte IO write

Arguments:

  Address - IO address to write
  Data    - Data to write

Returns: 

  None.

--*/
{
  ASSERT (mSmst);

  mSmst->SmmIo.Io.Write (
                    &mSmst->SmmIo, 
                    SMM_IO_UINT8, 
                    Address, 
                    1, 
                    &Data
                    );
}

UINT16
SmmIoRead16 (
  IN  UINT16    Address
  )
/*++

Routine Description:

  Do a two byte IO read

Arguments:

  Address - IO address to read
  
Returns: 
  
  Data read

--*/  
{
  UINT16      Buffer;
  
  ASSERT (mSmst);

  mSmst->SmmIo.Io.Read (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT16, 
                  Address,
                  1, 
                  &Buffer 
                  );
  return Buffer;
}

VOID
SmmIoWrite16 (
  IN  UINT16    Address,
  IN  UINT16    Data
  )
/*++

Routine Description:

  Do a two byte IO write

Arguments:

  Address - IO address to write
  Data    - Data to write

Returns: 

  None.

--*/
{
  ASSERT (mSmst);

  mSmst->SmmIo.Io.Write ( 
                    &mSmst->SmmIo, 
                    SMM_IO_UINT16, 
                    Address,
                    1, 
                    &Data 
                    );
}

UINT32
SmmIoRead32 (
  IN  UINT16    Address
  )
/*++

Routine Description:

  Do a four byte IO read

Arguments:

  Address - IO address to read
  
Returns: 
  
  Data read

--*/    
{
  UINT32        Buffer;

  ASSERT (mSmst);

  mSmst->SmmIo.Io.Read ( 
                    &mSmst->SmmIo, 
                    SMM_IO_UINT32, 
                    Address,
                    1, 
                    &Buffer 
                    );
  return Buffer;
}

VOID
SmmIoWrite32 (
  IN  UINT16    Address,
  IN  UINT32    Data
  )
/*++

Routine Description:

  Do a four byte IO write

Arguments:

  Address - IO address to write
  Data    - Data to write

Returns: 

  None.

--*/
{
  ASSERT (mSmst);

  mSmst->SmmIo.Io.Write ( 
                    &mSmst->SmmIo, 
                    SMM_IO_UINT32, 
                    Address,
                    1, 
                    &Data 
                    );
}

VOID
SmmMemWrite8 (
  IN  UINT64    Dest,
  IN  UINT8     Data
  )
/*++

Routine Description:

  Do a one byte Memory write

Arguments:

  Dest - Memory address to write
  Data - Data to write

Returns: 

  None.

--*/
{
  mSmst->SmmIo.Mem.Write (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT8, 
                  Dest, 
                  1, 
                  &Data
                  );

  return ;
}

UINT8
SmmMemRead8 (
  IN  UINT64    Dest
  )
/*++

Routine Description:

  Do a one byte Memory read

Arguments:

  Dest - Memory address to read

Returns: 

  Data read

--*/
{
  UINT8 Data;
  mSmst->SmmIo.Mem.Read (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT8, 
                  Dest, 
                  1, 
                  &Data
                  );

  return Data;
}

VOID
SmmMemWrite16 (
  IN  UINT64    Dest,
  IN  UINT16    Data
  )
/*++

Routine Description:

  Do a two bytes Memory write

Arguments:

  Dest - Memory address to write
  Data - Data to write

Returns: 

  None.

--*/
{
  mSmst->SmmIo.Mem.Write (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT16, 
                  Dest, 
                  1, 
                  &Data
                  );

  return ;
}

UINT16
SmmMemRead16 (
  IN  UINT64    Dest
  )
/*++

Routine Description:

  Do a two bytes Memory read

Arguments:

  Dest - Memory address to read

Returns: 

  Data read

--*/
{
  UINT16 Data;
  mSmst->SmmIo.Mem.Read (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT16, 
                  Dest, 
                  1, 
                  &Data
                  );

  return Data;
}

VOID
SmmMemWrite32 (
  IN  UINT64    Dest,
  IN  UINT32    Data
  )
/*++

Routine Description:

  Do a four bytes Memory write

Arguments:

  Dest - Memory address to write
  Data - Data to write

Returns: 

  None.

--*/
{
  mSmst->SmmIo.Mem.Write (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT32, 
                  Dest, 
                  1, 
                  &Data
                  );

  return ;
}

UINT32
SmmMemRead32 (
  IN  UINT64    Dest
  )
/*++

Routine Description:

  Do a four bytes Memory read

Arguments:

  Dest - Memory address to read

Returns: 

  Data read

--*/
{
  UINT32 Data;
  mSmst->SmmIo.Mem.Read (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT32, 
                  Dest, 
                  1, 
                  &Data
                  );

  return Data;
}

VOID
SmmMemAnd32 (
  IN  UINT64    Dest,
  IN  UINT32    Data
  )
/*++

Routine Description:

  Do a four bytes Memory read, then AND with Data, then write back to the same address 

Arguments:

  Dest - Memory address to write
  Data - Data to do AND

Returns: 

  None.

--*/
{
  UINT32 Data_And;
  mSmst->SmmIo.Mem.Read (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT32, 
                  Dest, 
                  1, 
                  &Data_And
                  );
  Data_And&=Data;
  mSmst->SmmIo.Mem.Write (
                  &mSmst->SmmIo, 
                  SMM_IO_UINT32, 
                  Dest, 
                  1, 
                  &Data_And
                  );

  return ;
}
