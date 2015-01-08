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
