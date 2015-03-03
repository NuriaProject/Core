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

#include "nuria/resource.hpp"

#include <QCoreApplication>
#include <QMetaEnum>
#include <QThread>
#include <QMutex>

#include "private/invocationwaitobject.hpp"

namespace Nuria {
class ResourcePropertyPrivate : public QSharedData {
public:
	
	ResourcePropertyPrivate (Resource::Property::Type t, const QString &n,
	                         const Resource::ArgumentTypeMap &args, int result)
	        : type (t), name (n), arguments (args), resultType (result)
	{
		
	}
	
	// 
	Resource::Property::Type type;
	QString name;
	Resource::ArgumentTypeMap arguments;
	int resultType;
	
};

}

Nuria::Resource::Resource (QObject *parent)
        : QObject (parent)
{
	
	qRegisterMetaType< Property > ();
	qRegisterMetaType< PropertyList > ();
	qRegisterMetaType< ResourcePointer > ();
	qRegisterMetaType< InvokeResultState > ();
	qRegisterMetaType< InvokeCallback > ();
	
}

Nuria::Resource::~Resource () {
	// 
}

bool Nuria::Resource::isSerializable () const {
	return false;
}

QByteArray Nuria::Resource::serialize () {
	return QByteArray ();
}

bool Nuria::Resource::deserialize (const QByteArray &data) {
	Q_UNUSED(data);
	return false;
}

Nuria::Invocation Nuria::Resource::invoke (const QString &slot, const QVariantMap &arguments,
                                           InvokeCallback callback, int timeout) {
	try {
		return invokeImpl (slot, arguments, callback, timeout);
	} catch (const ResourceHandlerException &e) {
		callback (e.m_state, e.m_result);
		return Invocation ();
	}
	
}

Nuria::Invocation Nuria::Resource::listen (const QString &signal, InvokeCallback callback) {
	try {
		return listenImpl (signal, callback);
	} catch (const ResourceHandlerException &e) {
		callback (e.m_state, e.m_result);
		return Invocation ();
	}
	
}

Nuria::Invocation Nuria::Resource::invokeImpl (const QString &slot, const QVariantMap &arguments,
                                               InvokeCallback callback, int timeout) {
	Q_UNUSED(arguments);
	
	if (slot.isEmpty ()) {
		return properties (callback, timeout);
	}
	
	// Unknown slot
	callback (UnknownError, slot);
	return Invocation ();
}

Nuria::Invocation Nuria::Resource::listenImpl (const QString &signal, InvokeCallback callback) {
	callback (UnknownError, signal);
	return Invocation ();
}

QString Nuria::Resource::resultStateName (InvokeResultState state) {
	static QMetaEnum theEnum =
	                staticMetaObject.enumerator (staticMetaObject.indexOfEnumerator ("InvokeResultState"));
	
	const char *name = theEnum.valueToKey (state);
	if (name) {
		return QLatin1String (name);
	}
	
	// 
	return QStringLiteral("<Unknown:%1>").arg (int (state));
}

Nuria::Resource::Property::Property ()
        : d (nullptr)
{
	
}

Nuria::Resource::Property::Property(const Nuria::Resource::Property &other)
        : d (other.d)
{
	
}

Nuria::Resource::Property &Nuria::Resource::Property::operator= (const Property &other) {
	this->d = other.d;
	return *this;
}

Nuria::Resource::Property::Property (Type type, const QString &name, const ArgumentTypeMap &arguments, int resultType)
        : d (new ResourcePropertyPrivate (type, name, arguments, resultType))
{
	
}

Nuria::Resource::Property::~Property () {
	// 
}

bool Nuria::Resource::Property::operator== (const Property &other) const {
	if (this->d == other.d) { // Same d pointer (also: Both are NULL)
		return true;
	}
	
	if (!this->d || !other.d) { // One d-ptr is NULL
		return false;
	}
	
	// Both have valid d-pointers, check them.
	return (this->d->type == other.d->type && this->d->name == other.d->name &&
	        this->d->resultType == other.d->resultType &&
	        this->d->arguments == other.d->arguments);
	
}

