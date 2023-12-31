/** @file

@copyright
 Copyright (c) 2016 Intel Corporation. All rights reserved
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
    "T66WMI.aml",
    "SSDT",
    1,
    "COMPAL",
    "T66WMITB",
    0x1000
    )
{
// include ("OemASL.asl")
  include ("WMI/T66WMI.asl")
}
