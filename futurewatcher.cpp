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

#include "futurewatcher.hpp"

#include <QTimerEvent>

namespace Nuria {
class GenericFutureWatcherPrivate {
public:
	
	Future< QVariant > future;
	int timerId;
	
};
}

Nuria::GenericFutureWatcher::GenericFutureWatcher (const Nuria::Future< QVariant > &future, QObject *parent)
	: QObject (parent), d_ptr (new GenericFutureWatcherPrivate)
{
	
	// Register
	this->d_ptr->future = future;
	this->d_ptr->future.registerWatcher (this);
	this->d_ptr->timerId = -1;
	
}

Nuria::GenericFutureWatcher::~GenericFutureWatcher () {
	
	this->d_ptr->future.unregisterWatcher (this);
	delete this->d_ptr;
	
}

Nuria::Future< QVariant > Nuria::GenericFutureWatcher::future () const {
	return this->d_ptr->future;
}

void Nuria::GenericFutureWatcher::setFuture (const Nuria::Future< QVariant > &future) {
	
	this->d_ptr->future.unregisterWatcher (this);
	this->d_ptr->future = future;
	this->d_ptr->future.registerWatcher (this);
	
}

QVariant Nuria::GenericFutureWatcher::variant () const {
	return this->d_ptr->future.value ();
}

void Nuria::GenericFutureWatcher::notify () {
	emit finished (this->d_ptr->future);
	emit finished ();
}

void Nuria::GenericFutureWatcher::timerEvent (QTimerEvent *evt) {
	
	// We only care about our timer ..
	if (evt->timerId () != this->d_ptr->timerId) {
		QObject::timerEvent (evt);
		return;
	}
	
	// Accept.
	evt->accept ();
	
	// Stop timer and see if data has alread arrived. If so, trigger the signal.
	killTimer (this->d_ptr->timerId);
	
	if (this->d_ptr->future.isFinished ()) {
		notify ();
	}
	
}

void Nuria::GenericFutureWatcher::waitForResult () {
	
	// If a future waits for a result, we start a timer here.
	// Why? If the future value has been set in between the moment the
	// future has been checked and the the watcher instance has been
	// created, this watcher will never fire. Thus, we start a timer which
	// fires the next time the application gets into a event loop and check
	// the value again.
	
	this->d_ptr->timerId = startTimer (0);
	
}
