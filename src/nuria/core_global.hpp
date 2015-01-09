/* Copyright (c) 2014-2015, The Nuria Project
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * The NuriaProject Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with The NuriaProject Framework.
 * If not, see <http://www.gnu.org/licenses/>.
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
	__attribute__((annotate("nuria_annotate:" QT_STRINGIFY(#name) "=" #__VA_ARGS__)))
#define NURIA_INTROSPECT __attribute__((annotate("nuria_introspect")))
#define NURIA_SKIP __attribute__((annotate("nuria_skip")))
#define NURIA_READ(field) __attribute__((annotate("nuria_read:" #field)))
#define NURIA_WRITE(field) __attribute__((annotate("nuria_write:" #field)))
#define NURIA_REQUIRE(...) __attribute__((annotate("nuria_require:" #__VA_ARGS__)))
#else
#define NURIA_ANNOTATE(name, ...)
#define NURIA_INTROSPECT
#define NURIA_SKIP
#define NURIA_READ(field) 
#define NURIA_WRITE(field) 
#define NURIA_REQUIRE(...)
#endif

// 
#if defined(NuriaCore_EXPORTS)
#  define NURIA_CORE_EXPORT Q_DECL_EXPORT
#else
#  define NURIA_CORE_EXPORT Q_DECL_IMPORT
#endif

#endif // NURIA_CORE_GLOBAL_HPP
