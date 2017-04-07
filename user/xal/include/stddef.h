/*
 * $FILE: stddef.h
 *
 * stddef file
 *
 * $VERSION$
 *
 * $AUTHOR$
 *
 * $LICENSE:
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef _XAL_STDDEF_H_
#define _XAL_STDDEF_H_

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(_type, _member) __compiler_offsetof(_type,_member)
#else
#define offsetof(_type, _member) ((xmSize_t) &((_type *)0)->_member)
#endif


#endif
