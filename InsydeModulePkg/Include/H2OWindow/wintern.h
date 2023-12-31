/* wintern.h*/
/*
 * Copyright (c) 1999, 2005 Greg Haerr <greg@censoft.com>
 *
 * Microwindows internal routines header file
 */
#ifndef _WINTERN_H_
#define _WINTERN_H_

#include <string.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/AbsolutePointer.h>

#define DBLCLICKSPEED	750		/* mouse dblclik speed msecs (was 450)*/

/* gotPaintMsg values*/
#define PAINT_PAINTED		0	/* WM_PAINT msg has been processed*/
#define PAINT_NEEDSPAINT	1	/* WM_PAINT seen, paint when can*/
#define PAINT_DELAYPAINT	2	/* WM_PAINT seen,paint after user move*/

/* non-win32 api access for microwin*/
BOOL		MwSetDesktopWallpaper(PMWIMAGEHDR pImage);
void		MwRegisterFdInput(HWND hwnd,int fd);
void		MwUnregisterFdInput(HWND hwnd,int fd);
void		MwRegisterFdOutput(HWND hwnd,int fd);
void		MwUnregisterFdOutput(HWND hwnd,int fd);
void		MwRegisterFdExcept(HWND hwnd,int fd);
void		MwUnregisterFdExcept(HWND hwnd,int fd);

/* internal routines*/
EFI_STATUS GdAddGopDevice (EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop);
EFI_STATUS GdRemoveGopDevice (EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop);
EFI_STATUS GdAddEfiKeyData (EFI_KEY_DATA *KeyData);

EFI_STATUS
GdAddPointerData (
  EFI_SIMPLE_POINTER_STATE      *SimplePointerState    OPTIONAL,
  EFI_ABSOLUTE_POINTER_STATE    *AbsolutePointerState  OPTIONAL
  );



/* winuser.c*/
PWNDCLASS	MwFindClassByName(LPCWSTR lpClassName);
void		MwDestroyWindow(HWND hwnd,BOOL bSendMsg);
HWND		MwGetTopWindow(HWND hwnd);
void		MwCalcClientRect(HWND hwnd);
void		MwSendSizeMove(HWND hwnd, BOOL bSize, BOOL bMove);
void		MwSetCursor(HWND wp, PMWCURSOR pcursor);

/* wingdi.c*/
#define MwIsClientDC(hdc)	(((hdc)->flags & DCX_WINDOW) == 0)
#define MwIsMemDC(hdc)		(((hdc)->psd->flags & PSF_MEMORY) == PSF_MEMORY)
void		MwPaintNCArea(HWND hwnd);
HWND		MwPrepareDC(HDC hdc);
void		MwSetClipWindow(HDC hdc);
void		MwSetTextCoding(long mode);

/* winsbar.c*/
void		MwAdjustNCScrollbars(HWND hwnd);
void		MwPaintNCScrollbars(HWND hwnd, HDC hdc);
void		MwHandleNCMessageScrollbar(HWND hwnd, UINT msg, WPARAM hitcode,
			LPARAM lParam);

/* winexpos.c*/
void		MwRedrawScreen(void);
void		MwHideWindow(HWND hwnd,BOOL bChangeFocus,BOOL bSendMsg);
void		MwShowWindow(HWND hwnd,BOOL bSendMsg);
void		MwRaiseWindow(HWND hwnd);
void		MwLowerWindow(HWND hwnd);
BOOL		MwCheckOverlap(HWND topwp, HWND botwp);
void		MwClearWindow(HWND wp,MWCOORD x,MWCOORD y,MWCOORD width,
			MWCOORD height,BOOL exposeflag);
void		MwExposeArea(HWND wp, MWCOORD rootx,MWCOORD rooty,
			MWCOORD width,MWCOORD height);
/* winevent.c*/
BOOL		MwCheckMouseEvent(void);
BOOL		MwCheckKeyboardEvent(void);
void		MwHandleMouseStatus(MWCOORD newx, MWCOORD newy, MWCOORD newz, int newbuttons);
void		MwTranslateMouseMessage(HWND hwnd,UINT msg,int hittest,int buttons);
void		MwDeliverMouseEvent(int buttons, int changebuttons,
			MWKEYMOD modifiers);
void		MwDeliverKeyboardEvent(EFI_KEY_DATA      *KeyData, MWKEYMOD modifiers,
			MWSCANCODE scancode, BOOL pressed);
void		MwDeliverExposureEvent(HWND wp, MWCOORD x, MWCOORD y,
			MWCOORD width,MWCOORD height);
void		MwUnionUpdateRegion(HWND wp, MWCOORD x, MWCOORD y,
			MWCOORD width,MWCOORD height, BOOL bUnion);
void		MwMoveCursor(MWCOORD x, MWCOORD y);
void		MwCheckCursor(void);
HWND		MwFindVisibleWindow(MWCOORD x, MWCOORD y);
void		MwCheckMouseWindow(void);
int		strzcpy(char *dst,const char *src,int dstsiz);

/* winuser.c*/
extern int	mwSYSMETRICS_CYCAPTION;
extern int	mwSYSMETRICS_CXBORDER;
extern int	mwSYSMETRICS_CYBORDER;
extern int	mwSYSMETRICS_CXFRAME;
extern int	mwSYSMETRICS_CYFRAME;
extern int	mwSYSMETRICS_CXDOUBLECLK;
extern int	mwSYSMETRICS_CYDOUBLECLK;
extern int	mwSYSMETRICS_CYHSCROLL;
extern int	mwSYSMETRICS_CXHSCROLL;
extern int	mwSYSMETRICS_CXVSCROLL;
extern int	mwSYSMETRICS_CYVSCROLL;

/* wingdi.c*/
extern BOOL	mwERASEMOVE;	/* default repaint algorithm*/
extern long	mwTextCoding;	/* current text encoding*/

/* winmain.c*/
int		MwOpen(void);
void		MwClose(void);
void		MwSelect(BOOL mayWait);
int		MwInitialize(void);
void		MwTerminate(void);
extern	HWND	listwp;			/* list of all windows */
extern	HWND	rootwp;			/* root window pointer */
extern	HWND	focuswp;		/* focus window for keyboard */
extern	HWND	mousewp;		/* window mouse is currently in */
extern	HWND	capturewp;		/* capture window*/
extern  HWND	dragwp;			/* window user is dragging*/
extern	HCURSOR	curcursor;		/* currently enabled cursor */
extern	MWCOORD	cursorx;		/* x position of cursor */
extern	MWCOORD	cursory;		/* y position of cursor */
extern	MWCOORD	cursorz;
extern	MWSCREENINFO	sinfo;		/* screen information */
extern	UINT64	startTicks;	/* tickcount on startup */
extern  int	mwpaintNC;		/* experimental nonclient regions*/
extern  BOOL	mwforceNCpaint;		/* force NC paint for alphablend*/

#if VTSWITCH
/* temp framebuffer vt switch stuff at upper level
 * this should be handled at the lower level, just like vgalib does.
 */
void MwInitVt(void);
int  MwCurrentVt(void);
int  MwCheckVtChange(void);
void MwRedrawVt(int t);
void MwExitVt(void);
extern int mwvterm;
#endif /* VTSWITCH*/

#endif
