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

#include "nuria/callback.hpp"

#include <QMetaMethod>
#include <QMetaObject>
#include <QAtomicInt>
#include <QPointer>
#include <QThread>

#include "nuria/logger.hpp"

// Callback types. We store them outside to easily add more options in the
// future without rendering the Callback to be too heavy.
struct CallbackSlot {
	Qt::ConnectionType type;
	QPointer< QObject > qobj;
	QMetaMethod method;
	const char *name;
	
};

// 
namespace Nuria {
class CallbackPrivate : public QSharedData {
public:
	
	~CallbackPrivate () {
		clear ();
		freeBoundValues ();
	}
	
	void clear () {
		switch (type) {
		case Callback::Invalid: break;
		case Callback::Slot:
			delete ptr.slot;
			break;
		default:
			delete ptr.base;
			break;
		}
		
	}
	
	void freeBoundValues () {
		for (int i = 0; i < boundCount; i++) {
			QMetaType::destroy (boundTypes[i], boundValues[i]);
		}
		
		// 
		delete[] boundValues;
		delete[] boundTypes;
		
		// 
		boundValues = nullptr;
		boundTypes = nullptr;
		boundCount = 0;
	}
	
	// Data
	Callback::Type type = Callback::Invalid;
	union {
		Callback::TrampolineBase *base;
		CallbackSlot *slot;
	} ptr;
	
	// Binding
	void **boundValues = nullptr;
	int *boundTypes = nullptr;
	int boundCount = 0;
	bool variadic = false;
	
	// 
	int retType = 0;
	QList< int > args;
};
}

Nuria::Callback::Callback ()
	: d (new CallbackPrivate)
{
	this->d->ref.ref ();
}

Nuria::Callback::Callback (QObject *receiver, const char *slot, bool variadic,
                           Qt::ConnectionType connectionType)
	: d (new CallbackPrivate)
{
	
	this->d->ref.ref ();
	this->d->variadic = variadic;
	setCallback (receiver, slot, connectionType);
	
}

Nuria::Callback::Callback (const Callback &other)
	: d (other.d)
{
	this->d->ref.ref ();
	
}

Nuria::Callback::~Callback () {
	if (!this->d->ref.deref ()) {
		delete this->d;
	}
	
}

Nuria::Callback &Nuria::Callback::operator= (const Callback &other) {
	other.d->ref.ref ();
	if (!this->d->ref.deref ()) {
		delete this->d;
	}
	
	this->d = other.d;
	return *this;
}

bool Nuria::Callback::operator== (const Callback &other) const {
	return (this->d == other.d);
}

bool Nuria::Callback::isValid () const {
	return (this->d && this->d->type != Invalid);
}

Nuria::Callback::Type Nuria::Callback::type () const {
	return this->d->type;
}

bool Nuria::Callback::isVariadic () const {
	return this->d->variadic;
}

void Nuria::Callback::setVariadic (bool variadic) {
	this->d->variadic = variadic;
}

int Nuria::Callback::returnType () const {
	return this->d->retType;
}

QList< int > Nuria::Callback::argumentTypes () const {
	return this->d->args;
}

bool Nuria::Callback::setCallback (QObject *receiver, const char *slot, Qt::ConnectionType connectionType) {
	
	this->d->freeBoundValues ();
	this->d->clear ();
	
	this->d->type = Slot;
	this->d->ptr.slot = new CallbackSlot;
	this->d->ptr.slot->qobj = receiver;
	this->d->ptr.slot->name = slot + 1;
	this->d->ptr.slot->type = connectionType;
	
	// Read arguments
	const QMetaObject *meta = receiver->metaObject ();
	int idx = meta->indexOfMethod (slot + 1);
	
	// Slot not found?
	if (idx == -1) {
		nError() << "Failed to find method" << slot << "in" << receiver;
		this->d->clear ();
		this->d->type = Invalid;
		return false;
	}
	
	// Read return type
	QMetaMethod method = meta->method (idx);
	this->d->retType = QMetaType::type (method.typeName ());
	
	// Read argument types
	QList< QByteArray > argNames (method.parameterTypes ());
	for (int i = 0; i < argNames.length (); i++) {
		this->d->args.append (QMetaType::type (argNames.at (i).constData ()));
	}
	
	// Store slot method
	this->d->ptr.slot->method = method;
	
	//
	return true;
}

