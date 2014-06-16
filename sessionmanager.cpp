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

#include "sessionmanager.hpp"

#include <QCache>

class Nuria::SessionManagerPrivate {
public:
	QCache< QByteArray, Session > sessions;
};

Nuria::SessionManager::SessionManager (int maxSessions, QObject *parent)
	: Nuria::AbstractSessionManager (parent), d_ptr (new SessionManagerPrivate)
{
	d_ptr->sessions.setMaxCost (maxSessions);
}

int Nuria::SessionManager::maxSessions ()
{
	return d_ptr->sessions.maxCost ();
}

void Nuria::SessionManager::setMaxSessions (int maxSessions)
{
	d_ptr->sessions.setMaxCost (maxSessions);
}

bool Nuria::SessionManager::exists(const QByteArray &id)
{
	return d_ptr->sessions.contains (id);
}

Nuria::Session Nuria::SessionManager::get (const QByteArray &id) {
	if (d_ptr->sessions.contains (id)) {
		return *d_ptr->sessions.object (id);
	} else {
		Session *session = new Session (createSession (id));
		d_ptr->sessions.insert (id, session);
		return *session;
	}
}

void Nuria::SessionManager::removeSession (const QByteArray &id) {
	d_ptr->sessions.remove (id);
}