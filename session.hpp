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

#ifndef NURIA_SESSION_HPP
#define NURIA_SESSION_HPP

#include "essentials.hpp"

#include <QSharedData>
#include <QVariant>

namespace Nuria {

class AbstractSessionManager;

class SessionPrivate;

/**
 * \brief A managed key-value-store named by \a id.
 * 
 * A Session is a key-value-store for persistent storage of session data.
 * Sessions are managed by an AbstractSessionManager.
 */
class NURIA_CORE_EXPORT Session {
public:
	/**
	 * Creates an invalid session.
	 */
	Session ();
	
	Session (const Session& other);
	~Session ();
	
	/**
	 * Returns \a true if a session is valid. Sessions obtained through
	 * a session manager are always valid.
	 */
	bool isValid () const;
	
	/**
	 * Returns the id used to identify the session in the manager.
	 */
	QByteArray id () const;
	
	/**
	 * Returns the manager which handles this session.
	 */
	AbstractSessionManager *manager () const;
	
	/**
	 * Returns \c true if the stored data has changed.
	 */
	bool isDirty () const;
	
	/**
	 * Marks the session as dirty. Used to indicate changed data.
	 */
	void markDirty ();
	
	/**
	 * Marks the session as clean, used to indicate that changed data has
	 * been saved. Should only be used by a session manager.
	 */
	void markClean ();
	
	/**
	 * Removes the session from the manager.
	 */
	void remove ();
		
	/**
	 * Fetches the value stored under key.
	 */
	QVariant value (const QString& key) const;
	
	/**
	 * Returns a writable reference to the stored value under key.
	 */
	QVariant &operator[] (const QString& key);
	
	/**
	 * Fetches the value stored under key
	 */
	QVariant operator[] (const QString& key) const;
	
	Session &operator= (const Session &other);
	
	/**
	 * Checks if the other session object refers to the same session.
	 */
	bool operator== (const Session &other) const { return (this->d == other.d); }
	
	/**
	 * Checks if the other session object refers to another session
	 */
	bool operator!= (const Session &other) const { return (this->d != other.d); }

private:
	friend class AbstractSessionManager;
	
	/**
	 * Creates a Session identified by id and owned by manager. 
	 */
	Session (const QByteArray &id, AbstractSessionManager *manager);
	
	QExplicitlySharedDataPointer< SessionPrivate > d;
};

} // namespace Nuria

#endif // NURIA_SESSION_HPP
