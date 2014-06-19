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

#ifndef NURIA_SESSIONMANAGER_HPP
#define NURIA_SESSIONMANAGER_HPP

#include "essentials.hpp"
#include "abstractsessionmanager.hpp"

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
class NURIA_CORE_EXPORT SessionManager : public Nuria::AbstractSessionManager {
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
	
	/**
	 * Sets the maximum number of sessions to \a maxSessions .
	 */
	void setMaxSessions (int maxSessions);
	
	bool exists (const QByteArray &id) override;
	Session get (const QByteArray &id) override;

public slots:
	virtual void removeSession (const QByteArray &id) override;	

private:	
	SessionManagerPrivate *d_ptr;
};

} // namespace Nuria

#endif // NURIA_SESSIONMANAGER_HPP
