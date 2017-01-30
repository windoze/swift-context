/*
 * Copyright (c) 2001-2012 Marc Alexander Lehmann <schmorp@schmorp.de>
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 *
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License ("GPL") version 2 or any later version,
 * in which case the provisions of the GPL are applicable instead of
 * the above. If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the BSD license, indicate your decision
 * by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the BSD or the GPL.
 *
 * This library is modelled strictly after Ralf S. Engelschalls article at
 * http://www.gnu.org/software/pth/rse-pmt.ps. So most of the credit must
 * go to Ralf S. Engelschall <rse@engelschall.com>.
 *
 * This coroutine library is very much stripped down. You should either
 * build your own process abstraction using it or - better - just use GNU
 * Portable Threads, http://www.gnu.org/software/pth/.
 *
 */

/**
 * Original code is at https://github.com/ramonza/libcoro
 * Stripped down by removing all backends except CORO_ASM
 */

#include <stddef.h>

/*
 * A coroutine state is saved in the following structure. Treat it as an
 * opaque type. errno and sigmask might be saved, but don't rely on it,
 * implement your own switching primitive if you need that.
 */

typedef struct coro_context
{
  void **sp; /* must be at offset 0 */
} coro_context;


void coro_create (coro_context *ctx, /* an uninitialised coro_context */
                  void (*coro)(void *),    /* the coroutine code to be executed */
                  void *arg,         /* a single pointer passed to the coro */
                  void *sptr,        /* start of stack area */
                  unsigned int ssze);      /* size of stack area in bytes */

#if __i386__ || __x86_64__
void __attribute__ ((__noinline__, __regparm__(2)))
#else
void __attribute__ ((__noinline__))
#endif
coro_transfer (coro_context *prev, coro_context *next);

typedef struct coro_stack
{
  void *sptr;
  size_t ssze;
} coro_stack;

int coro_stack_alloc (coro_stack *stack, size_t size);

void coro_stack_free (coro_stack *stack);
