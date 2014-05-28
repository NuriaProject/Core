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

#ifndef NURIA_ABSTRACTSESSIONMANAGER_HPP
#define NURIA_ABSTRACTSESSIONMANAGER_HPP

#include "essentials.hpp"
#include "session.hpp"
#include <QObject>

namespace Nuria {

/** 
 * \brief Provides simple session managment capabilities.
 * 
 * The abstract session manager provides a common interface to session 
 * managment capabilities. A session is a key-value-store identified by an
 * arbitrary identifier, known as the session id.
 * The session id SHOULD contain only printable characters for 
 * compatibility with text-based transport mechanisms.
 * 
 * \sa Nuria::Session
 * \sa Nuria::SessionManager
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
	 * Returns true if the session \a id exists inside this manager.
	 */
	virtual bool exists (const QByteArray &id) = 0;
	
	/**
	 * Fetches the session \a id. If such a Session does not
	 * exist, a new one with that id will be created instead.
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
	
	/**
	 * Creates the actual Session object with a given \id
	 */
	Session createSession (const QByteArray& id);

public slots:
	/**
	 * Removes a Session from the manager.
	 */
	virtual void removeSession (const QByteArray &id) = 0;
	
};

} // namespace Nuria

#endif // NURIA_ABSTRACTSESSIONMANAGER_HPP
