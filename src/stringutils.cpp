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

#include "nuria/stringutils.hpp"
#include "nuria/bitutils.hpp"

// SSE2 header. Available for GCC and MSVC it seems.
#include <emmintrin.h>

#include <QString>

static inline bool hasHighBitSet (const uint8_t *ptr) {
	__m128i piece = _mm_loadu_si128 (reinterpret_cast< const __m128i * > (ptr));
	return (_mm_movemask_epi8 (piece) != 0); // result = ptr[0] & 0x80 | ptr[1] & 0x80 | ...
}

Nuria::StringUtils::CheckState Nuria::StringUtils::checkValidUtf8 (const char *string, int length, int &pos) {
	const uint8_t *str = reinterpret_cast< const uint8_t * > (string);
	
	for (int i = 0; i < length;) {
		if (length - i >= sizeof(__m128i) && !hasHighBitSet (str + i)) {
			i += sizeof(__m128i);
			continue;
		}
		
		if (str[i] < 0x80) { // ASCII character
			i++;
			continue;
		}
		
		int needed = clz (~(str[i]) & 0xFF) - 24;
		
		// Unexpected value for 'needed'?
		if (Q_UNLIKELY(needed < 2 || needed > 4)) {
			pos = i;
			return Failed;
		}
		
		// Too short?
		if (Q_UNLIKELY(length - i - needed < 0)) {
			pos = i;
			return Incomplete;
		}
		
		// Check for 10xx xxxxx in all following bytes
		if ((str[i + 1] >> 6 != 2) ||
		    (needed > 2 && str[i + 2] >> 6 != 2) ||
		    (needed > 3 && str[i + 3] >> 6 != 2)) {
			pos = i;
			return Failed;
		}
		
		// Decode value
		uint32_t v = 0;
		switch (needed) {
		case 2:
			v = ((str[i] & 0x1F) << 6) | (str[i + 1] & 0x7F);
			break;
		case 3:
			v = ((str[i] & 0x0F) << 12) | ((str[i + 1] & 0x7F) << 6) | (str[i + 2] & 0x7F);
			break;
		case 4:
			v = ((str[i] & 0x07) << 18) | ((str[i + 1] & 0x7F) << 12) |
			    ((str[i + 2] & 0x7F) << 6) | (str[i + 3] & 0x7F);
			break;
		}
		
		// Make sure that the value is inside the allowed space.
		// Also check for overlong encodings.
		if (v > 0x10FFFF || // Highest allowed index
		    v == 0xC0 || v == 0xC1 || // Disallowed
		    v < 0x80 || // Inflated to two bytes
		    (needed > 2 && v <= 0x07FF) || // Inflated
		    (needed > 3 && v <= 0xFFFF) || // Inflated
		    (v >= 0xD800 && v <= 0xDFFF) // Surrogates
		    ) {
			pos = i;
			return Failed;
		}
		
		// Okay, next.
		i += needed;
	}
	
	// Done.
	return Valid;
}
