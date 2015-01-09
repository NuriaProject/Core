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

#ifndef NURIA_THREADLOCAL_HPP
#define NURIA_THREADLOCAL_HPP

#include <QThreadStorage>

/**
 * This macro creates a static global variable which is thread local.
 * Calling \a Name will return the thread-local instance of the variable.
 * If there is no instance yet one will be created. It will be destroyed
 * when the thread the instance lives in is being destroyed.
 * If you need to pass additional arguments to the constructor use
 * NURIA_THREAD_GLOBAL_STATIC_WITH_ARGS instead. If that isn't enough
 * consider using NURIA_THREAD_GLOBAL_STATIC_WITH_INIT instead.
 * 
 * \sa NURIA_THREAD_GLOBAL_STATIC_WITH_ARGS NURIA_THREAD_GLOBAL_STATIC_WITH_INIT
 * 
 * \par Example
 * \code
 * // On the global scope
 * NURIA_THREAD_GLOBAL_STATIC(QStringList, users)
 * // ... in a function ...
 * nDebug() << "First user:" << users ()->first ();
 * \endcode
 */
#define NURIA_THREAD_GLOBAL_STATIC(Type, Name)                    \
	static Type *Name () {                                    \
		static QThreadStorage< Type * > storage;          \
		if (!storage.hasLocalData ()) {                   \
			storage.setLocalData (new Type);          \
		}                                                 \
		return storage.localData ();                      \
	}

/**
 * Same as NURIA_THREAD_GLOBAL_STATIC with the difference that this
 * macro expects the arguments which will be passed to the constructor
 * of the thread-local variable if it needs to be created.
 * \note Make sure to enclose the arguments in brackets.
 * \code
 * // On the global scope
 * NURIA_THREAD_GLOBAL_STATIC_WITH_ARGS(QString, instanceName, ("Default value")));
 * // ... in a function ...
 * nDebug() << "Instance name:" << instanceName();
 * \endcode
 * \sa NURIA_THREAD_GLOBAL_STATIC
 */
#define NURIA_THREAD_GLOBAL_STATIC_WITH_ARGS(Type, Name, Args)    \
	static Type *Name () {                                    \
		static QThreadStorage< Type * > storage;          \
		if (!storage.hasLocalData ()) {                   \
			storage.setLocalData (new Type Args);     \
		}                                                 \
		return storage.localData ();                      \
	}

/**
 * Same as NURIA_THREAD_GLOBAL_STATIC with the difference that this
 * macro takes a function which can be used to init the thread-local
 * instance before it's used for the first time.
 * \note The init function must return a pointer to the thread-local data.
 * \code
 * // On the global scope
 * NURIA_THREAD_GLOBAL_STATIC_WITH_INIT(QString, instanceName, initFunc);
 * // ... later ...
 * QString *initFunc () {
 * 	return new QString ("Hello");
 * }
 * // ... in a function ...
 * nDebug() << "Instance name:" << instanceName();
 * \endcode
 * \sa NURIA_THREAD_GLOBAL_STATIC
 */
#define NURIA_THREAD_GLOBAL_STATIC_WITH_INIT(Type, Name, Method)  \
	static Type *Name () {                                    \
		static QThreadStorage< Type * > storage;          \
		if (!storage.hasLocalData ()) {                   \
			storage.setLocalData (Method ());         \
		}                                                 \
		return storage.localData ();                      \
	}

#endif // NURIA_THREADLOCAL_HPP
