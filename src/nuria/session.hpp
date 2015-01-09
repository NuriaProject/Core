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

#ifndef NURIA_SESSION_HPP
#define NURIA_SESSION_HPP

#include "essentials.hpp"

#include <QSharedData>
#include <QVariant>

namespace Nuria {

class AbstractSessionManager;
class SessionPrivate;
class Session;

NURIA_CORE_EXPORT bool operator== (const Session &a, const Session &b);
NURIA_CORE_EXPORT bool operator!= (const Session &a, const Session &b);

/**
 * \brief A data storage managed by an AbstractSessionManager
 * 
 * A Session is a key-value-store for persistent storage of session data.
 * Sessions are managed by an AbstractSessionManager and are object to its
 * storage policy.
 * 
 * \note This structure is \b explicitly-shared.
 * \note Sessions can be compared for (in)equality.
 */
class NURIA_CORE_EXPORT Session {
public:
	/** Creates an invalid session. */
	Session ();
	
	/** Copy constructor. */
	Session (const Session& other);
	
	/** Assignment operator. */
	Session &operator= (const Session &other);
	
	/** Destructor. */
	~Session ();
	
	/** Returns \c true if a session is valid. */
	bool isValid () const;
	
	/** Returns the id used to identify the session in the manager. */
	QByteArray id () const;
	
	/**
	 * Returns the manager which handles this session.
	 * \warning If the instance is invalid, \c nullptr may be returned.
	 */
	AbstractSessionManager *manager () const;
	
	/** Returns \c true if the stored data has changed. */
	bool isDirty () const;
	
	/**
	 * Marks the session as dirty, which indicates to the associated session
	 * manager that this instance should be written back to the manager
	 * implementation specific back-end, such as a database.
	 */
	void markDirty ();
	
	/**
	 * Marks the session as clean, used to indicate that changed data has
	 * been saved.
	 * 
	 * \warning This should only be used by a session manager.
	 */
	void markClean ();
	
	/** Removes the session from the manager. */
	void remove ();
	
	/**
	 * Returns the reference count of this session instance.
	 * The reference count indicates how many other session instances are
	 * in the application pointing to the same data, including this instance
	 * itself. If this method returns \c 1, then this is the only instance
	 * left.
	 * 
	 * \sa AbstractSessionManager
	 */
	int refCount () const;
	
	/**
	 * Returns the value known as \a key.
	 * If there's no value for \a key, then a invalid QVariant is returned
	 * which is not inserted into the internal storage.
	 * The dirty flag is not changed by this method.
	 */
	QVariant value (const QString &key) const;
	
	/** Returns \c true if there's a value for \a key. */
	bool contains (const QString &key) const;
	
	/**
	 * Returns the value known as \a key. If there's no value for \a key
	 * yet, a invalid QVariant will be inserted and the reference to it
	 * returned.
	 * 
	 * The session will be marked dirty.
	 * \sa markDirty value
	 */
	QVariant &operator[] (const QString &key);
	
	/** Same as value(). */
	QVariant operator[] (const QString &key) const;
	

private:
	friend class AbstractSessionManager;
	friend bool operator== (const Session &a, const Session &b);
	friend bool operator!= (const Session &a, const Session &b);
	
	/**
	 * Constructs a session identified by \a id and owned by \a manager. 
	 */
	Session (const QByteArray &id, AbstractSessionManager *manager);
	
	QExplicitlySharedDataPointer< SessionPrivate > d;
};

} // namespace Nuria

Q_DECLARE_METATYPE(Nuria::Session)

#endif // NURIA_SESSION_HPP
