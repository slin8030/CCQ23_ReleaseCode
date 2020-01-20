/** @file
    This is an implementation for .

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef __BPDT_HEADER_H__
#define __BPDT_HEADER_H__

//Please do not put #includes here, they should be with the implementation code

extern EFI_GUID gEfiBpdtLibBp1DataGuid;
extern EFI_GUID gEfiBpdtLibBp2DataGuid;

#define BPDT_SIGN_GREEN       0x000055AA    //Normal Boot
#define BPDT_SIGN_YELLOW      0x00AA55AA    //Recovery Boot
//#define BPDT_SIGN_RED       any other value, ie !(GREEN || YELLOW)


typedef enum {
  BootPart1,
  BootPart2,
  BootPartMax
} BOOT_PARITION_SELECT;

typedef struct {
  VOID   *DataPtr;
  UINT32  Size;
} BPDT_PAYLOAD_DATA;

typedef enum {
  BpdtOemSmip,      // 0
  BpdtCseRbe,       // 1
  BpdtCseBup,       // 2
  BpdtUcode,        // 3
  BpdtIbb,          // 4
  BpdtSbpdt,        // 5 - Secondary BPDT within a BPDT Entry
  BpdtObb,          // 6
  BpdtCseMain,      // 7
  BpdtIsh,          // 8
  BpdtCseIdlm,      // 9
  BpdtIfpOverride,  //10
  BpdtDebugTokens,  //11
  BpdtUfsPhyConfig, //12
  BpdtUfsGppLunId,  //13
  BpdtPmc,          //14
  BpdtIunit,        //15
  BpdtMaxType
} BPDT_ENTRY_TYPES;

typedef struct {
  UINT32  Signature;
  UINT16  DscCount;
  UINT16  BpdtVer;
  UINT32  RedundantBlockXorSum;
  UINT32  IfwiVer;
  UINT64  FitToolVer;
} BPDT_HEADER;


typedef struct {
  UINT16  Type;
  UINT16  Flags;
  UINT32  LbpOffset;  // Offset of Sub-Partition starting from base of LBP
  UINT32  Size;       // Size of the Sub-Partition
} BPDT_ENTRY;


typedef struct {
  UINT32  HeaderMarker;
  UINT32  NumOfEntries;
  UINT8   HeaderVersion;
  UINT8   EntryVersion;
  UINT8   HeaderLength;
  UINT8   Checksum;
  UINT32  SubPartName;
} SUBPART_DIR_HEADER;


typedef struct {
  CHAR8   EntryName[12];
  UINT32  EntryOffset:25;
  UINT32  Huffman:1;
  UINT32  Reserved1:6;
  UINT32  EntrySize;
  UINT32  Reserved2;
} SUBPART_DIR_ENTRY;

typedef enum {
  HashIbbl = 0,
  HashIbbm,
  HashObb
} BPM_HASH_SELECT;

///
/// Boot Policy Manifest (BPM) data structure
///
typedef struct {
  UINT32                    ExtType;       ///< 0x00000013  (Little-Endian)
  UINT32                    ExtLength;
  UINT32                    NemData;
  UINT32                    IbblHashAlgo;  ///< 0x00000002
  UINT32                    IbblHashSize;  ///< 0x00000020
  UINT8                     IbblHash[0x20];
  UINT32                    IbbmHashAlgo;
  UINT32                    IbbmHashSize;
  UINT8                     IbbmHash[0x20];
  UINT32                    ObbHashAlgo;
  UINT32                    ObbHashSize;
  UINT8                     ObbHash[0x20];
  UINT8                     Reserved[124]; ///< Others data
} BPM_DATA_FILE;


/**
  Locate and return a pointer to the selected Logical Boot Partition.

  For block devices, the address will be the location in memory that the BP was copied to.
  For memory mapped device (eg SPI), the address will be the location on the device.

  @param    LbpSel         Which BP to locate on the boot device
  @param    Address        The address of the selected BP on the boot device

  @retval   EFI_SUCCESS    The operation completed successfully.
  @retval   other          The selected BP could not be found.
**/
EFI_STATUS
EFIAPI
GetBootPartitionPointer (
  IN  BOOT_PARITION_SELECT  LbpSel,
  OUT VOID                  **LbpPointer
  );

