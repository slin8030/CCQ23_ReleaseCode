/*---------------------------------------------------------------------------
 *
 *	$Workfile:   T66WMI.ASL  $
 *
 *	$Header:   $
 *
 *	Copyright (c) 2016~ Compal Company
 *
 *	This program contains proprietary and confidential information.
 *	All rights reserved except as may be permitted by prior
 *	written consent.
 *
 *	Content:
 *
 *		R E V I S I O N    H I S T O R Y
 *	$Log:   $
 *
 *  Date        Who             Change
 *	----------  ------------    -------------------------------------------
 *-------------------------------------------------------------------------*/
#define WMI_MAX_BIT       0x80000000

#define WMIERR(a)                 Or(WMI_MAX_BIT , a)

#define WMI_ERROR(a)              LNotEqual (And(a, WMI_MAX_BIT),0)

#define WMI_SUCCESS               0
#define WMI_LOAD_ERROR            WMIERR (1)
#define WMI_INVALID_PARAMETER     WMIERR (2)
#define WMI_UNSUPPORTED           WMIERR (3)
#define WMI_BAD_BUFFER_SIZE       WMIERR (4)
#define WMI_BUFFER_TOO_SMALL      WMIERR (5)
#define WMI_NOT_READY             WMIERR (6)
#define WMI_DEVICE_ERROR          WMIERR (7)
#define WMI_WRITE_PROTECTED       WMIERR (8)
#define WMI_OUT_OF_RESOURCES      WMIERR (9)
#define WMI_VOLUME_CORRUPTED      WMIERR (10)
#define WMI_VOLUME_FULL           WMIERR (11)
#define WMI_NO_MEDIA              WMIERR (12)
#define WMI_MEDIA_CHANGED         WMIERR (13)
#define WMI_NOT_FOUND             WMIERR (14)
#define WMI_ACCESS_DENIED         WMIERR (15)
#define WMI_NO_RESPONSE           WMIERR (16)
#define WMI_NO_MAPPING            WMIERR (17)
#define WMI_TIMEOUT               WMIERR (18)
#define WMI_NOT_STARTED           WMIERR (19)
#define WMI_ALREADY_STARTED       WMIERR (20)
#define WMI_ABORTED               WMIERR (21)
#define WMI_ICMP_ERROR            WMIERR (22)
#define WMI_TFTP_ERROR            WMIERR (23)
#define WMI_PROTOCOL_ERROR        WMIERR (24)
#define WMI_INCOMPATIBLE_VERSION  WMIERR (25)
#define WMI_SECURITY_VIOLATION    WMIERR (26)
#define WMI_CRC_ERROR             WMIERR (27)
#define WMI_END_OF_MEDIA          WMIERR (28)
#define WMI_END_OF_FILE           WMIERR (31)
#define WMI_INVALID_LANGUAGE      WMIERR (32)


//Comvert Status value form CMFC version to WMI version.
Method(CTWE, 1) {
  If (LEqual(Arg0,0x0)){  
    return (WMI_LOAD_ERROR)
  }ElseIf (LEqual(Arg0,0x1)){
    return (WMI_SUCCESS)
  }ElseIf (LEqual(Arg0,0x8000)){
    return (WMI_UNSUPPORTED)
  }Else{
    return (WMI_NOT_READY)
  }
}

