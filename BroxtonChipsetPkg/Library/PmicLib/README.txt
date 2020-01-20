[Coding style]
1.  Use 2 spaces as indent.  No TAB should be used.

2.  Only APIs should be published cross modules.
 a. DO NOT refer PMIC register definition outside of this module. DO NOT copy register definition to other modules.

3.  Public API starts with "Pmic" while private API starts with "XXXPmic". And public API should be exposed in PmicLib.h.
    For example, CcPmicIsPwrBtnPressed is private API used for module internal use only.  PmicVbusControl is the public API although it is for Crystal Cove only.

4.  Define every bitmask as macro. Actually macro is encouraged to replace instant number.
    If there is reason that could not use macro, at least mention detail of what the value stands for in comment.

5.  Do not use platform name in function naming. If there is multi profiles for each platform, use generic name and invokes them in PlatformPmic.c (PMIC platform driver).


[File list]
Vlv2CRDeviceRefCodePkg\ValleyView2Soc\SouthCluster\Library\PmicLib
01/21/2014  03:53 PM             4,996 PmicLib.c                               #public API
01/15/2014  10:29 AM             1,719 PmicLib.inf                             #component inf
01/22/2014  09:16 AM            16,850 PmicReg_CrystalCove.h                   #ROHM & dialog register definition
01/21/2014  03:51 PM            29,355 Pmic_CrystalCove.c                      #ROHM&DIALOG routine
12/27/2013  02:22 PM               997 Pmic_CrystalCove.h
01/15/2014  09:38 AM             6,161 PmicReg_DollarCove_TI.h                 #TI register definition
01/17/2014  04:31 PM            10,230 PmicReg_DollarCove_XPOWERS.h            #XPOWERS register definition
01/22/2014  09:17 AM             7,976 PmicReg_PennyCove_XPOWERS.h             #obsoleted
01/15/2014  11:27 AM             8,590 Pmic_DollarCove_TI.c                    #TI PMIC support routine
01/22/2014  02:09 PM             1,442 Pmic_DollarCove_TI.h
01/21/2014  04:05 PM            11,163 Pmic_DollarCove_XPOWERS.c               #XPWR PMIC support routine
01/17/2014  02:55 PM             1,088 Pmic_DollarCove_XPOWERS.h
01/22/2014  04:01 PM             4,100 Pmic_i2c_mutex.c
01/22/2014  09:16 AM            12,473 Pmic_PennyCove_XPOWERS.c
01/15/2014  11:48 AM             6,684 Pmic_Private.h
01/08/2014  01:17 PM               373 README.txt

[Code tips]
>>To enable specific PMIC support:
Uncomment corresponding line for specific PMIC.? Code is in PmicLib.c
PmicLib.c
struct PMIC_object *g_pmic_support_list[] = {
?&cc_obj,? //Crystal Cove (Rohm & Dialog)
? //&ti_dc_obj,? //DollarCove TI PMIC
? &dc_xpower_obj, //DollarCove XPOWER PMIC
? //&pc_xpwr_obj, //PennyCove XPOWERS PMIC
};
?

>>PMIC device type probe:
PMIC type is probed in PmicLibConstructor. It is invoked every time Pmiclib
is loaded. It will detect PMIC type and load correct PMIC object (software
concept). PMIC object implements PMIC functions.

>>DVFS init:
DVFS initialization code is coved in TiPmicDVFSInit and DcXPmicDVFSInit. It
will be invoked when doing PmicMiscInit.


>>SVID or I2c access for PUNIT:
For DollarCove, bits[27:26] of REG_SB_BIOS_CONFIG(0x06)??? must set to b'01
to tell PUNIT to use i2c instead of SVID. It is covered in TiPmicDVFSInit and
DcXPmicDVFSInit.

MsgBus32AndThenOr(VLV_PUNIT, 0x06, data32, 0xF3FFFFFF, 0x04000000); //
REG_SB_BIOS_CONFIG bit[27:26]=b'01 init PUNIT to use i2c to control power rail

>>I2C5 RTD3:
I2C5 is for PMIC. And RTD3 is disabled as required.? _PS0 and _PS3 are
already commented off.

Code location: Vlv2CRDeviceRefCodePkg\AcpiTables\PchLpss.asl?? Line1859?
Device(I2C5)


>>ACPI semaphore:
A private name is defined for PO. We will switch to _DSM method after PO.
Code location: Vlv2CRDeviceRefCodePkg\AcpiTables\PchLpss.asl?? Line1827?
Device(I2C5)

Device(I2C5)
{
 Name (_SEM, 0x01)???? // BYT-CR only,1 enable or 0(default) disable Punit
semaphore on I2C5


>>ACPI entries:
Not done in current code since HID and name is still open from Architect side
. Android OS driver will bypass HID check to continue loading during PO.

>>Most Chip specific initialization :
It includes LDO, Power button threshold, BUCK init sequences.
They are done in g_xpwr_MiscRegInit structure.


>>G3 bit:
BIOS to leave the G3 bit at default set so that PMC does not read the Native
Pin PWRBTN_B

Vlv2CRTbltDevicePkg\PlatformSmm\Platform.c  InitializePlatformSmm
SouthClusterConfig.vfi StateAfterG3 is disabled by default