//[-start-160701-IB06740505-add]//
EFI_STATUS
EFIAPI
GetSpiBootPartitionLinearAddress (
  IN  BOOT_PARITION_SELECT  LbpSel,
  OUT UINT32                *BpFlashLinearAddress
  );
//[-end-160701-IB06740505-add]//

/**
  Parse the Boot Partition Descriptor Table of the provided Logical Boot Partition.
  Note: "payload" is the Sub-Partition pointed to by the BDPT Entry of the given type.

  @param   LbpPointer      Pointer to the start of the BootPartition to be parsed
  @param   EntryType       The entry type of the subparition(payload) to look for
  @param   BpdtPayloadPtr  Address of the Struct to put the results into (MUST be allocated by caller)

  @retval  EFI_SUCCESS     The operation completed successfully.
**/
EFI_STATUS
EFIAPI
GetBpdtPayloadDataRaw (
  IN  VOID                *LbpPointer,
  IN  BPDT_ENTRY_TYPES    EntryType,
  OUT BPDT_PAYLOAD_DATA   *BpdtPayloadPtr
  );

/**
  Parse the Boot Partition Descriptor Table on the selected Logical Boot Partition.
  Takes the BP as input since some BPDT entry types can exist in both locations.
  Note: "payload" is the Sub-Partition pointed to by the BDPT Entry of the given type.

  @param   EntryType       The entry type of the subparition(payload) to look for
  @param   BpdtPayloadPtr  Pointer to a Struct (in a HOB) containing the Size and Absolute address
                           in memory(or spi) of the BPDT payload(subpart) of EntryType.
                           If the caller shadows the Payload for performance, it should update this value

  @retval  EFI_SUCCESS     The operation completed successfully.
**/
EFI_STATUS
EFIAPI
GetBpdtPayloadData (
  IN  BOOT_PARITION_SELECT  BpSel,
  IN  BPDT_ENTRY_TYPES      EntryType,
  OUT BPDT_PAYLOAD_DATA     **BpdtPayloadPtr
  );


/**
  Search the Subpartition pointed to by BpdtPayloadPtr for a Directory entry with "EntryName",
  then fill in the Struct pointed to by SubPartPayloadPtr with the Addres and Size of the Dir Entry.
  Note: "payload" is the Data pointed to by the Directory Entry with the given name.

  @param   BpdtPayloadPtr     Location and Size of the Payload to search
  @param   EntryName          the String name of the Directory entry to look for
  @param   SubPartPayloadPtr  Address of the Struct to put the results into (MUST be allocated by caller)

  @retval  EFI_SUCCESS        The operation completed successfully.
**/
EFI_STATUS
EFIAPI
GetSubPartitionPayloadData (
  IN  BPDT_PAYLOAD_DATA     *BpdtPayloadPtr,
  IN  CONST CHAR8           *EntryName,
  OUT BPDT_PAYLOAD_DATA     *SubPartPayloadPtr
  );


/**
  This is a wrapper function for using VerifyHashBpm that will only
  check the hash for the payloads used during normal boot.

  If you want to check other payloads (ie during recovery flow), then the
  calling code should locate and pass in the payload directly to VerifyHashBpm().

  @param  PayloadSel   Which hash in BPM to compare with.

  @retval EFI_SUCCESS  The function completed successfully and the Hash matched.
  @retval other        An error occured when locating and computing the Hash.
**/
EFI_STATUS
EFIAPI
LocateAndVerifyHashBpm (
  IN BPM_HASH_SELECT   PayloadSel
  );


/**
  Computes the Hash of a given data block and compares to the one in the Boot Policy Metadata.

  @param  PayloadSel   Which hash in BPM to compare with.
  @param  PayloadPtr   Pointer to the begining of the data to be hashed.
  @param  DataSize     Size of the data to be hashed.
  @param  BpmPtr       Pointer to the BPM structure.

  @retval EFI_SUCCESS             If the function completed successfully.
  @retval EFI_ABORTED             If the attempt to compute the hash fails.
  @retval EFI_INVALID_PARAMETER   If the hash type or size in BPM is unsupported.
  @retval EFI_SECURITY_VIOLATION  If the has does not match the one in BPM.
**/
EFI_STATUS
EFIAPI
VerifyHashBpm (
  IN BPM_HASH_SELECT   PayloadSel,
  IN BPM_DATA_FILE     *BpmPtr,
  IN VOID              *PayloadPtr,
  IN UINT32            DataSize
  );

#endif
