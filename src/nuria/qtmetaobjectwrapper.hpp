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
