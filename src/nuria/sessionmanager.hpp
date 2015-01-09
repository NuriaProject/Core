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

#ifndef NURIA_SESSIONMANAGER_HPP
#define NURIA_SESSIONMANAGER_HPP

#include "abstractsessionmanager.hpp"
#include "essentials.hpp"

namespace Nuria {

class SessionManagerPrivate;

/**
 * \brief Provides an AbstractSessionManager with memory-based storage.
 * 
 * SessionManager is a memory-storage reference implementation of the
 * AbstractSessionManager interface. Note that SessionManager limits the
 * number of sessions at a given time, any additional session will lead 
 * to the deletion of the least recently used session. The default limit
 * is set at \c 1000 sessions.
 * 
 * \sa AbstractSessionManager
 */
class NURIA_CORE_EXPORT SessionManager : public AbstractSessionManager {
	Q_OBJECT
public:
	/**
	 * Creates a SessionManager for storing \a maxSessions sessions at 
	 * once.
	 */
	explicit SessionManager (int maxSessions = 1000, QObject *parent = 0);
	~SessionManager ();
	
	
	/**
	 * Returns the maximum number of sessions.
	 */
	int maxSessions ();
	
	/** Sets the maximum number of sessions to \a maxSessions. */
	void setMaxSessions (int maxSessions);
	
	bool exists (const QByteArray &id) const override;
	Session get (const QByteArray &id) override;

public slots:
	virtual void removeSession (const QByteArray &id) override;	

private:	
	SessionManagerPrivate *d_ptr;
};

}

#endif // NURIA_SESSIONMANAGER_HPP
