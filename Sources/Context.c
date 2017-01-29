/*
 * Copyright (c) 2001-2011 Marc Alexander Lehmann <schmorp@schmorp.de>
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
 */

/**
 * Original code is at https://github.com/ramonza/libcoro
 * Stripped down by removing all backends except CORO_ASM
 */

#include <stdlib.h>
#include <string.h>
#include "Context.h"

typedef void (*coro_func)(void *);

#if __arm__ && \
  (defined __ARM_ARCH_7__  || defined __ARM_ARCH_7A__ \
|| defined __ARM_ARCH_7R__ || defined __ARM_ARCH_7M__ \
|| __ARM_ARCH == 7)
#define CORO_ARM 1
#endif

__thread static coro_func coro_init_func;
__thread void *coro_init_arg;
__thread coro_context *new_coro, *create_coro;

static void
coro_init (void)
{
  volatile coro_func func = coro_init_func;
  volatile void *arg = coro_init_arg;

  coro_transfer (new_coro, create_coro);

#if __GCC_HAVE_DWARF2_CFI_ASM && __amd64
  /*asm (".cfi_startproc");*/
  /*asm (".cfi_undefined rip");*/
#endif

  func ((void *)arg);

#if __GCC_HAVE_DWARF2_CFI_ASM && __amd64
  /*asm (".cfi_endproc");*/
#endif

  /* the new coro returned. bad. just abort() for now */
  abort ();
}


#if _WIN32 || __CYGWIN__
#define CORO_WIN_TIB 1
#endif

