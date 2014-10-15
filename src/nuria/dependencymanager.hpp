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

#ifndef NURIA_DEPEDENCYMANAGER_HPP
#define NURIA_DEPEDENCYMANAGER_HPP

#include "essentials.hpp"
#include <functional>
#include <QObject>

namespace Nuria {
class DependencyManagerPrivate;

/**
 * \brief Dependency injection manager.
 * 
 * Dependency injection is interesting whenever a class has dependencies
 * to other utility classes. Those classes usually only have a single
 * application-wide instance, thus those are often implemented as singletons.
 * While using singletons is mostly easy to do, it has also its flaws:
 * 1. It requires additional methods to be implemented. Granted this is easy,
 *    but it's also really repetitive.
 * 2. It's really hard to test classes which rely on singletons.
 * 
 * \par Usage
 * 
 * First, you'll need to register all classes which you want to use as
 * dependencies to the Qt meta system:
 * 
 * \code
 * Q_DECLARE_METATYPE(MyType*)
 * \endcode
 * 
 * This line should come right after the class definition. Please note that
 * the macro itself \b must be invoked on the global scope.
 * 
 * \note 'MyType' must inherit QObject.
 * 
 * After this, you can start using it right away with the NURIA_DEPENDENCY
 * macro:
 * 
 * \code
 * MyType *myType = NURIA_DEPENDENCY(MyType)
 * \endcode
 * 
 * \par Requirements for dependency classes
 * If DependencyManager should create instances on-demand, a constructor which
 * takes zero (Or only optional ones) arguments must be annotated using
 * \a Q_INVOKABLE. 
 * 
 * \par Using creators
 * 
 * If the name of the object doesn't match the typename, or if you need to
 * do further initialisation work alongside the constructor, you can set
 * a 'creator' using setCreator().
 * 
 * \note This is especially helpful in multi-threaded applications!
 * 
 * \par Usage in unit-tests
 * 
 * For more fine-grained control, please see the methods this class offers.
 * If you're writing unit tests, then you're probably interested in
 * storeObject(). Example:
 * 
 * \code
 * Nuria::DependencyManager::instance ()->storeObject ("MyType", myType);
 * \endcode
 * 
 * \note You have to supply a name here as the code won't be able to figure
 * out the name itself.
 * 
 */
class NURIA_CORE_EXPORT DependencyManager : public QObject {
	Q_OBJECT
public:
	
	/**
	 * Behaviours for multi-threaded applications.
	 * \sa defaultBehaviour
	 */
	enum ThreadingPolicy {
		/**
		 * Maps to the current default policy.
		 */
		DefaultPolicy,
		
		/**
		 * One pool for all objects, but with a mutex guarding the
		 * internal structures.
		 */
		ApplicationGlobal,
		
		/**
		 * One pool for all objects, with \b no mutex guards. Use this
		 * setting for single-threaded applications.
		 */
		SingleThread,
		
		/**
		 * One pool per thread. Objects are freed when the thread gets
		 * destroyed, though some structures can only be freed upon
		 * application exit.
		 * \note This is the default behaviour
		 */
		ThreadLocal
	};
	
	/** Destructor. */
	~DependencyManager ();
	
	/**
	 * Returns the global instance of the manager.
	 */
	static DependencyManager *instance ();
	
	/**
	 * Returns the current default threading policy.
	 * \sa setDefaultThreadingPolicy ThreadingPolicy
	 */
	ThreadingPolicy defaultThreadingPolicy () const;
	
	/**
	 * Sets the default threading policy.
	 * Passing DefaultPolicy has no effect.
	 * Changing the policy is \b not thread-safe.
	 * \sa ThreadingPolicy
	 */
	void setDefaultThreadingPolicy (ThreadingPolicy policy);
	
