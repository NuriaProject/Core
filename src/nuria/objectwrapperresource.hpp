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

#ifndef NURIA_OBJECTWRAPPERRESOURCE_HPP
#define NURIA_OBJECTWRAPPERRESOURCE_HPP

#include "resource.hpp"

namespace Nuria {

class ObjectWrapperResourcePrivate;

/**
 * \brief Helper class to wrap QObjects to a Resource
 * 
 * This class lets you expose all signals and slots of a QObject as Resource
 * without having to write any Resource-specific code.
 * 
 * \par Usage
 * There are two modes of operation basically. You can either sub-class this
 * class or use it stand-alone. No matter how you choose, you'll only need to
 * create an instance of this class, passing the resource interface name, the
 * wrapped object and the meta object of the wrapped object:
 * 
 * \code
 * MyQObject *myQObject = ...;
 * Resource *resource = ObjectWrapperResource ("com.example.MyResource", myQObject, myQObject->metaObject (), this);
 * \endcode
 * 
 * Please be aware that QObject::metaObject() is a virtual method, thus you have
 * to pass the class meta object manually when sub-classing:
 * 
 * \code
 * class MyResource : public ObjectWrapperResource {
 * Q_OBJECT
 * public:
 *   MyResource (QObject *parent)
 *   : ObjectWrapperResource ("com.example.MyResource", this, &MyResource::staticMetaObject, parent)
 *   { ... }
 * };
 * \endcode
 * 
 * The wrapper will expose all public slots and signals of the wrapped object.
 * 
 * \note Signals and slot names don't contain their C++ signature unlike Qt
 * signals and slots.
 * 
 * \par Overloaded signals and slots
 * This resource implements the following behaviour on overloaded methods:
 * 
 * \note Signals and slots don't interfere, so a signal and two methods all
 * called "foo" are fine according to the rules layed out below.
 * 
 * \b Signals shall not be overloaded.
 * If a signal is overloaded, a listen() call to this will choose one in a
 * implementation-defined manner. The user should not assume any deterministic
 * behaviour on this, and thus, should not overload signals.
 * 
 * \b Slots can be overloaded with care.
 * When invoking a slot, the first matching one is used. The internal choosing
 * process does the following steps (in this order):
 * 
 * 1. The name has to match. Else, continue to the next one.
 * 2. The amount of passed arguments equals to the expected amount.
 * 3. All passed arguments must be used by the slot. This means a match on the
 * arguments' name.
 * 4. The arguments value is of the expected type or can likely be converted
 * using QVariant::canConvert().
 * 5. If all of the above match, use this slot.
 * 
 * To safely overload methods you should thus choose specific argument names
 * for the C++ methods.
 * 
 */
class NURIA_CORE_EXPORT ObjectWrapperResource : public Resource {
	Q_OBJECT
public:
	
	/**
	 * Creates a wrapper on \a object with \a meta.
	 * 
	 * If \a meta is \c nullptr, then QObject::metaObject() will be called
	 * on \a object.
	 * 
	 * \warning When sub-classing this class, pass
	 * YourType::staticMetaObject as \a meta.
	 */
	explicit ObjectWrapperResource (const QString &interface, QObject *object, const QMetaObject *meta = nullptr,
	                                QObject *parent = nullptr);
	
	/** Destructor. */
	~ObjectWrapperResource ();
	
	/**
	 * Instructs the wrapper to exclude a signal or slot, making it
	 * unavailable to callers of the resource.
	 * 
	 * \a signalOrSlot is the result of a SIGNAL or SLOT macro. Examples:
	 * \code
	 * exclude (SLOT(foo(int)));
	 * exclude (SIGNAL(fooHappened(QString)));
	 * \endcode
	 * 
	 * It's not possible to later include excluded signals or slots.
	 */
	void exclude (const char *signalOrSlot);
	
	// Resource interface
	QString interfaceName () const override;
	Invocation properties (InvokeCallback callback, int timeout = -1) override;
	
protected:
	Invocation invokeImpl (const QString &slot, const QVariantMap &arguments, InvokeCallback callback,
	                       int timeout = -1) override;
	Invocation listenImpl (const QString &signal, InvokeCallback callback) override;
	
private:
	ObjectWrapperResourcePrivate *d_ptr;
	
	
};

} // namespace Nuria

#endif // NURIA_OBJECTWRAPPERRESOURCE_HPP
