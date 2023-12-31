/*

  Copyright(c) 2012-2015 Intel Corporation. All rights reserved.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "sventx.h"
#include "sventx/event.h"
#include <Library/BaseMemoryLib.h>

/**
 * SVEN global state
 */
static sven_header_t sven_hdr = { 0 };

static union u_sven_null_init
{
    sven_handle_t handle;
    sven_header_t header;
} zero = {{ 0 }}; /* for initializations */

/**
 * This handle is used in case if no valid handle could be obtained.
 * Accessing the nullHandle is basically a NOP. It simplifies the
 * rest of the code from testing for null handles, which is often as
 * expensive as the entire operation.
 */
sven_handle_t sventx_nullHandle;

#if !defined(SVEN_PCFG_ENABLE_DEFAULT_SCATTER_WRITE)
/**
 * null-device style default output function
 */
static void nullWriter(psven_handle_t svenh,
                        psven_scatter_prog_t scatterprog,
                        const void * pdesc)
{
}
#endif

/**
 * Initialize the SVENTX library.
 *
 * This function must be called during the start of the platform before any
 * other instrumentation library call. The function initializes the global
 * state data necessary for the library to execute. Passing NULL as state
 * means using the shared global state singleton. Passing a valid pointer
 * allows using multiple SVENTX state context structures in parallel.
 *
 * @param header Pointer to SVENTX global state structure or NULL for default.
 * @param pfinit Pointer to platform initialization function or 0 if not used.
 * @param init_param Value passed to the the platform init hook function.
 */
SVEN_EXPORT void SVEN_CALLCONV
sventx_Init(psven_header_t header, sven_inithook_t pfinit, const void * init_param)
{
    if (0 == header) {
        /* No user supplied global state storage, use internal default state
         */
        header = & sven_hdr;
    }

    CopyMem (header, &zero.header, sizeof (sven_header_t));
    header->svh_version =  SVEN_VERSION_CODE;
#if defined(SVEN_PCFG_ENABLE_HOT_GATE)
    header->svh_hot     =  SVEN_HOT_ENABLE_DEFAULT;
#endif

#if defined(SVEN_PCFG_ENABLE_DEFAULT_SCATTER_WRITE)
    header->svh_writer  = sth_ScatterWrite;
#else
    header->svh_writer  = nullWriter;
#endif

    CopyMem (&sventx_nullHandle, &zero.header, sizeof (sven_header_t));

    /* call platform state initialization hook if defined
     */
    if (pfinit != (sven_inithook_t)0)
    {
        (*pfinit)(header, init_param);
    }
}
/**
 * Destroy the SVENTX library state.
 *
 * This function must be called during shutdown of the platform to release
 * any SVENTX resources.
 *
 * @param header Pointer to library state or NULL to use shared default.
 * @param pfdestroy Pointer to platform state destroy function or 0
 *                  if not used.
 */
SVEN_EXPORT void SVEN_CALLCONV
sventx_Destroy(psven_header_t header, sven_destroyhook_t pfdestroy)
{
    if (0 == header) {
        /* No user supplied global state storage, use internal default state
         */
        header = & sven_hdr;
    }

    /* call platform state destroy hook first, if defined
     */
    if ((sven_destroyhook_t)0 != pfdestroy)
    {
        (*pfdestroy)(header);
    }
}

/**
 *  Initialize a SVEN handle.
 *
 * @oparam header Pointer to library state or NULL to use shared default.
 * @param svh Pointer to new/uninitialized SVEN handle
 * @param init_param Value passed to the the platform handle init function
 * @param fromheap 1 of heap allocated handle, 0 otherwise
 */
SVEN_EXPORT  psven_handle_t SVEN_CALLCONV
sventx_InitHandle(psven_header_t header,
                  psven_handle_t svh,
                  const void * init_param,
                  sven_u32_t fromHeap)
{
    if ((psven_handle_t)0 == svh)
    {
        return &sventx_nullHandle;
    }

    if (0 == header) {
        /* No user supplied global state storage, use internal default state
         */
        header = & sven_hdr;
    }

    CopyMem (svh, &zero.handle, sizeof (psven_handle_t));

    svh->svh_header         = header;
    svh->svh_flags.shf_alloc= fromHeap ? 1:0;

#if defined(SVEN_PCFG_ENABLE_SEQUENCE_COUNT)
    svh->svh_sequence_count = 0;
#endif

    /* call platform handle initialization hook if defined
     */
#if defined(SVEN_PCFG_ENABLE_PLATFORM_HANDLE_DATA)
    if ((sven_inithandle_hook_t)0 != svh->svh_header->svh_inith)
    {
        svh->svh_header->svh_inith(svh, init_param);
    }
#endif
    return svh;
}

/**
 *  Release a SVEN handle.
 *
 * @param svh Pointer to initialized SVEN handle
 */
SVEN_EXPORT void SVEN_CALLCONV sventx_DeleteHandle(psven_handle_t svh)
{
    if (&sventx_nullHandle != svh && (psven_handle_t)0 != svh)
    {

#if defined(SVEN_PCFG_ENABLE_PLATFORM_HANDLE_DATA)
        /* call platform handle release hook if defined
         */
        if ((sven_releasehandle_hook_t)0 != svh->svh_header->svh_releaseh)
        {
            svh->svh_header->svh_releaseh(svh);
        }
#endif

#if defined(SVEN_PCFG_ENABLE_HEAP_MEMORY)
        if (0 != svh->svh_flags.shf_alloc)
        {
            SVEN_HEAP_FREE(svh);
        } else
#endif
        {
            *svh = zero.handle;
        }
    }
}
