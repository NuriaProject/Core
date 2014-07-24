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

#ifndef NURIA_QTMETAOBJECTWRAPPER_HPP
#define NURIA_QTMETAOBJECTWRAPPER_HPP

#include "runtimemetaobject.hpp"

namespace Nuria {

class QtMetaObjectWrapperPrivate;

/**
 * \brief QtMetaObjectWrapper lets you use a QMetaObject as Nuria::MetaObject
 * 
 * This class is meant to be used for classes you don't have writing access to
 * (In which case you could simply tag it with NURIA_INTROSPECT) or if you
 * don't want to use Tria. You can also use this class to ease transition to
 * Tria. Note that when using Tria, you won't need to manually export types
 * anymore like you need to when using pure Qt.
 * 
 * \note You can instruct Tria to generate code for non-tagged classes using
 * \c -introspect-all or \c -introspect-inheriting!
 * 
 * \note It is recommended to use Tria to generate the code instead of using
 * this class.
 * 
 * \par Behaviour
 * 
 * The wrapper will expose all known public methods (Q_INVOKABLE methods and
 * slots), enums and Qt properties. Information of the base class is
 * not included. Constructors are \b not exposed.
 * 
 * Annotations stored as \c Q_CLASSINFO() will be stored as class annotations
 * with the value as QString. To expose methods which are not public slots
 * prefix them with \c Q_INVOKABLE - This also applies to constructors.
 * 
 */
class NURIA_CORE_EXPORT QtMetaObjectWrapper : public RuntimeMetaObject {
public:
	
	/**
	 * Creates a instance which is populated by \a metaObject.
	 * 
         * There is no further action required to use the MetaObject.
	 */
	QtMetaObjectWrapper (const QMetaObject *metaObject);
	
	/** Destructor. */
	~QtMetaObjectWrapper () override;
	
private:
	
	void installDeleter ();
	void populateAnnotations (const QMetaObject *meta);
	void populateMethods (const QMetaObject *meta);
	void populateEnums (const QMetaObject *meta);
	void populateFields (const QMetaObject *meta);
	
};

}

#endif // NURIA_QTMETAOBJECTWRAPPER_HPP
