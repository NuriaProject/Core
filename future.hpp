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

#ifndef NURIA_FUTURE_HPP
#define NURIA_FUTURE_HPP

#include "essentials.hpp"
#include <QVariant>

namespace Nuria {

class GenericFutureWatcher;
class FuturePrivate;
template< typename T > class Future;

/** \internal */
class NURIA_CORE_EXPORT FutureBase {
public:
	FutureBase (const FutureBase &other);
	FutureBase (FuturePrivate *dptr);
	~FutureBase ();
	
	/** Assignment operator for Future<>. */
	FutureBase &operator= (const FutureBase &other);
	
	/** Comparison operator. */
	bool operator== (const FutureBase &other) const;
	
	/** Returns \c true when the task has been finished. */
	bool isFinished () const;
	
	/** Returns the expected type id of the result. */
	int type () const;
	
	/** Waits for the task to finish. */
	void waitForFinished () const;
	
private:
	template< typename T > friend class Future;
	friend class GenericFutureWatcher;
	
	FutureBase (int type);
	const QVariant &variant () const;
	void setVariant (const QVariant &v);
	
	void registerWatcher (GenericFutureWatcher *watcher);
	void unregisterWatcher (GenericFutureWatcher *watcher);
	
	FuturePrivate *d;
};

template< >
class Future< QVariant > : public FutureBase {
public:
	
	Future () : FutureBase (QMetaType::QVariant) {}
	
	Future (const QVariant &result) : FutureBase (QMetaType::QVariant)
	{ setVariant (result); }
	
	template< typename T >
	Future (const Future< T > &other) : FutureBase (other.d) {}
	
	Future (FuturePrivate *d) : FutureBase (d) {}
	
	/**
	 * Returns the result of the task.
	 * If the task is not yet completed, the method will lock.
	 */
	inline QVariant value () const
	{ return variant (); }
	
	/** Same as value(). */
	inline operator QVariant () const
	{ return variant (); }
	
	/**
	 * Used by the callee to set a value. Setting a value will mark the
	 * task as being complete.
	 */
	inline void setValue (const QVariant &val)
	{ setVariant (val); }
	
	/** Assignment operator. */
	inline Future< QVariant > &operator= (const QVariant &rhs)
	{ setVariant (rhs); return *this; }
	
	/** Returns this. */
	Future< QVariant > toGeneric () const
	{ return *this; }
	
};

template< >
class Future< void > : public FutureBase {
public:
	
	Future () : FutureBase (0) {}
	Future (bool finished) : FutureBase (0)
	{ if (finished) { setVariant (true); } }
	
	/** Returns a Future<QVariant> instance. */
	Future< QVariant > toGeneric () const
	{ return Future< QVariant > (*this); }
	
};

/**
 * \brief The Future class provides a QFuture-esque interface for tasks which
 * may take some time to complete.
 * 
 * The purpose of this class is to have an easy-to-use interface for actions
 * which may take some time to complete. These tasks may reside in a different
 * thread or may take action on a remote computer. It is ment to enhance APIs
 * which rely heavily on asynchronous tasks to make them easier to use. Instead
 * of having a lot of signals, this class would allow the callee to directly
 * communicate with the caller.
 * 
 * This class also has a specialization for \a void and \a QVariant.
 * 
 * This class is thread-safe.
 */
template< typename T >
class Future : public FutureBase {
public:
	
	Future () : FutureBase (qMetaTypeId< T > ()) {}
	
	/**
	 * Constructs a Future instance which is already finished.
	 */
	Future (const T &result) : FutureBase ()
	{ setVariant (QVariant::fromValue (result)); }
	
	/**
	 * Returns the result of the task.
	 * If the task is not yet completed, the method will lock.
	 */
	inline T value () const
	{ return qvariant_cast< T > (variant ()); }
	
	/** Same as value(). */
	inline operator T () const
	{ return qvariant_cast< T > (variant ()); }
	
	/**
	 * Used by the callee to set a value. Setting a value will mark the
	 * task as being complete.
	 */
	inline void setValue (const T &val)
	{ setVariant (QVariant::fromValue (val)); }
	
	/** Assignment operator. */
	inline Future< T > &operator= (const T &rhs)
	{ setVariant (QVariant::fromValue (rhs)); return *this; }
	
	/**
	 * Returns a generic future.
	 */
	Future< QVariant > toGeneric () const
	{ return Future< QVariant > (*this); }
	
};

}

Q_DECLARE_METATYPE(Nuria::Future< QVariant >)

#endif // NURIA_FUTURE_HPP
