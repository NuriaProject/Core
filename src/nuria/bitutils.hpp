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

#ifndef NURIA_BITUTILS_HPP
#define NURIA_BITUTILS_HPP

#include <qcompilerdetection.h>

// Portability support header
namespace Nuria {

/**
 * \fn Nuria::ffs(int i)
 * 
 * Returns the index of the first set bit. Evaluates to the compiler internal
 * implementation.
 */

/**
 * \fn Nuria::clz(int i)
 * 
 * Returns the number of leading zeros in \a i. Evaluates to the compiler
 * internal implementation where available.
 */

#if defined(Q_CC_MINGW) || defined (Q_CC_GNU) || defined (Q_CC_CLANG)
static inline int ffs (int i) { return __builtin_ffs (i); }
static inline int clz (int i) { return __builtin_clz (i); }
#elif defined(Q_CC_MSVC)
static inline int ffs (int i) {
	usigned long r;
	if (_BitScanForward (&r, i) == 0) {
		return 0;
	}
	
	return r;
}

static inline int clz (int i) {
	usigned long r;
	if (_BitScanReverse (&r, i) == 0) {
		return 0;
	}
	
	return 31 - r;
}
#else
#error "Please provide a ffs (Find First Set Bit) function for your platform/compiler"
#endif

}

#endif // NURIA_BITUTILS_HPP