	/**
	 * Returns object \a name.
	 * If \a type is not \c -1 and \a name wasn't created yet, it will be
	 * created then, stored and returned.
	 * Else, \c nullptr is returned.
	 * 
	 * \note If \a type is not \c -1, it will be used as type check.
	 * If \a type and the type of object \a name don't match, \a nullptr
	 * is returned.
	 * 
	 * \sa getDependency NURIA_DEPENDENCY objectType
	 */
	void *objectByName (const QByteArray &name, int type = -1, ThreadingPolicy policy = DefaultPolicy);
	
	/**
	 * Returns the meta type of object \a name or \c -1 if not found.
	 */
	int objectType (const QByteArray &name, ThreadingPolicy policy = DefaultPolicy) const;
	
	/**
	 * Returns \c true if there is object \a name.
	 */
	inline bool hasObject (const QByteArray &name, ThreadingPolicy policy = DefaultPolicy) const
	{ return objectType (name, policy) != -1; }
	
	/**
	 * Stores \a object of \a type as \a name. If there is already a object
	 * of the same name, it will be overwritten. \a object \b must be a
	 * registered type.
	 * 
	 * \sa Q_DECLARE_METATYPE qRegisterMetaType
	 */
	void storeObject (const QByteArray &name, void *object, int type,
			  ThreadingPolicy policy = DefaultPolicy);
	
	/** \overload */
	template< typename T >
	void storeObject (const QByteArray &name, T *object)
	{ storeObject (name, object, qMetaTypeId< T * > ()); }
	
	/**
	 * Sets the creator of object \a name to \a creator. When the object
	 * \a name is read and has not been created yet, \a creator will be
	 * called.
	 * 
	 * Please note that this function is \b not thread-safe, as you'll
	 * use this function at the start of your application and then start
	 * threads.
	 * 
	 * \note \a creator must be thread-safe!
	 */
	void setCreator (const QByteArray &name, const std::function< QObject *() > &creator);
	
	/**
	 * Tries to find object \a name of type \a T.
	 * On failure, \c nullptr is returned.
	 */
	template< typename T >
	inline static T *get (const QByteArray &name, ThreadingPolicy policy = DefaultPolicy) {
		return static_cast< T * > (instance ()->objectByName (name, qMetaTypeId< T * > (), policy));
	}
	
private slots:
	
	void freeAllObjects ();
	
private:
	explicit DependencyManager (QObject *parent = 0);
	
	DependencyManagerPrivate *d_ptr;
};

/**
 * \brief Smart pointer class for dependency injection.
 * 
 * Smart pointer which lazy-loads a referenced dependency from
 * DependencyManager.
 * 
 * \par Usage
 * 
 * For objects with the default name, it's sufficient to use the default
 * constructor:
 * \code
 * Dependency< MyType > myType;
 * \endcode
 * 
 * If you're using a different object name, you can pass it:
 * \code
 * Dependency< MyType > myType (QByteArrayLiteral("someType"));
 * \endcode
 * 
 * To access the instance, use operator-> as you'd any other pointer:
 * \code
 * Dependency< MyType > myType;
 * myType->doSomething ();
 * \endcode
 * 
 * \sa DependencyManager
 */
template< typename T >
class Dependency {
	mutable T *m_obj = nullptr;
	QByteArray m_name;
public:
	
	/**
	 * Constructor. Takes an optional argument \a objectName. If not set,
	 * the name of the type of \c T is used.
	 */
	Dependency (const QByteArray &objectName = QByteArray ())
	        : m_name (objectName)
	{
		if (objectName.isEmpty ()) {
			const char *name = QMetaType::typeName (qMetaTypeId< T * > ());
			this->m_name.setRawData (name, ::qstrlen (name));
		}
		
	}
	
	/** Convenience accessor. See get(). */
	T *operator-> () const
	{ return get (); }
	
	/**
	 * Returns the referenced instance. On first invocation, it will be
	 * fetched from the DependencyManager.
	 */
	T *get () const {
		if (!this->m_obj) {
			this->m_obj = DependencyManager::get< T > (this->m_name);
		}
		
		return this->m_obj;
	}
	
};

}

#endif // NURIA_DEPEDENCYMANAGER_HPP
