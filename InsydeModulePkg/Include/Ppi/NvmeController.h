/** @file
  NVM Express Host Controller PEI Header

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _PEI_NVME_CONTROLLER_PPI_H_
#define _PEI_NVME_CONTROLLER_PPI_H_

#define PEI_NVME_CONTROLLER_PPI_GUID \
  { \
    0xf4747c79, 0x3809, 0x46f1, 0xaa, 0xa3, 0xb3, 0xdc, 0xdf, 0x6a, 0x98, 0x92 \
  }

///
/// Forward declaration for the PEI_NVME_CONTROLLER_PPI
///
typedef struct _PEI_NVME_CONTROLLER_PPI  PEI_NVME_CONTROLLER_PPI;

typedef
EFI_STATUS
(EFIAPI *PEI_GET_NVME_CONTROLLER) (
  IN     EFI_PEI_SERVICES               **PeiServices,
  IN     PEI_NVME_CONTROLLER_PPI        *This,
  IN     UINT8                          NvmeControllerId,
  OUT    EFI_PHYSICAL_ADDRESS           *NvmeMemBaseAddr
  );

struct _PEI_NVME_CONTROLLER_PPI {
  PEI_GET_NVME_CONTROLLER               GetNvmeController;
};

extern EFI_GUID gPeiNvmeControllerPpiGuid;

#endif