void Nuria::Callback::bindList (const QVariantList &arguments) {
	
	//
	this->d->freeBoundValues ();
	if (arguments.isEmpty ()) {
		return;
	}
	
	// Acquire memory
	int count = arguments.length ();
	void **args = new void *[count];
	int *types = new int[count];
	
	// Convert the QVariantList...
	for (int i = 0; i < count; i++) {
		QVariant cur = arguments.at (i);
		
		// Convert now if needed.
		const void *instance = 0;
		
		if (cur.userType () == qMetaTypeId< Nuria::Callback::Placeholder > ()) {
			args[i] = new Placeholder (cur.value< Placeholder > ());
			types[i] = cur.userType ();
			continue;
			
		} else if (cur.userType () != this->d->args.at (i)) {
			types[i] = cur.userType ();
			
			if (cur.convert (this->d->args.at (i))) {
				instance = cur.constData ();
				types[i] = this->d->args.at (i);
			}
			
		} else {
			types[i] = cur.userType ();
			instance = cur.constData ();
		}
		
		// Copy new instance...
		args[i] = QMetaType::create (types[i], instance);
	}
	
	// Store and done.
	this->d->boundCount = count;
	this->d->boundValues = args;
	this->d->boundTypes = types;
	
}

/**
 * Helper function. Puts \a value at \a pos into \a args.
 * If \a valType doesn't match \a type, it will be converted. If it can't be
 * converted, a default constructed value will be inserted and \c false will
 * be returned. \a off is the offset in \a args.
 */
static bool argumentHelper (void **args, bool *delMe, void *value, int valType,
			    int pos, int off, int type) {
	
	// Construct anyway?
	if (!value) {
		void *inst = QMetaType::create (type, 0);
		
		args[pos + off] = inst;
		delMe[pos] = true;
		
		if (type < QMetaType::Double) {
			*(int *)inst = 0;
		} else if (type == QMetaType::Double) {
			*(double *)inst = 0.0f;
		}
		
		return false;
	}
	
	// Expects a QVariant?
	if (type == QMetaType::QVariant) {
		args[pos + off] = (valType == QMetaType::QVariant)
		                  ? reinterpret_cast< QVariant * > (value)
		                  : new QVariant (valType, value);
		delMe[pos] = (valType != QMetaType::QVariant);
		return true;
	}
	
	// Do we have the value in a QVariant?
	if (valType == QMetaType::QVariant) {
		QVariant *variant = reinterpret_cast< QVariant * > (value);
		valType = variant->userType ();
		value = const_cast< void * > (variant->constData ());
	}
	
	// Check value
	if (valType == type) {
		args[pos + off] = value;
		delMe[pos] = false;
		return true;
	}
	
	// Try to convert
	QVariant v (valType, value);
	if (!v.convert (type)) {
		// Conversion failed.
		return argumentHelper (args, delMe, 0, 0, pos, off, type);
		
	}
	
	// Conversion succeeded!
	args[pos + off] = QMetaType::create (type, v.constData ());
	
	delMe[pos] = true;
	return true;
}

QVariant Nuria::Callback::invoke (const QVariantList &arguments) const {
	
	// Sanity check
	if (this->d->type == Invalid) {
		return QVariant();
	}
	
	// Is this a variadic callback?
	if (this->d->variadic) {
		return invokePrepared ({ QVariant(arguments) });
	}
	
	return invokePrepared (arguments);
}

QVariant Nuria::Callback::invokePrepared (const QVariantList &arguments) const {
	
	// Convert the QVariantList...
	int count = arguments.length ();
	void *args[count];
	int types[count];
	
	for (int i = 0; i < count; i++) {
		const QVariant &cur = arguments.at (i);
		types[i] = cur.userType ();
		args[i] = const_cast< void * > (cur.constData ());
	}
	
	// Invoke!
	return invokeInternal (count, args, types);
	
}

QVariant Nuria::Callback::invoke (int count, void **args, int *types) const {
	if (this->d->type == Invalid) {
		return QVariant();
	}
	
	// Variadic?
	if (this->d->variadic) {
		QVariantList list;
		
		void *argument[] = { &list };
		int type = QMetaType::QVariantList;
		
		// Convert args/types to QVariantList
		for (int i = 0; i < count; i++) {
			list.append (QVariant (types[i], args[i]));
		}
		
		// Invoke.
		return invokeInternal (1, argument, &type);
		
	}
	
	return invokeInternal (count, args, types);
}

class InvokeCleanupHelper {
public:
	void **values;
	const QList< int > *types;
	bool *remove;
	
	InvokeCleanupHelper (void **arr, const QList< int > *type, bool *toRemove)
		: values (arr), types (type), remove (toRemove) {}
	
	~InvokeCleanupHelper () {
		for (int i = 0; i < types->length (); i++) {
			if (remove[i]) {
				QMetaType::destroy (types->at (i), values[i + 1]);
			}
			
		}
		
	}
	
};

