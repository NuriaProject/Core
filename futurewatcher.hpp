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

#ifndef NURIA_FUTUREWATCHER_HPP
#define NURIA_FUTUREWATCHER_HPP

#include "essentials.hpp"
#include "future.hpp"
#include <QObject>

namespace Nuria {

class GenericFutureWatcherPrivate;

/**
 * \brief The GenericFutureWatcher class can be used to get notified when a Future
 * instances gets finished.
 * 
 * This version can consume a Future with any T. If you want a more specific
 * version use FutureWatcher<>.
 * 
 * \note Registering a watcher on a already finished future will immediately
 * emit the finished() signals.
 */
class NURIA_CORE_EXPORT GenericFutureWatcher : public QObject {
	Q_OBJECT
public:
	explicit GenericFutureWatcher (const Future< QVariant > &future, QObject *parent = 0);
	~GenericFutureWatcher ();
	
	/**
	 * Returns the watched future.
	 */
	Future< QVariant > future () const;
	
	/**
	 * Sets the to be watched future.
	 */
	void setFuture (const Future< QVariant > &future);
	
	/**
	 * Same as Future::value()
	 */
	QVariant variant () const;
	
signals:
	
	void finished ();
	void finished (Future< QVariant > future);
	
protected:
	void timerEvent (QTimerEvent *evt);
	
private:
	friend class FutureBase;
	
	// 
	void waitForResult ();
	
	// Used by FutureBase to notify that the task is finished.
	void notify ();
	
	// 
	GenericFutureWatcherPrivate *d_ptr;
	
};

}

#endif // NURIA_FUTUREWATCHER_HPP
