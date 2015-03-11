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

#include "nuria/objectwrapperresource.hpp"
#include <nuria/callback.hpp>
#include <nuria/logger.hpp>

#include <QMetaObject>
#include <QMetaMethod>
#include <QVector>

struct Connection {
	Nuria::Resource::InvokeCallback callback;
	int id;
};

struct Method {
	QVector< int > types;
	QVector< QString > names;
	int propertyIndex = 0;
};

enum FindMethodResults {
	UnknownMethod = -1,
	UnknownArguments = -2
};

namespace Nuria {
class ObjectWrapperResourcePrivate : public QObject {
public:
	
	ObjectWrapperResourcePrivate (QObject *parent) : QObject (parent) { }
	
	const QMetaObject *meta;
	QObject *object;
	QString interface;
	Resource::PropertyList propertyList;
	
	QMap< int, Method > methods;
	QMultiMap< int /* signal method id */, Connection > connections;
	int connectionCounter = 0;
	
	Invocation properties (Resource::InvokeCallback callback, ObjectWrapperResource *instance);
	Resource::PropertyList generatePropertyList ();
	Resource::InvokeResultState invoke (const QString &slot, const QVariantMap &arguments,
	                                    Resource::InvokeCallback callback);
	int findMethod (QMetaMethod::MethodType type, const QString &name, const QVariantMap &arguments);
	
	void initMethods ();
	void addMethodToPropertyList (QMetaMethod method, Method &descriptor);
	void removeMethodFromPropertyList (int index);
	void connectToSignal (QMetaMethod method);
	Method createMetaMethod (QMetaMethod method);
	int deliverSignalEmission (int index, void **args);
	void removeConnection (int id);
	
	// Forward signal connections
	int qt_metacall (QMetaObject::Call call, int index, void **args) override;
	
};

struct SignalConnectionInterface : public Invocation::Interface {
        SignalConnectionInterface (int id, ObjectWrapperResourcePrivate *d)
	        : connectionId (id), d_ptr (d) {}
	
	void cancel () {
		this->d_ptr->removeConnection (this->connectionId);
	}
        
        int connectionId;
        ObjectWrapperResourcePrivate *d_ptr;
};

}

Nuria::ObjectWrapperResource::ObjectWrapperResource (const QString &interface, QObject *object, const QMetaObject *meta,
                                                     QObject *parent)
        : Resource (parent), d_ptr (new ObjectWrapperResourcePrivate (this))
{
	
	this->d_ptr->meta = (meta) ? meta : metaObject ();
	this->d_ptr->object = object;
	this->d_ptr->interface = interface;
	
	// Populate meta structures
	if (this->d_ptr->meta) {
		this->d_ptr->initMethods ();
	}
	
}

Nuria::ObjectWrapperResource::~ObjectWrapperResource () {
	delete this->d_ptr;
}

void Nuria::ObjectWrapperResource::exclude (const char *signalOrSlot) {
	Q_ASSERT(signalOrSlot);
	
	// Find method
	int index = 0;
	if (signalOrSlot[0] == '1') { // Slot
		index = this->d_ptr->meta->indexOfSlot (signalOrSlot + 1);
	} else { // Signal
		index = this->d_ptr->meta->indexOfSignal (signalOrSlot + 1);
	}
	
	// Sanity check
	if (index < 0) {
		nError() << "Failed to find signal or slot" << signalOrSlot
		         << "in class" << this->d_ptr->meta->className ();
		return;
	}
	
	// Remove entry
	Method descriptor = this->d_ptr->methods.take (index);
	this->d_ptr->removeMethodFromPropertyList (descriptor.propertyIndex);
	
}

QString Nuria::ObjectWrapperResource::interfaceName () const {
	return this->d_ptr->interface;
}

Nuria::Invocation Nuria::ObjectWrapperResource::properties (Nuria::Resource::InvokeCallback callback, int timeout) {
	Q_UNUSED(timeout);
	return this->d_ptr->properties (callback, this);
}

