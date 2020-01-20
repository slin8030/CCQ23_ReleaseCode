## @file
#  This is the ASM for starting a linux kernel
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

ASM_GLOBAL ASM_PFX(AsmStartLinuxKernel)
ASM_PFX(AsmStartLinuxKernel):
  movl     4(%esp), %eax
  subl     $0x10, %esp
  leal     NewSelectorJmp, %ebx
  movl     %ebx, (%esp)
  movl     $0x10, %ebx
  movl     %ebx, 4(%esp)
  jmpl     (%esp)

NewSelectorJmp:
  addl     $0x10, %esp

  movl     $0x18, %ebx
  movl     %ebx, %ds
  movl     %ebx, %es
  movl     %ebx, %fs
  movl     %ebx, %gs
  movl     %ebx, %ss

  movl     $0x8000, %esi
  xorl     %ebp, %ebp
  xorl     %edi, %edi
  xorl     %ebx, %ebx
  jmpl     %eax
  ret
