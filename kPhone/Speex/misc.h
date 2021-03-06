/* Copyright (C) 2002 Jean-Marc Valin */
/**
   @file misc.h
   @brief Various compatibility routines for Speex
*/
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <qstring.h>

#ifndef MISC_H
#define MISC_H

#ifndef SPEEX_VERSION
#define SPEEX_MAJOR_VERSION 1
#define SPEEX_MINOR_VERSION 0
#define SPEEX_MICRO_VERSION 4
#define SPEEX_EXTRA_VERSION ""
#define SPEEX_VERSION "speex-1.0.4"
#endif

/*Disable some warnings on VC++*/
#ifdef _MSC_VER
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#endif

#define VERY_SMALL 1e-15

#ifndef RELEASE
void print_vec(float *vec, int len, char *name);
#endif

unsigned int be_int(unsigned int i);
unsigned int le_int(unsigned int i);


unsigned short be_short(unsigned short s);
unsigned short le_short(unsigned short s);

/** Speex wrapper for calloc. To do your own dynamic allocation, all you need to do is replace this function, speex_realloc and speex_free */
void *speex_alloc (int size);

/** Speex wrapper for realloc. To do your own dynamic allocation, all you need to do is replace this function, speex_alloc and speex_free */
void *speex_realloc (void *ptr, int size);

/** Speex wrapper for calloc. To do your own dynamic allocation, all you need to do is replace this function, speex_realloc and speex_alloc */
void speex_free (void *ptr);

/** Speex wrapper for mem_move */
void *speex_move (void *dest, void *src, int n);

void speex_error(QString str);

void speex_warning(QString str);

void speex_warning_int(QString str, int val);

void speex_rand_vec(float std, float *data, int len);

float speex_rand(float std);

void _speex_putc(int ch, void *file);

#endif