bool Nuria::Resource::Property::isValid () const {
	return (this->d && this->d->type != Invalid);
}

Nuria::Resource::Property::Type Nuria::Resource::Property::type () const {
	return (this->d ? this->d->type : Invalid);
}

QString Nuria::Resource::Property::name () const {
	return (this->d ? this->d->name : QString ());
}

Nuria::Resource::ArgumentTypeMap Nuria::Resource::Property::arguments () const {
	return (this->d ? this->d->arguments : ArgumentTypeMap ());
}

int Nuria::Resource::Property::resultType() const {
	return (this->d ? this->d->resultType : QMetaType::UnknownType);
}

struct Nuria::InvocationResultBase::Data : public QSharedData {
	Nuria::Internal::InvocationWaitObject waiter;
	Nuria::Invocation invocation;
	QVariant result;
	QMutex mutex;
	volatile int error = -1;
};

Nuria::InvocationResultBase::InvocationResultBase ()
        : d (new Data)
{
	// Unlock in callback()
	this->d->mutex.lock ();
}

Nuria::InvocationResultBase::~InvocationResultBase () {
	// 
}

void Nuria::InvocationResultBase::init (const Nuria::Invocation &invocation) {
	this->d->invocation = invocation;
}

Nuria::InvocationResultBase::InvocationResultBase (const InvocationResultBase &other)
        : d (other.d)
{
	
}

Nuria::InvocationResultBase &Nuria::InvocationResultBase::operator= (const InvocationResultBase &other) {
	this->d = other.d;
	return *this;
}

QVariant Nuria::InvocationResultBase::result () const {
	return this->d->result;
}

bool Nuria::InvocationResultBase::hasFinished () const {
	return this->d->error >= 0;
}

bool Nuria::InvocationResultBase::hasError () const {
	return this->d->error > Resource::Success;
}

Nuria::Resource::InvokeResultState Nuria::InvocationResultBase::resultState () const {
	return Resource::InvokeResultState (this->d->error);
}

Nuria::Invocation Nuria::InvocationResultBase::invocation () const {
	return this->d->invocation;
}

Nuria::Resource::InvokeResultState Nuria::InvocationResultBase::waitForFinished (WaitMode waitMode) {
	wait (waitMode);
	return resultState ();
}

void Nuria::InvocationResultBase::callback (Resource::InvokeResultState resultState, const QVariant &result) {
	this->d->error = int (resultState);
	this->d->result = result;
	
	this->d->mutex.unlock (); // Locked in constructor
	emit this->d->waiter.wakeup (); // Unlock asynchronous processes in this thread
	
}

void Nuria::InvocationResultBase::wait (WaitMode waitMode) {
	if (this->d->error >= 0) { return; }
	
	// Resource sanity check
	Resource *resource = this->d->invocation.resource ();
	if (!resource) { return; }
	
	// Decide on the strategy to be used.
	if (waitMode == Automatic) {
		// If the resource lives in the current thread, use a event loop
		waitMode = (resource->thread () == QThread::currentThread ()) ? EventLoop : Blocking;
	}
	
	// Wait ...
	if (waitMode == EventLoop) {
		QEventLoop loop;
		loop.connect (&this->d->waiter, &Internal::InvocationWaitObject::wakeup,
		              &loop, &QEventLoop::quit);
		loop.exec ();
		
	} else {
		// Resource lives in a different thread. Lock the mutex and wait
		// until we have it.
		this->d->mutex.lock ();
		this->d->mutex.unlock ();
	}
	
}

Nuria::ResourceHandlerException::ResourceHandlerException (Resource::InvokeResultState state,
                                                           const QVariant &result) noexcept
        : m_state (state), m_result (result)
{
	
}

Nuria::ResourceHandlerException::~ResourceHandlerException () {
	
}

const char *Nuria::ResourceHandlerException::what () const noexcept {
	return "";
}
