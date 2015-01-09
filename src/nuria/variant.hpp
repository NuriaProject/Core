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

#ifndef NURIA_VARIANT_HPP
#define NURIA_VARIANT_HPP

#include "essentials.hpp"
#include <QVariant>

namespace Nuria {

/**
 * \brief Utilities for working with QVariants.
 */
class NURIA_CORE_EXPORT Variant {
public:
	
	/**
	 * Builds a QVariantList out of all arguments passed to the method.
	 * This does the same as
	 * \codeline QVariantList () << QVariant::fromValue (first) << ...
	 * But it's easier and shorter to read and write.
	 * 
	 * \note This method implicitly converts passed arguments of type
	 * const char* to QString. You can disable this by defining
	 * \c NURIA_NO_CHAR_ARRAY_TO_QSTRING before including this header.
	 */
	template< typename ... Items >
	static QVariantList buildList (const Items &... items) {
		QVariantList list;
		buildListImpl (list, items ...);
		return list;
	}
	
	/**
	 * Steals the pointer from \a variant and returns it. Works only for
	 * pointer types, that is, \c T* but not T. If \a variant is invalid,
	 * \c nullptr is returned. After return, \a variant will be invalid if
	 * it contained a pointer.
	 * 
	 * \note Ownership of the returned pointer is transferred to the caller.
	 */
	static void *stealPointer (QVariant &variant);
	
	/**
	 * Returns the pointer from \a variant, if \a variant contains a pointer
	 * type. Works both for \c T and \c T*. Ownership of the returned
	 * pointer is not taken from \a variant and \a variant itself is not
	 * changed. If the operation failed, \c nullptr is returned.
	 */
	static void *getPointer (QVariant &variant);
	
private:
	
	/** \internal Helper for buildListImpl. */
	template< typename T > static inline const T &passThrough (const T &t) { return t; }
	
	// Simplifies usage for buildList().
#ifndef NURIA_NO_CHAR_ARRAY_TO_QSTRING
	static inline QString passThrough (const char *str) { return QString (str); }
#endif
	
	/** \internal Helper for buildList() */
	static inline void buildListImpl (QVariantList &) { }
	
	// 
	template< typename T, typename ... Items >
	static inline void buildListImpl (QVariantList &list, const T &cur, const Items & ... items) {
		list.append (QVariant::fromValue (passThrough (cur)));
		buildListImpl (list, items ...);
	}
	
	/** \internal Not used */
	Variant () = delete;
	
};

}

#endif // NURIA_VARIANT_HPP
