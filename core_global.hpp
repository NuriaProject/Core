/* Copyright (c) 2014, The Nuria Project
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *    1. The origin of this software must not be misrepresented; you must not
 *       claim that you wrote the original software. If you use this software
 *       in a product, an acknowledgment in the product documentation would be
 *       appreciated but is not required.
 *    2. Altered source versions must be plainly marked as such, and must not be
 *       misrepresented as being the original software.
 *    3. This notice may not be removed or altered from any source
 *       distribution.
 */

#ifndef NURIA_CORE_GLOBAL_HPP
#define NURIA_CORE_GLOBAL_HPP

#include <qglobal.h>

// Version info
#define NURIA_VERSION 0x000100
#define NURIA_VERSION_STR "0.1.0"

// Tria annotations.
// See Nuria::MetaObject for information.
#ifdef TRIA_RUN
// Clang parses attributes in reversed order
#define NURIA_ANNOTATE(name, ...) \
	__attribute__((annotate(# __VA_ARGS__))) \
	__attribute__((annotate("nuria_annotate:" #name)))
#define NURIA_INTROSPECT __attribute__((annotate("nuria_introspect")))
#define NURIA_SKIP __attribute__((annotate("nuria_skip")))
#else
#define NURIA_ANNOTATE(name, ...)
#define NURIA_INTROSPECT
#define NURIA_SKIP
#endif

// 
#if defined(CORE_LIBRARY)
#  define NURIA_CORE_EXPORT Q_DECL_EXPORT
#else
#  define NURIA_CORE_EXPORT Q_DECL_IMPORT
#endif

#endif // NURIA_CORE_GLOBAL_HPP
