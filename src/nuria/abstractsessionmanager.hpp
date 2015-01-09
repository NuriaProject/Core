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

#ifndef NURIA_ABSTRACTSESSIONMANAGER_HPP
#define NURIA_ABSTRACTSESSIONMANAGER_HPP

#include "essentials.hpp"
#include "session.hpp"
#include <QObject>

namespace Nuria {

/** 
 * \brief Interface for a session manager
 * 
 * The abstract session manager provides a common interface for session 
 * managment. Sessions are uniquely named by their id, which is assigned by a
 * session manager. The id itself \b should only contain printable ASCII symbols
 * like characters or digits. Please note that session manager implementation
 * may choose to obey or not obey this rule.
 * 
 * See SessionManager for a general purpose in-memory session manager.
 * 
 * \sa Session
 * \sa SessionManager
 * 
 * \par Management of sessions
 * It's important to know that sessions are not stored into the session managers
 * back-end (Which is defined by implementing AbstractSessionManager)
 * immediately. Instead, sessions use a dirty flag. Implementations should keep
 * a list of all currently used sessions and iterate over it at some point,
 * checking the dirty flags of all sessions and eventually writing changed ones
 * into some kind of storage engine.
 * 
 * As this list itself holds a reference to the session, it'd be never cleard
 * from memory. To counter this, you can use Session::refCount() to see if the
 * session held inside the manager itself is the last reference that's left,
 * enabling the manager to clear sessions which are no longer needed.
 * 
 */
class NURIA_CORE_EXPORT AbstractSessionManager : public QObject {
	Q_OBJECT
public:
	explicit AbstractSessionManager (QObject *parent = nullptr);
	
	/**
	 * Creates a new session with an unique id.
	 * 
	 * The default implementation does so by calling get() using a new id
	 * from generateNewId().
	 */
	virtual Session create ();
	
	/**
	 * Returns \c true if the session \a id is known by the manager.
	 */
	virtual bool exists (const QByteArray &id) const = 0;
	
	/**
	 * Fetches the session \a id. If no session \a id is known, a new one
	 * with that id will be created instead.
	 * 
	 * \warning It is possible that an invalid session will be generated,
	 * e.g. when the SessionManager is for some reason unable to fetch
	 * or create a real session. In this case the session will not be
	 * saved or fetched later on, but will still function for data 
	 * storage.
	 */
	virtual Session get (const QByteArray &id) = 0;

protected:	
	/** 
	 * Generate a new unique session id.
	 * 
	 * The default implementation uses UUIDs and thus should be 
	 * globally unique.
	 * 
	 * \note This is only guaranteed to be unique to the specific manager.
	 */
	virtual QByteArray generateNewId ();
	
	/** Creates the actual session object with a given \a id. */
	Session createSession (const QByteArray& id);

public slots:
	/** Removes the session \a id from the manager. */
	virtual void removeSession (const QByteArray &id) = 0;
	
};

} // namespace Nuria

#endif // NURIA_ABSTRACTSESSIONMANAGER_HPP
