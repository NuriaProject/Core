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

#include "unixsignalhandler.hpp"

#include <QCoreApplication>
#include <QSocketNotifier>
#include <nuria/debug.hpp>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <memory>

// Global for easy access from signal handler
static int g_pipe[2];

namespace Nuria {

class UnixSignalHandlerPrivate {
public:
	QSocketNotifier *notifier = nullptr;
	
	QMap< int, struct sigaction * > actionHandlers;
	QMultiMap< int, Callback > signalCallbacks;
	
};

}

Nuria::UnixSignalHandler *Nuria::UnixSignalHandler::get () {
	static UnixSignalHandler *instance = nullptr;
	
	if (!instance) {
		instance = new UnixSignalHandler (qApp);
	}
	
	return instance;
}

Nuria::UnixSignalHandler::UnixSignalHandler (QObject *parent)
	: QObject (parent), d_ptr (new UnixSignalHandlerPrivate)
{
	createInternalPipe ();
	
	this->d_ptr->notifier = new QSocketNotifier (g_pipe[0], QSocketNotifier::Read, this);
	connect (this->d_ptr->notifier, SIGNAL(activated(int)), SLOT(signalDelegate()));
	
}

void Nuria::UnixSignalHandler::printError (const char *message) {
	int err = errno;
	char buf[64];
	
	strerror_r (err, buf, sizeof buf);
	nError() << message << err << "=>" << buf;	
}

bool Nuria::UnixSignalHandler::tryReadUnixSignal () {
	int signalId = 0;
	
	if (::read (g_pipe[0], &signalId, sizeof signalId) != sizeof signalId) {
		if (errno != EWOULDBLOCK) {
			printError ("Failed to read from internal pipe:");
		}
		
		return false;
	}
	
	// 
	invokeSignalHandlers (signalId);
	return true;
}

void Nuria::UnixSignalHandler::invokeSignalHandlers (int signalId) {
	
	auto it = this->d_ptr->signalCallbacks.lowerBound (signalId);
	auto end = this->d_ptr->signalCallbacks.upperBound (signalId);
	for (; it != end; ++it) {
		(*it)(signalId); // Invoke
	}
	
	// 
	emit signalRaised (signalId);
	switch (signalId) {
	case SIGTERM: emit sigTerm (); break;
	case SIGINT:  emit sigInterrupt (); break;
	case SIGUSR1: emit sigUser1 (); break;
	case SIGUSR2: emit sigUser2 (); break;
	}
	
}

bool Nuria::UnixSignalHandler::createInternalPipe () {
	if (::pipe2 (g_pipe, O_CLOEXEC | O_NONBLOCK) == 0) {
		return true;
	}
	
	// 
	printError ("Failed to create internal pipe:");
	return false;
	
}

Nuria::UnixSignalHandler::~UnixSignalHandler () {
	delete this->d_ptr;
}

static void signalHandler (int signo, siginfo_t * /* info */, void * /* context */) {
	::write (g_pipe[1], &signo, sizeof signo);
}

bool Nuria::UnixSignalHandler::listenToUnixSignal (int signalId) {
	if (this->d_ptr->actionHandlers.contains (signalId)) {
		return true;
	}
	
	// 
	struct sigaction action;
	struct sigaction *originalAction = new struct sigaction;
	
	::sigfillset (&action.sa_mask);
	action.sa_sigaction = &signalHandler;
	action.sa_flags = SA_RESTART;
	
	if (::sigaction (signalId, &action, originalAction) != 0) {
		printError (qPrintable (QString ("Failed to install handler for signal %1").arg (signalId)));
		return false;
	}
	
	this->d_ptr->actionHandlers.insert (signalId, originalAction);
	return true;
	
}

bool Nuria::UnixSignalHandler::ignoreUnixSignal (int signalId) {
	if (!this->d_ptr->actionHandlers.contains (signalId)) {
		return false;
	}
	
	// 
	std::unique_ptr< struct sigaction > oldAction (this->d_ptr->actionHandlers.take (signalId));
	if (::sigaction (signalId, oldAction.get (), nullptr) != 0) {
		printError (qPrintable (QString ("Failed to restore handler for signal %1").arg (signalId)));
		return false;
	}
	
	return true;
}

bool Nuria::UnixSignalHandler::isListeningTo (int signalId) {
	return this->d_ptr->actionHandlers.contains (signalId);
}

void Nuria::UnixSignalHandler::invokeOnSignal (int signalId, const Nuria::Callback &callback) {
	if (listenToUnixSignal (signalId)) {
		this->d_ptr->signalCallbacks.insert (signalId, callback);
	}
	
}

void Nuria::UnixSignalHandler::signalDelegate () {
	
	// There may be multiple UNIX signal emissions in the pipe.
	while (tryReadUnixSignal ());
	
}
