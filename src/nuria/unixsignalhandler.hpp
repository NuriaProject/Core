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

#ifndef UNIXSIGNALHANDLER_HPP
#define UNIXSIGNALHANDLER_HPP

#include "callback.hpp"
#include <QObject>

namespace Nuria {

class UnixSignalHandlerPrivate;

/**
 * \brief Utility class to react to UNIX signals in a Qt fashion
 * 
 * \par Usage
 * 
 * To install your own handler, all you have to do is to tell the class that
 * it should install a signal handler itself using listenToUnixSignal(). After
 * that, you can connect to the various Qt signals which are emitted when a
 * UNIX signal was triggered.
 * 
 * To uninstall a UNIX handler use ignoreUnixSignal().
 * 
 * \note Connected signals and callbacks don't have to be signal-safe as they're
 * run outside the signal itself.
 * 
 * \par Restrictions
 * 
 * Please note that the UNIX signal system is quite fragile. Other code may
 * replace the installed signal handlers \b without warning.
 * 
 * When unregistering, the class tries to restore the old handler. If the old
 * handler is invalid at this point, the result will be undefined behaviour
 * likely crashing the process.
 * 
 * If there is another UNIX signal handler at time of registration, the old
 * one will \b not be called, which can potentionally break other code. Please
 * note that Qt installs a handler on \c SIGCHLD for QProcess - Thus you should
 * not install your own handler on that UNIX signal.
 * 
 * See also sigaction(3) for in-depth details.
 */
class NURIA_CORE_EXPORT UnixSignalHandler : public QObject {
	Q_OBJECT
public:
	
	/** Returns the global instance. */
	static UnixSignalHandler *get ();
	
	/** Destructor. */
	~UnixSignalHandler () override;
	
	/**
	 * The class will attempt to register as signal handler for \a signalId.
	 * If registration fails, \c false is returned. When already listening
	 * to \a signalId, \c true is returned.
	 */
	bool listenToUnixSignal (int signalId);
	
	/**
	 * Will unregister the handler of \a signalId. Returns \c true on
	 * success. Returns \c false when \a signalId is not listened to
	 * or if deregistration failed.
	 */
	bool ignoreUnixSignal (int signalId);
	
	/**
	 * Returns \c true if \a signalId is currently listened to, that means,
	 * if the class has a registered handler for it.
	 */
	bool isListeningTo (int signalId);
	
	/**
	 * Instructs the class to invoke \a callback when \a unixSignal has
	 * been caught. \a unixSignal may be \c AllSignals. \a callback will
	 * receive the signal identifier of type \c int as only argument.
	 * 
	 * \note The method will try to register a handler for \a unixSignal
	 * if it's not already listened to by the class.
	 */
	void invokeOnSignal (int signalId, const Callback &callback);
	
signals:
	
	/**
	 * Emitted when any signal has been raised.
	 * \sa listenToUnixSignal
	 */
	void signalRaised (int signalId);
	
	/** Emitted on \c SIGTERM. */
	void sigTerm ();
	
	/** Emitted on \c SIGINT. */
	void sigInterrupt ();
	
	/** Emitted on \c SIGUSR1. */
	void sigUser1 ();
	
	/** Emitted on \c SIGUSR2. */
	void sigUser2 ();
	
private slots:
	
	void signalDelegate ();
	
private:
	explicit UnixSignalHandler (QObject *parent = 0);
	
	bool createInternalPipe ();
	void printError (const char *message);
	bool tryReadUnixSignal ();
	void invokeSignalHandlers (int signalId);
	
	UnixSignalHandlerPrivate *d_ptr;
	
};

}

#endif // UNIXSIGNALHANDLER_HPP
