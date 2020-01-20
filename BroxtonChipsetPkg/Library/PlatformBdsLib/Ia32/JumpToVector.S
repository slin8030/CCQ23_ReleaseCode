## @file
#  This is the ASM for Jump to a specific address
# @copyright
#  INTEL CONFIDENTIAL
#  Copyright 1999 - 2016 Intel Corporation.
#
#  The source code contained or described herein and all documents related to the
#  source code ("Material") are owned by Intel Corporation or its suppliers or
#  licensors. Title to the Material remains with Intel Corporation or its suppliers
#  and licensors. The Material may contain trade secrets and proprietary and
#  confidential information of Intel Corporation and its suppliers and licensors,
#  and is protected by worldwide copyright and trade secret laws and treaty
#  provisions. No part of the Material may be used, copied, reproduced, modified,
#  published, uploaded, posted, transmitted, distributed, or disclosed in any way
#  without Intel's prior express written permission.
#
#  No license under any patent, copyright, trade secret or other intellectual
#  property right is granted to or conferred upon you by disclosure or delivery
#  of the Materials, either expressly, by implication, inducement, estoppel or
#  otherwise. Any license under such intellectual property rights must be
#  express and approved by Intel in writing.
#
#  Unless otherwise agreed by Intel in writing, you may not remove or alter
#  this notice or any other notice embedded in Materials by Intel or
#  Intel's suppliers or licensors in any way.
#
#  This file contains a 'Sample Driver' and is licensed as such under the terms
#  of your license agreement with Intel or your vendor. This file may be modified
#  by the user, subject to the additional terms of the license agreement.
#
# @par Specification
##
#
# Notes:
# - Masm uses "This", "ebx", etc as a directive.
# - H2INC is still not embedded in our build process so I translated the struc manually.
# - Unreferenced variables/arguments (This, NewBsp, NewStack) were causing compile errors and
#       did not know of "pragma" mechanism in MASM and I did not want to reduce the warning level.
#       Instead, I did a dummy referenced.
#

#
# Routine Description:
#       This allows the caller to switch the stack and goes to the new entry point
#
# Arguments:
#       EntryPoint      - Pointer to the location to enter
#       Parameter       - Parameter to pass in
#
# Returns:
#
#       Nothing. Goes to the Entry Point passing in the new parameters
#
ASM_GLOBAL ASM_PFX(JumpToVector)
ASM_PFX(JumpToVector):

  push  %ebx

  mov   12(%esp), %ebx                  # ebx = Parameter
  mov   8(%esp), %ecx                   # ecx = EntryPoint
  push  %ebx
  push  $0
  jmp   %ecx

  pop   %ebx
  ret