Nuria::Invocation Nuria::ObjectWrapperResource::invokeImpl (const QString &slot, const QVariantMap &arguments,
                                                            InvokeCallback callback, int timeout) {
	Q_UNUSED(timeout);
	
	if (slot.isEmpty ()) {
		return this->d_ptr->properties (callback, this);
	}
	
	// Try to invoke
	InvokeResultState result = this->d_ptr->invoke (slot, arguments, callback);
	if (result != Success) {
		callback (result, slot);
	}
	
	// Done
	return Invocation (this);
}

Nuria::Invocation Nuria::ObjectWrapperResource::listenImpl (const QString &signal, InvokeCallback callback) {
	int index = this->d_ptr->findMethod (QMetaMethod::Signal, signal, QVariantMap ());
	if (index < 0) { // Not found?
		callback (Resource::UnknownError, signal);
		return Invocation (this);
	}
	
	// Register
	int id = this->d_ptr->connectionCounter++;
	this->d_ptr->connections.insert (index, Connection { callback, id });
	
	// Return invocation
	Invocation::Interface *interface = new SignalConnectionInterface (id, this->d_ptr);
	return Invocation (this, QSharedPointer< Invocation::Interface > (interface));
}

void Nuria::ObjectWrapperResourcePrivate::addMethodToPropertyList (QMetaMethod method, Method &descriptor) {
	Nuria::Resource::Property::Type type = (method.methodType () == QMetaMethod::Signal)
	                                       ? Nuria::Resource::Property::Signal
	                                       : Nuria::Resource::Property::Slot;
	QString name = QString::fromLatin1 (method.name ());
	int resultType = method.returnType ();
	Nuria::Resource::ArgumentTypeMap args;
	
	// Convert descriptor to argument type map
	for (int i = 0; i < descriptor.names.length (); i++) {
		args.insert (descriptor.names.at (i), descriptor.types.at (i));
	}
	
	// Add to list
	descriptor.propertyIndex = this->propertyList.length ();
	this->propertyList.append (Nuria::Resource::Property (type, name, args, resultType));
	
}

void Nuria::ObjectWrapperResourcePrivate::removeMethodFromPropertyList (int index) {
	this->propertyList.remove (index);
	
	// Update method descriptors
	for (auto it = this->methods.begin (), end = this->methods.end (); it != end; ++it) {
		if (it->propertyIndex > index) { // Update identifier if it's higher than 'index'
			it->propertyIndex--;
		}
		
	}
	
}

Nuria::Invocation Nuria::ObjectWrapperResourcePrivate::properties (Resource::InvokeCallback callback,
                                                                   ObjectWrapperResource *instance) {
	callback (Resource::Success, QVariant::fromValue (this->propertyList));
	return Nuria::Invocation (instance);
}

QVariantList argumentsToList (const Method &descriptor, const QVariantMap &map) {
	QVariantList list;
	
	for (int i = 0, count = descriptor.names.length (); i < count; ++i) {
		list.append (map.value (descriptor.names.at (i)));
	}
	
	// Done.
	return list;
}

Nuria::Resource::InvokeResultState Nuria::ObjectWrapperResourcePrivate::invoke (const QString &slot,
                                                                                const QVariantMap &arguments,
                                                                                Resource::InvokeCallback callback) {
	// Find slot
	int idx = findMethod (QMetaMethod::Slot, slot, arguments);
	if (idx < 0) {
		return (idx == UnknownArguments) ? Resource::BadArgumentError : Resource::UnknownError;
	}
	
	// 
	QMetaMethod method = this->meta->method (idx);
	QByteArray signature = method.methodSignature ();
	QVariantList args = argumentsToList (this->methods.value (idx), arguments);
	
	// Invoke
	Callback invoker (this->object, signature.constData () - 1);
	QVariant result = invoker.invoke (args);
	
	// Done.
	callback (Resource::Success, result);
	return Resource::Success;
}

static bool hasMethodRequiredArguments (const Method &method, const QVariantMap &arguments) {
	if (arguments.count () != method.names.length ()) {
		return false;
	}
	
	// Check that all arguments are expected
	for (int i = 0; i < method.names.length (); i++) {
		int type = method.types.at (i);
		const QString &current = method.names.at (i);
		
		// Check that we have a value for the argument.
		if (!arguments.contains (current) && arguments.value (current).userType () != type) {
			return false;
		}
		
	}
	
	// Ok
	return true;
}

