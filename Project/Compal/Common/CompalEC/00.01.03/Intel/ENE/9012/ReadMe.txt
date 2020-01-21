ReadMe for CompalEcLib function
-----------------------------------
Add global chipset name definition for CompalEcLib:
 
 1.>Project\Compal\A30\ZAL20\Project.env
...
  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = SKYLAKE  -> for SkyLake 

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = BRASWELL  -> for Braswell 

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = BROADWELL  -> for Broadwell

OR

 2.>Project\Compal\Common\Package.dsc
...
[Defines]
  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = SKYLAKE  -> for SkyLake 

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = BRASWELL  -> for Braswell 

  EDK_GLOBAL  COMPAL_CHIPSET_NAME_FOR_ECLIB = BROADWELL  -> for Broadwell


