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

#include "nuria/sessionmanager.hpp"

#include <QCache>

class Nuria::SessionManagerPrivate {
public:
	QCache< QByteArray, Session > sessions;
};

Nuria::SessionManager::SessionManager (int maxSessions, QObject *parent)
	: Nuria::AbstractSessionManager (parent), d_ptr (new SessionManagerPrivate)
{
	this->d_ptr->sessions.setMaxCost (maxSessions);
}

Nuria::SessionManager::~SessionManager () {
	delete this->d_ptr;
}

int Nuria::SessionManager::maxSessions () {
	return this->d_ptr->sessions.maxCost ();
}

void Nuria::SessionManager::setMaxSessions (int maxSessions) {
	this->d_ptr->sessions.setMaxCost (maxSessions);
}

bool Nuria::SessionManager::exists (const QByteArray &id) const {
	return this->d_ptr->sessions.contains (id);
}

Nuria::Session Nuria::SessionManager::get (const QByteArray &id) {
	if (this->d_ptr->sessions.contains (id)) {
		return *(this->d_ptr->sessions.object (id));
	}
	
	Session session = createSession (id);
	this->d_ptr->sessions.insert (id, new Session (session));
	return session;
	
}

void Nuria::SessionManager::removeSession (const QByteArray &id) {
	this->d_ptr->sessions.remove (id);
}