asm (
   "\t.text\n"
   #if _WIN32 || __CYGWIN__ || __APPLE__
   "\t.globl _coro_transfer\n"
   "_coro_transfer:\n"
   #else
   "\t.globl coro_transfer\n"
   "coro_transfer:\n"
   #endif
   /* windows, of course, gives a shit on the amd64 ABI and uses different registers */
   /* http://blogs.msdn.com/freik/archive/2005/03/17/398200.aspx */
   #if __amd64

     #if _WIN32 || __CYGWIN__
       #define NUM_SAVED 29
       "\tsubq $168, %rsp\t" /* one dummy qword to improve alignment */
       "\tmovaps %xmm6, (%rsp)\n"
       "\tmovaps %xmm7, 16(%rsp)\n"
       "\tmovaps %xmm8, 32(%rsp)\n"
       "\tmovaps %xmm9, 48(%rsp)\n"
       "\tmovaps %xmm10, 64(%rsp)\n"
       "\tmovaps %xmm11, 80(%rsp)\n"
       "\tmovaps %xmm12, 96(%rsp)\n"
       "\tmovaps %xmm13, 112(%rsp)\n"
       "\tmovaps %xmm14, 128(%rsp)\n"
       "\tmovaps %xmm15, 144(%rsp)\n"
       "\tpushq %rsi\n"
       "\tpushq %rdi\n"
       "\tpushq %rbp\n"
       "\tpushq %rbx\n"
       "\tpushq %r12\n"
       "\tpushq %r13\n"
       "\tpushq %r14\n"
       "\tpushq %r15\n"
       #if CORO_WIN_TIB
         "\tpushq %fs:0x0\n"
         "\tpushq %fs:0x8\n"
         "\tpushq %fs:0xc\n"
       #endif
       "\tmovq %rsp, (%rcx)\n"
       "\tmovq (%rdx), %rsp\n"
       #if CORO_WIN_TIB
         "\tpopq %fs:0xc\n"
         "\tpopq %fs:0x8\n"
         "\tpopq %fs:0x0\n"
       #endif
       "\tpopq %r15\n"
       "\tpopq %r14\n"
       "\tpopq %r13\n"
       "\tpopq %r12\n"
       "\tpopq %rbx\n"
       "\tpopq %rbp\n"
       "\tpopq %rdi\n"
       "\tpopq %rsi\n"
       "\tmovaps (%rsp), %xmm6\n"
       "\tmovaps 16(%rsp), %xmm7\n"
       "\tmovaps 32(%rsp), %xmm8\n"
       "\tmovaps 48(%rsp), %xmm9\n"
       "\tmovaps 64(%rsp), %xmm10\n"
       "\tmovaps 80(%rsp), %xmm11\n"
       "\tmovaps 96(%rsp), %xmm12\n"
       "\tmovaps 112(%rsp), %xmm13\n"
       "\tmovaps 128(%rsp), %xmm14\n"
       "\tmovaps 144(%rsp), %xmm15\n"
       "\taddq $168, %rsp\n"
     #else
       #define NUM_SAVED 6
       "\tpushq %rbp\n"
       "\tpushq %rbx\n"
       "\tpushq %r12\n"
       "\tpushq %r13\n"
       "\tpushq %r14\n"
       "\tpushq %r15\n"
       "\tmovq %rsp, (%rdi)\n"
       "\tmovq (%rsi), %rsp\n"
       "\tpopq %r15\n"
       "\tpopq %r14\n"
       "\tpopq %r13\n"
       "\tpopq %r12\n"
       "\tpopq %rbx\n"
       "\tpopq %rbp\n"
     #endif
     "\tpopq %rcx\n"
     "\tjmpq *%rcx\n"

   #elif __i386__

     #define NUM_SAVED 4
     "\tpushl %ebp\n"
     "\tpushl %ebx\n"
     "\tpushl %esi\n"
     "\tpushl %edi\n"
     #if CORO_WIN_TIB
       #undef NUM_SAVED
       #define NUM_SAVED 7
       "\tpushl %fs:0\n"
       "\tpushl %fs:4\n"
       "\tpushl %fs:8\n"
     #endif
     "\tmovl %esp, (%eax)\n"
     "\tmovl (%edx), %esp\n"
     #if CORO_WIN_TIB
       "\tpopl %fs:8\n"
       "\tpopl %fs:4\n"
       "\tpopl %fs:0\n"
     #endif
     "\tpopl %edi\n"
     "\tpopl %esi\n"
     "\tpopl %ebx\n"
     "\tpopl %ebp\n"
     "\tpopl %ecx\n"
     "\tjmpl *%ecx\n"

   #elif CORO_ARM /* untested, what about thumb, neon, iwmmxt? */

     #if __ARM_PCS_VFP
       "\tvpush {d8-d15}\n"
       #define NUM_SAVED (9 + 8 * 2)
     #else
       #define NUM_SAVED 9
     #endif
     "\tpush {r4-r11,lr}\n"
     "\tstr sp, [r0]\n"
     "\tldr sp, [r1]\n"
     "\tpop {r4-r11,lr}\n"
     #if __ARM_PCS_VFP
       "\tvpop {d8-d15}\n"
     #endif
     "\tmov r15, lr\n"

   #elif __mips__ && 0 /* untested, 32 bit only */

    #define NUM_SAVED (12 + 8 * 2)
     /* TODO: n64/o64, lw=>ld */

     "\t.set    nomips16\n"
     "\t.frame  $sp,112,$31\n"
     #if __mips_soft_float
       "\taddiu   $sp,$sp,-44\n"
     #else
       "\taddiu   $sp,$sp,-112\n"
       "\ts.d     $f30,88($sp)\n"
       "\ts.d     $f28,80($sp)\n"
       "\ts.d     $f26,72($sp)\n"
       "\ts.d     $f24,64($sp)\n"
       "\ts.d     $f22,56($sp)\n"
       "\ts.d     $f20,48($sp)\n"
     #endif
     "\tsw      $28,40($sp)\n"
     "\tsw      $31,36($sp)\n"
     "\tsw      $fp,32($sp)\n"
     "\tsw      $23,28($sp)\n"
     "\tsw      $22,24($sp)\n"
     "\tsw      $21,20($sp)\n"
     "\tsw      $20,16($sp)\n"
     "\tsw      $19,12($sp)\n"
     "\tsw      $18,8($sp)\n"
     "\tsw      $17,4($sp)\n"
     "\tsw      $16,0($sp)\n"
     "\tsw      $sp,0($4)\n"
     "\tlw      $sp,0($5)\n"
     #if !__mips_soft_float
       "\tl.d     $f30,88($sp)\n"
       "\tl.d     $f28,80($sp)\n"
       "\tl.d     $f26,72($sp)\n"
       "\tl.d     $f24,64($sp)\n"
       "\tl.d     $f22,56($sp)\n"
       "\tl.d     $f20,48($sp)\n"
     #endif
     "\tlw      $28,40($sp)\n"
     "\tlw      $31,36($sp)\n"
     "\tlw      $fp,32($sp)\n"
     "\tlw      $23,28($sp)\n"
     "\tlw      $22,24($sp)\n"
     "\tlw      $21,20($sp)\n"
     "\tlw      $20,16($sp)\n"
     "\tlw      $19,12($sp)\n"
     "\tlw      $18,8($sp)\n"
     "\tlw      $17,4($sp)\n"
     "\tlw      $16,0($sp)\n"
     "\tj       $31\n"
     #if __mips_soft_float
       "\taddiu   $sp,$sp,44\n"
     #else
       "\taddiu   $sp,$sp,112\n"
     #endif

   #else
     #error unsupported architecture
   #endif
);