int Nuria::ObjectWrapperResourcePrivate::findMethod (QMetaMethod::MethodType type, const QString &name,
                                                     const QVariantMap &arguments) {
	int error = UnknownMethod;
	
	int i = QObject::staticMetaObject.methodCount ();
	for (; i < this->meta->methodCount (); i++) {
		QMetaMethod m = this->meta->method (i);
		const Method &method = this->methods.value (i);
		
		if (m.methodType () == type && m.name () == name) {
			if (type == QMetaMethod::Signal || hasMethodRequiredArguments (method, arguments)) {
				return i;
			} else {
				error = UnknownArguments;
			}
			
		}
		
	}
	
	// Not found
	return error;
}

void Nuria::ObjectWrapperResourcePrivate::initMethods () {
	
	// Iterate over the objects signals and slots
	for (int i = this->meta->methodOffset (); i < this->meta->methodCount (); i++) {
		QMetaMethod m = this->meta->method (i);
		QMetaMethod::MethodType type = m.methodType ();
		
		// Store public slots and signals
		if (type == QMetaMethod::Signal || (type == QMetaMethod::Slot && m.access () == QMetaMethod::Public)) {
			Method descriptor = createMetaMethod (m);
			addMethodToPropertyList (m, descriptor);
			this->methods.insert (m.methodIndex (), descriptor);
		}
		
		// Connect to signals
		if (type == QMetaMethod::Signal) {
			connectToSignal (m);
		}
		
	}
	
}

void Nuria::ObjectWrapperResourcePrivate::connectToSignal (QMetaMethod method) {
	// Connect objects' signal to our homegrown qt_metacall().
	int index = method.methodIndex ();
	if (!QMetaObject::connect (this->object, index, this, index, Qt::DirectConnection)) {
		nError() << "connect() failed on" << this->object << "for signal" << method.methodSignature ();
	}
	
}

Method Nuria::ObjectWrapperResourcePrivate::createMetaMethod (QMetaMethod method) {
	Method m;
	QList< QByteArray > names = method.parameterNames ();
	for (int i = 0, count = names.length (); i < count; ++i) {
		 m.names.append (QString::fromLatin1 (names.at (i)));
		 m.types.append (method.parameterType (i));
	}
	
	// Done
	return m;
}

int Nuria::ObjectWrapperResourcePrivate::deliverSignalEmission (int index, void **args) {
	const Method self = this->methods.value (index);
	
	// Convert signal arguments to a map
	QVariantMap arguments;
	int count = args ? self.names.length () : 0;
	for (int i = 0; i < count; ++i) {
		const QString &name = self.names.at (i);
		
		if (self.types.at (i) == QMetaType::QVariant) {
			arguments.insert (name, *reinterpret_cast< QVariant * > (args[i + 1]));
		} else {
			arguments.insert (name, QVariant (self.types.at (i), args[i + 1]));
		}
		
	}
	
	// Invoke all connected listeners
	auto it = this->connections.lowerBound (index);
	auto end = this->connections.upperBound (index);
	for (; it != end; ++it) {
		it->callback (Resource::Success, arguments);
	}
	
}

void Nuria::ObjectWrapperResourcePrivate::removeConnection (int id) {
	for (auto it = this->connections.begin (), end = this->connections.end (); it != end; ++it) {
		if (it->id == id) {
			this->connections.erase (it);
			return;
		}
		
	}
	
}

int Nuria::ObjectWrapperResourcePrivate::qt_metacall (QMetaObject::Call call, int index, void **args) {
	
	// Ask QObject ..
	int idx = QObject::qt_metacall (call, index, args);
	if (call != QMetaObject::InvokeMetaMethod || idx < 0) {
		return idx;
	}
	
	// Sanity check
	if (index >= this->meta->methodCount ()) {
		return -1;
	}
	
	// Deliver to our listeners
	if (this->connections.contains (index)) {
		deliverSignalEmission (index, args);
	}
	
	return idx;
}
