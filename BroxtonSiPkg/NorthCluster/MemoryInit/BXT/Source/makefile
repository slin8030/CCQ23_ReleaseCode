#**************************************************************************
#*                                                                        *
#*      Intel Restricted Secret                                           *
#*                                                                        *
#*      Cherryview Memory Reference Code                                  *
#*                                                                        *
#*      Copyright (c) 2013 Intel Corp.                                    *
#*                                                                        *
#*      This program has been developed by Intel Corporation.             *
#*      Licensee has Intel's permission to incorporate this source code   *
#*      into their product, royalty free.  This source code may NOT be    *
#*      redistributed to anyone without Intel's written permission.       *
#*                                                                        *
#*      Intel specifically disclaims all warranties, express or           *
#*      implied, and all liability, including consequential and other     *
#*      indirect damages, for the use of this code, including liability   *
#*      for infringement of any proprietary rights, and including the     *
#*      warranties of merchantability and fitness for a particular        *
#*      purpose.  Intel does not assume any responsibility for any        *
#*      errors which may appear in this code nor any responsibility to    *
#*      update it.                                                        *
#*                                                                        *
#**************************************************************************
#**************************************************************************
#*                                                                        *
#*  PURPOSE:                                                              *
#*                                                                        *
#*      Makefile for Denverton SOC                                        *
#*      Memory Reference Code.                                            *
#*                                                                        *
#**************************************************************************

#--------------------------------------------------------------------
# A dummy pseudo-target just so it is the first in the file.
# Now, NMAKE is the same as NMAKE ALL. This is just for developer
# convenience, such that all source and object files appear at the
# top of this file for easy modification.
#--------------------------------------------------------------------
DUMMY: ALL

#--------------------------------------------------------------------
# Build-time platform options
# Switches:  1 = Include feature
#            0 = Exclude feature
#--------------------------------------------------------------------


OPTIMIZE_FOR_SIZE     = 1
BXT_A0=0
BXT_PO=0
PROJ=BXTP

#--------------------------------------------------------------------
# Some basic definitions.
#--------------------------------------------------------------------
ROOTPATH                     = ..\..
MEMREFCODEBINARYNAME         = MEMINIT

RC_ROM_BASEADDRESS = 0xfffc0000

#--------------------------------------------------------------------
# Directory Declarations
#--------------------------------------------------------------------
SOURCEDIR=$(ROOTPATH)\BXT\Source
OUT32DIR=$(ROOTPATH)\OUT32
TOOLSDIR=$(ROOTPATH)\Restricted\Tools

INCLUDEDIR=/I $(ROOTPATH)\BXT\Include /I $(ROOTPATH)\Mmrc\ProjectIndependent\Include /I $(ROOTPATH)\Mmrc\ProjectData\$(PROJ)\Include /I $(ROOTPATH)\Mmrc\ProjectData\$(PROJ)\Restricted\MINIBIOS\Include
#--------------------------------------------------------------------
# Compiler and Linker Flags
#--------------------------------------------------------------------

CL              = cl
ML              = ml
MLFLAGS         = /c /Zd /Fm$(OUT32DIR)\\ /Fl$(OUT32DIR)\\ /Fo$(OUT32DIR)\\
LINK32          = link

LINK32FLAGS     = /nologo \
                  /incremental:no \
                  /debug \
                  /align:32 \
                  /nodefaultlib \
                  /LTCG \
                  /subsystem:console \
                  /merge:.rdata=.data \
                  /pdb:$(OUT32DIR)\$(MEMREFCODEBINARYNAME).pdb \
                  /map:$(OUT32DIR)\$(MEMREFCODEBINARYNAME).map \
                  /out:$(OUT32DIR)\$(MEMREFCODEBINARYNAME).dll \
                  /DLL

CLFLAGS         = /Oi- /Zp1 /Gr /GL- /w /WX /Gy /FAsc /c /Zi /Gm /X /Fd /GS- /Gs98304 $(INCLUDEDIR) 
CLFLAGS          = $(CLFLAGS) /DMINIBIOS=1  /DBXT_PPO_SUPPORT=1 /DBXT_PLATFORM /DBXT_MRC_SUPPORT=1 /DSMM_SUPPORT=1 /Fo$(OUT32DIR)\\ /Fa$(OUT32DIR)\\

H2INC           = $(MASMPATH)\h2inc
H2INCFLAGS      = /Zu /Zp1 /D_MSC_EXTENSIONS /DMINIBIOS=1 /DASM_INC /X $(INCLUDEDIR)
H2INCFLAGS	= $(H2INCFLAGS) /DBXT_PPO_SUPPORT=1 /DBXT_PLATFORM /DBXT_MRC_SUPPORT=1 /DSMM_SUPPORT=1


#Optimize for small size on P4
!IF "$(OPTIMIZE_FOR_SIZE)" == "1"
CLFLAGS          = $(CLFLAGS) /O1 /Osyb2
!ELSE
#Disable optimization for debugging
CLFLAGS          = $(CLFLAGS) /Od
!ENDIF

#--------------------------------------------------------------------
# Memory Reference Code Object Files.
#--------------------------------------------------------------------
LINK32OBJS      =  $(OUT32DIR)\MrcEntrypoints.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\OemHooks.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MrcFunc.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\McFunc.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\Mailbox.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\DetectDimms.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\ConfigMemData.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\ConfigMem.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MmrcProjectDataGenerated.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MmrcProjectData.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\cpgcapi.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\StringsGenerated.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MmrcLibraries.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\Uart.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\RegAccess.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\Printf.obj
LINK32OBJS      =  $(LINK32OBJS) $(OUT32DIR)\MmrcHooks.obj