void
coro_create (coro_context *ctx, coro_func coro, void *arg, void *sptr, unsigned int ssize)
{
  coro_context nctx;
  if (!coro)
    return;

  coro_init_func = coro;
  coro_init_arg  = arg;

  new_coro    = ctx;
  create_coro = &nctx;

  #if __i386__ || __x86_64__
    ctx->sp = (void **)(ssize + (char *)sptr);
    *--ctx->sp = (void *)abort; /* needed for alignment only */
    *--ctx->sp = (void *)coro_init;
    #if CORO_WIN_TIB
      *--ctx->sp = 0;                    /* ExceptionList */
      *--ctx->sp = (char *)sptr + ssize; /* StackBase */
      *--ctx->sp = sptr;                 /* StackLimit */
    #endif
  #elif CORO_ARM
    /* return address stored in lr register, don't push anything */
  #else
    #error unsupported architecture
  #endif

  ctx->sp -= NUM_SAVED;
  memset (ctx->sp, 0, sizeof (*ctx->sp) * NUM_SAVED);

  #if __i386__ || __x86_64__
    /* done already */
  #elif CORO_ARM
    ctx->sp[0] = coro; /* r4 */
    ctx->sp[1] = arg;  /* r5 */
    ctx->sp[8] = (char *)coro_init; /* lr */
  #else
    #error unsupported architecture
  #endif

  coro_transfer (create_coro, new_coro);
}

#include <stdlib.h>

#ifndef _WIN32
# include <unistd.h>
#endif

#if _POSIX_MAPPED_FILES
# include <sys/mman.h>
# define CORO_MMAP 1
# ifndef MAP_ANONYMOUS
#  ifdef MAP_ANON
#   define MAP_ANONYMOUS MAP_ANON
#  else
#   undef CORO_MMAP
#  endif
# endif
# include <limits.h>
#else
# undef CORO_MMAP
#endif

#if _POSIX_MEMORY_PROTECTION
# ifndef CORO_GUARDPAGES
#  define CORO_GUARDPAGES 4
# endif
#else
# undef CORO_GUARDPAGES
#endif

#if !CORO_MMAP
# undef CORO_GUARDPAGES
#endif

#if !__i386__ && !__x86_64__ && !__powerpc__ && !__arm__ && !__aarch64__ && !__m68k__ && !__alpha__ && !__mips__ && !__sparc64__
# undef CORO_GUARDPAGES
#endif

#ifndef CORO_GUARDPAGES
# define CORO_GUARDPAGES 0
#endif

#if !PAGESIZE
  #if !CORO_MMAP
    #define PAGESIZE 4096
  #else
    static size_t
    coro_pagesize (void)
    {
      static size_t pagesize;

      if (!pagesize)
        pagesize = sysconf (_SC_PAGESIZE);

      return pagesize;
    }

    #define PAGESIZE coro_pagesize ()
  #endif
#endif

int
coro_stack_alloc (struct coro_stack *stack, unsigned int size)
{
  if (!size)
    size = 256 * 1024;

  stack->sptr = 0;
  stack->ssze = ((size_t)size * sizeof (void *) + PAGESIZE - 1) / PAGESIZE * PAGESIZE;

  size_t ssze = stack->ssze + CORO_GUARDPAGES * PAGESIZE;
  void *base;

  #if CORO_MMAP
    /* mmap supposedly does allocate-on-write for us */
    base = mmap (0, ssze, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (base == (void *)-1)
      {
        /* some systems don't let us have executable heap */
        /* we assume they won't need executable stack in that case */
        base = mmap (0, ssze, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (base == (void *)-1)
          return 0;
      }

    #if CORO_GUARDPAGES
      mprotect (base, CORO_GUARDPAGES * PAGESIZE, PROT_NONE);
    #endif

    base = (void*)((char *)base + CORO_GUARDPAGES * PAGESIZE);
  #else
    base = malloc (ssze);
    if (!base)
      return 0;
  #endif

  stack->sptr = base;
  return 1;
}

void
coro_stack_free (struct coro_stack *stack)
{
  #if CORO_MMAP
    if (stack->sptr)
      munmap ((void*)((char *)stack->sptr - CORO_GUARDPAGES * PAGESIZE),
              stack->ssze                 + CORO_GUARDPAGES * PAGESIZE);
  #else
    free (stack->sptr);
  #endif
}

