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

#include "nuria/abstractsessionmanager.hpp"

#include <QUuid>

#include "nuria/session.hpp"

Nuria::AbstractSessionManager::AbstractSessionManager (QObject *parent) 
	: QObject (parent)
{
}

QByteArray Nuria::AbstractSessionManager::generateNewId () {
	QByteArray id = QUuid::createUuid ().toByteArray ();
	id = id.mid (1, id.length () - 2);
	return id;
}

Nuria::Session Nuria::AbstractSessionManager::createSession (const QByteArray &id) {
	return Session(id, this);
}

Nuria::Session Nuria::AbstractSessionManager::create () {
	return get (generateNewId ());
}
