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

#ifndef NURIA_ESSENTIALS_HPP
#define NURIA_ESSENTIALS_HPP

#include "core_global.hpp"
#include <cstdint>

namespace Nuria {

/** \internal Helper for Nuria::jenkinsHash */
constexpr uint32_t jenkinsOne (uint32_t hash, const char *key, int len) {
	return (len < 1)
		? hash
		: jenkinsOne ((hash + *key + ((hash + *key) << 10)) ^
			      ((hash + *key + ((hash + *key) << 10)) >> 6),
			      key + 1, len - 1);
}

/**
 * \brief constexpr implementation of jenkins hashing algorithm.
 * 
 * This method will be evaluated by the compiler at compile-time, which is
 * useful when needing a simple hash of compile-time constants.
 * 
 * Used in Nuria::Debug for quick checks if a specific module shall not be
 * printed.
 * 
 */
constexpr uint32_t jenkinsHash (const char *key, size_t len) {
	return ((jenkinsOne (0, key, len) + (jenkinsOne (0, key, len) << 3)) ^
	        ((jenkinsOne (0, key, len) + (jenkinsOne (0, key, len) << 3)) >> 11)) +
	       (((jenkinsOne (0, key, len) + (jenkinsOne (0, key, len) << 3)) ^
		((jenkinsOne (0, key, len) + (jenkinsOne (0, key, len) << 3)) >> 11)) << 15);
}

}

#endif // NURIA_ESSENTIALS_HPP
