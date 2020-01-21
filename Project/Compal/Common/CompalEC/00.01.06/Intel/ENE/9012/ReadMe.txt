ReadMe for CompalEcLib function
---------------------------------------------------
Add global chipset name definition for CompalEcLib:
--------------------------------------------------- 
 1.>Project\Compal\A30\ZAL20\Project.env
...
  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = Apl  -> for Apollo Lake

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = Skyl  -> for SkyLake 

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = Brasw  -> for Braswell 

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = Bdw  -> for Broadwell

OR

 2.>Project\Compal\Common\Package.dsc
...
[Defines]
  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = Apl  -> for Apollo Lake

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = Skyl  -> for SkyLake 

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = Brasw  -> for Braswell 

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = Bdw  -> for Broadwell

---------------------------------------------------
Add Pcd for CompalECLib used:
---------------------------------------------------
1.>Project\Compal\Common\Compalcommon.dec
[PcdsDynamic]
  gCompalCommonCodeGuid.PcdCrisisMode|0xFF|UINT8|0x00000021
