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