QVariant Nuria::Callback::invokeInternal (int count, void **args, int *types) const {
	
	// Argument array, works like the one from qt_metacall().
	void *rawArgs[this->d->args.length () + 1];
	bool removeMe[this->d->args.length ()];
	
	// Will destroy all elements in rawArgs which are marked by 'removeMe'.
	InvokeCleanupHelper cleaner (rawArgs, &this->d->args, removeMe);
	Q_UNUSED(cleaner);
	
	// Construct return value
	QVariant retVal;
	if (this->d->retType == QMetaType::QVariant) {
		rawArgs[0] = &retVal;
	} else if (this->d->retType != 0 && this->d->retType != QMetaType::Void) {
		retVal = QVariant (this->d->retType, (const void *)0);
		rawArgs[0] = retVal.data ();
	}
	
	// 
	int i = 0;
	
	// Prepend bound variables...
	bool usesPlaceholders = false;
	
	if (this->d->boundCount) {
		for (; i < this->d->boundCount && i < this->d->args.length (); i++) {
			void *curValue = this->d->boundValues[i];
			int curType = this->d->boundTypes[i];
			
			int type = this->d->args.at (i);
			
			// Placeholder?
			if (curType == qMetaTypeId< Nuria::Callback::Placeholder > ()) {
				usesPlaceholders = true;
				
				Placeholder *p = reinterpret_cast< Nuria::Callback::Placeholder * > (curValue);
				int pos = (int)(*p);
				
				// Out of boundaries?
				if (pos < 0 || pos >= count) {
					argumentHelper (rawArgs, removeMe, 0, 0, i, 1, type);
				} else {
					argumentHelper (rawArgs, removeMe, args[pos], types[pos], i, 1, type);
				}
				
			} else {
				
				// Bound value
				rawArgs[i + 1] = curValue;
				removeMe[i] = false;
				
			}
			
		}
		
	}
	
	// Construct arguments. Skip if placeholders are used.
	if (!usesPlaceholders) {
		for (int j = 0; i < this->d->args.length () && j < count; i++, j++) {
			int curType = this->d->args.at (i);
			
			argumentHelper (rawArgs, removeMe, args[j], types[j], i, 1, curType);
			
		}
		
	}
	
	// Not enough arguments?
	for (; i < this->d->args.length (); i++) {
		
		// Construct default instance
		argumentHelper (rawArgs, removeMe, 0, 0, i, 1, this->d->args.at (i));
		
	}
	
	// Is this a slot?
	if (this->d->type == Slot) {
		
		// Has the object been destroyed?
		if (this->d->ptr.slot->qobj.isNull ()) {
			return QVariant();
		}
		
		// Does the QObject live in another thread?
		// Use call-and-forget if we don't expect a result.
		bool voidRet = (this->d->retType == 0 || this->d->retType == QMetaType::Void);
		Qt::ConnectionType type = this->d->ptr.slot->type;
		if (type == Qt::AutoConnection) {
			type = (this->d->ptr.slot->qobj->thread () == QThread::currentThread ())
			       ? Qt::DirectConnection
			       : (voidRet ? Qt::QueuedConnection : Qt::BlockingQueuedConnection);
		}
		
		// Create array of generic arguments
		QGenericArgument gArgs[10];
		for (int j = 0; j < this->d->args.length (); j++) {
			const char *name = QMetaType::typeName (this->d->args.at (j));
			gArgs[j] = QGenericArgument (name, rawArgs[j + 1]);
		}
		
		// Return value argument
		QGenericReturnArgument retArg;
		if (this->d->retType == QMetaType::QVariant) {
			retArg = QGenericReturnArgument ("QVariant", &retVal);
		} else if (!voidRet) {
			retArg = QGenericReturnArgument (retVal.typeName (), retVal.data ());
		}
		
		// Invoke!
		if (!this->d->ptr.slot->method.invoke (this->d->ptr.slot->qobj, type, retArg,
		                                       gArgs[0], gArgs[1], gArgs[2], gArgs[3], gArgs[4],
		                                       gArgs[5], gArgs[6], gArgs[7], gArgs[8], gArgs[9])) {
			nError() << "Failed to invoke slot" << this->d->ptr.slot->name
			         << "on" << this->d->ptr.slot->qobj;
		}
		
		// Done.
		return retVal;
	}
	
	// Invoke!
	this->d->ptr.base->trampoline (rawArgs);
	
	// Done.
	return retVal;
	
}

bool Nuria::Callback::initBase (Nuria::Callback::TrampolineBase *base,
				 int retType, const QList< int > &args) {
	
	// 
	if (!this->d) {
		this->d = new CallbackPrivate;
		this->d->ref.ref ();
	} else {
		this->d->freeBoundValues ();
		this->d->clear ();
	}
	
	// Type
	this->d->type = base->type;
	this->d->ptr.base = base;
	
	this->d->retType = retType;
	this->d->args = args;
	return true;
}
