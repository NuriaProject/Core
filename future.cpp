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

#include "future.hpp"

#include <QCoreApplication>
#include <QWaitCondition>
#include <QEventLoop>
#include <debug.hpp>
#include <QTimer>
#include <QMutex>

#include "futurewatcher.hpp"

namespace Nuria {
class FuturePrivate {
public:
	
	FuturePrivate () : ref (1) {}
	
	QMutex mutex;
	QAtomicInt ref;
	QVariant value;
	int type;
	QList< GenericFutureWatcher * > watcher;
	
};

}

Nuria::FutureBase::FutureBase (int type)
	: d (new FuturePrivate)
{
	
	d->type = type;
	
}

Nuria::FutureBase::FutureBase (const Nuria::FutureBase &other)
	: d (other.d)
{
	
	this->d->ref.ref ();
	
}

Nuria::FutureBase::FutureBase (Nuria::FuturePrivate *dptr)
	: d (dptr)
{
	
	this->d->ref.ref ();
	
}

Nuria::FutureBase::~FutureBase () {
	
	if (!this->d->ref.deref ()) {
		delete this->d;
	}
	
}

Nuria::FutureBase &Nuria::FutureBase::operator= (const Nuria::FutureBase &other) {
	
	// Ref other first, so "a = a" won't corrupt the heap.
	other.d->ref.ref ();
	if (!this->d->ref.deref ()) {
		delete this->d;
	}
	
	this->d = other.d;
	return *this;
}

bool Nuria::FutureBase::operator== (const FutureBase &other) const {
	return (this->d == other.d);
}

bool Nuria::FutureBase::isFinished () const {
	
	this->d->mutex.lock ();
	bool finished = this->d->value.isValid ();
	this->d->mutex.unlock ();
	
	return finished;
}

int Nuria::FutureBase::type () const {
	return this->d->type;
}

void Nuria::FutureBase::waitForFinished () const {
	
	this->d->mutex.lock ();
	
	// Already finished?
	if (this->d->value.isValid ()) {
		this->d->mutex.unlock ();
		return;
	}
	
	// Nope.
	this->d->mutex.unlock ();
	
	// Looks like we have to wait ...
	QEventLoop loop;
	
	// Construct a temporary watcher. Connect finished() to the quit() slot
	// of the temporary loop. This way we will be woken up when the task
	// has finished.
	Future< QVariant > f (this->d);
	Nuria::GenericFutureWatcher watcher (f);
	
	QObject::connect (&watcher, SIGNAL(finished()), &loop, SLOT(quit()));
	
	// Notify the watcher
	watcher.waitForResult ();
	
	// Process events. Will return when the future finished.
	loop.exec ();
	
	// Value arrived!
	
}

const QVariant &Nuria::FutureBase::variant () const {
	this->d->mutex.lock ();
	
	// Finished?
	if (this->d->value.isValid ()) {
		this->d->mutex.unlock ();
		return this->d->value;
	}
	
	// 
	this->d->mutex.unlock ();
	
	// Wait
	waitForFinished ();
	
	// 
	return this->d->value;
}

void Nuria::FutureBase::setVariant (const QVariant &v) {
	
	// Store value
	this->d->mutex.lock ();
	this->d->value = v;
	this->d->mutex.unlock ();
	
	// Notify watchers. Direct access is safe, as the list
	// won't be changed anymore outside this function.
	for (int i = 0; i < this->d->watcher.length (); i++) {
		this->d->watcher.at (i)->notify ();
	}
	
	// Safe memory..
	this->d->watcher.clear ();
	
}

void Nuria::FutureBase::registerWatcher (Nuria::GenericFutureWatcher *watcher) {
	this->d->mutex.lock ();
	bool notify = true;
	
	if (!this->d->value.isValid ()) {
		notify = false;
		this->d->watcher.append (watcher);
	}
	
	this->d->mutex.unlock ();
	
	// Notify?
	if (notify) {
		watcher->notify ();
	}
	
}

void Nuria::FutureBase::unregisterWatcher (Nuria::GenericFutureWatcher *watcher) {
	this->d->mutex.lock ();
	this->d->watcher.removeOne (watcher);
	this->d->mutex.unlock ();
}
