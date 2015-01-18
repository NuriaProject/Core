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

#ifndef NURIA_STRINGUTILS_HPP
#define NURIA_STRINGUTILS_HPP

#include "essentials.hpp"

namespace Nuria {

/**
 * \brief Collection of helpers for strings and string data
 */
class NURIA_CORE_EXPORT StringUtils {
	StringUtils () = delete;
public:
	
	/** Result values for checkForValidUtf8(). */
	enum CheckState {
		
		/** The passed string contains invalid UTF-8 data. */
		Failed = 0,
		
		/**
		 * The passed string has a \b incomplete sequence at its end
		 * that could be fixed by adding more data.
		 */
		Incomplete = 1,
		
		/** The passed string is UTF-8 compliant. */
		Valid = 2,
	};
	
	/**
	 * Checks if \a str of \a length only contains valid UTF-8 data.
	 * If yes, it returns Success and \a pos is not changed. Else,
	 * either Failed or Incomplete are returned. In this case, \a pos is set
	 * to the beginning of the failing sequence.
	 * 
	 * \note \a length must be the length of \a str, not \c -1.
	 */
	static CheckState checkValidUtf8 (const char *string, int length, int &pos);
	
};

} // namespace Nuria

#endif // NURIA_STRINGUTILS_HPP