#//UB_DIMM_BEGIN
!IF "$(UB_DIMM)" == "1"
!MESSAGE --Support UB_DIMM
CLFLAGS = $(CLFLAGS) /DUB_DIMM=1
H2INCFLAGS = $(H2INCFLAGS) /DUB_DIMM=1
!ELSE
!MESSAGE --Disable UB_DIMM
CLFLAGS = $(CLFLAGS) /DUB_DIMM=0
H2INCFLAGS = $(H2INCFLAGS) /DUB_DIMM=0
!ENDIF
#//UB_DIMM_END

#//SO_DIMM_BEGIN
!IF "$(SO_DIMM)" == "1"
!MESSAGE --Support SO_DIMM
CLFLAGS = $(CLFLAGS) /DSO_DIMM=1
H2INCFLAGS = $(H2INCFLAGS) /DSO_DIMM=1
!ELSE
!MESSAGE --Disable SO_DIMM
CLFLAGS = $(CLFLAGS) /DSO_DIMM=0
H2INCFLAGS = $(H2INCFLAGS) /DSO_DIMM=0
!ENDIF
#//SO_DIMM_END

!IF "$(SDV)" == "1"
!MESSAGE --Support Soft SDV
CLFLAGS = $(CLFLAGS) /DSDV_BIOS=1
H2INCFLAGS = $(H2INCFLAGS) /DSDV_BIOS=1
!ELSE
!MESSAGE --Disable Soft SDV
CLFLAGS = $(CLFLAGS) /DSDV_BIOS=0
H2INCFLAGS = $(H2INCFLAGS) /DSDV_BIOS=0
!ENDIF

CLFLAGS = $(CLFLAGS) /DMINIBIOS=1 
CLFLAGS = $(CLFLAGS) /DDEBUG_MSG=0


#--------------------------------------------------------------------
# Inference Rules
#--------------------------------------------------------------------

{$(SOURCEDIR)}.c{$(OUT32DIR)}.obj:
        $(CL) $(CLFLAGS) $<

{$(ROOTPATH)\Mmrc\ProjectIndependent\Source}.c{$(OUT32DIR)}.obj:
        $(CL) $(CLFLAGS) $<

{$(ROOTPATH)\Mmrc\ProjectData\$(PROJ)\Source}.c{$(OUT32DIR)}.obj:
        $(CL) $(CLFLAGS) $<

{$(ROOTPATH)\Mmrc\ProjectData\$(PROJ)\Restricted\MiniBIOS\Source}.c{$(OUT32DIR)}.obj:
        $(CL) $(CLFLAGS) $<

#--------------------------------------------------------------------

ALL:    CLEAN TARGETDIRECTORIES COPYSRC $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN MAKEASMLINKS
TARGETDIRECTORIES:
  @echo MKDIR Target Directories .\Out32
  if not exist $(OUT32DIR) mkdir $(OUT32DIR)


$(OUT32DIR)\MrcEntrypoints.obj: $(SOURCEDIR)\MrcEntrypoints.asm  
  $(ML) $(MLFLAGS) $(SOURCEDIR)\MrcEntrypoints.asm

$(OUT32DIR)\MrcEntrypoints.i: $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN


MAKEASMLINKS: $(OUT32DIR)\MrcEntrypoints.i

$(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN: $(LINK32OBJS)
        @echo Linking 32-bit Components for POST Memory Init Module...
        $(LINK32) /entry:MrcEntryPoint_FAR $(LINK32FLAGS) $(LINK32OBJS)
        @copy /b $*.dll $*.efi
        $(TOOLSDIR)\placepe $*.dll $*.efi $(RC_ROM_BASEADDRESS) NOTIMESTAMP > $*_placepe.log
        @copy /b $*.efi $*.bin
        $(TOOLSDIR)\FindEntryPoint $*.bin /A > $(OUT32DIR)\mrcentrypoints.i $(@B)
        copy /b $*.BIN $(OUT32DIR)\mrc_192k.bin
        $(TOOLSDIR)\fwpad $(OUT32DIR)\mrc_192k.bin 196608


$(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT: $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN
        @echo Generating dump file $(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT...
        dumpbin $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN /headers > $(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT
        dumpbin $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN /disasm >> $(OUT32DIR)\$(MEMREFCODEBINARYNAME).DUMPBIN.TXT
        @echo --------------------------------------------------------------
        @echo *** Memory Reference Code Build Complete ***
        @echo Mem Ref Code Image File: $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN
        @echo --------------------------------------------------------------
        -@echo Removing .pdb and .idb files in the $(SOURCEDIR) directory...
        -@if exist vc60.pdb  erase vc60.pdb
        -@if exist vc60.idb  erase vc60.idb
        -@if exist vc70.pdb  erase vc70.pdb
        -@if exist vc70.idb  erase vc70.idb
        -@if exist $(SOURCEDIR)\vc60.pdb  erase $(SOURCEDIR)\vc60.pdb
        -@if exist $(SOURCEDIR)\vc60.idb  erase $(SOURCEDIR)\vc60.idb
        -@if exist $(SOURCEDIR)\vc70.pdb  erase $(SOURCEDIR)\vc70.pdb
        -@if exist $(SOURCEDIR)\vc70.idb  erase $(SOURCEDIR)\vc70.idb

COPYSRC: $(OUT32DIR)\$(MEMREFCODEBINARYNAME).BIN
  @echo Copying $(SOURCEDIR) to $(OUT32DIR)
  copy $(SOURCEDIR) $(OUT32DIR) > nul

CLEAN:
  -@echo  Removing directory $(OUT32DIR)
  -@if exist $(OUT32DIR) rd $(OUT32DIR) /S /Q
