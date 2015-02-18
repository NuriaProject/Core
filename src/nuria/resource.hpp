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

#ifndef NURIA_RESOURCE_HPP
#define NURIA_RESOURCE_HPP

#include "essentials.hpp"
#include "invocation.hpp"
#include <QSharedData>
#include <functional>
#include <QPointer>
#include <QVariant>
#include <QObject>
#include <QVector>

namespace Nuria {

template< typename T > class InvocationResult;
class ResourcePropertyPrivate;

class ResourceHandlerException;

/**
 * \brief Base-class for RPC interfaces
 * 
 * The Resource class describes the generic sub-set of a remote procedure call
 * interface. Its main purpose is to provide a foundation to wrap protocols into
 * a generic form and not to implement a specific one per-se.
 * 
 * Please note that resources are not meant to replace Qts signal/slot
 * mechanism.
 * 
 * \par Terminology
 * The API tries to not invent too much new vocabulary. 
 * 
 * Signals and slots: As defined by Qt
 * 
 * Resource: A resource is a interface to a remote or local piece of code or
 * program which defines a API through a resource. It's also possible to
 * build hierarchical resources by letting a resource return other resources
 * through a slot.
 * 
 * \par Writing a resource
 * When writing a Resource sub-class, besides implementing the abstract
 * methods, you'll want to provide a slot called "" (Empty string), which
 * is called by properties(). This slot must return a PropertyList as result,
 * describing this resources properties.
 * 
 * The Frameworks way of providing resources is to offer a (abstract) Resource
 * sub-class. This sub-class then offers (abstract) virtual methods which define
 * the interface slots. Further, virtual methods of Resource are reimplemented
 * to account for the interface specific details. Commonly these are
 * interfaceName(), properties() exporting the interfaces signals and slots, and
 * invokeImpl() calling the prior defined virtual methods.
 * 
 * To implement such a resource, you sub-class that class and reimplement only
 * the interface methods (and not the virtual methods of Resource).
 * 
 * Have a look at Nuria::DirectoryResource as example.
 * 
 * \note Non-default interface names \b must be written in a java reverse-domain
 * style notation to avoid collissions with interfaces defined by the
 * NuriaProject Framework.
 * 
 * Apart from this property, there are no other mandatory parts.
 * 
 * Resources may also be \b optionally serializable. This means, that instances
 * may be serialized, and later (maybe on a different machine) be instantiated
 * with that data (Formally known as deserialization). For this to work, you
 * have to implement isSerializable(), serialize() and deserialize().
 * 
 * \par Using a resource
 * While it's possible to operate directly on Resource itself, it's generally
 * a good idea to cast to the corresponding Resource sub-class for easier use.
 * 
 * Apart from that, you don't have to keep care of anything.
 * 
 * \par Overloading of signals and slots
 * Resources don't enforce any strict behaviour on overloading methods like
 * C++ does. There may be multiple signals and slots sharing the same name.
 * 
 * The exact behaviour is resource dependent.
 * 
 * \par Resource ownership
 * It should be noted that while you can use resources pretty freely, you
 * probably don't own the Resource instance itself. If you plan on storing
 * Resource pointers, you should use ResourcePointer.
 * 
 * Signals and slots returning a Resource instance should also use
 * ResourcePointer.
 * 
 */
class NURIA_CORE_EXPORT Resource : public QObject {
	Q_OBJECT
	Q_ENUMS(InvokeResultState)
public:
	
	/**
	 * Enumeration of invocation result states for InvokeCallback.
	 * \sa resultStateName
	 */
	enum InvokeResultState {
		
		/**
		 * The invocation was successful. Note that all other values
		 * indicate an invocation error, even if its value is not in
		 * this enumeration.
		 */
		Success = 0,
		
		/** The invocation has been cancelled. */
		Cancelled = 1,
		
		/**
		 * The invocation failed as the referenced signal or slot is
		 * unknown to the resource.
		 */
		UnknownError = 2,
		
		/**
		 * The invocation failed due to one or more missing arguments
		 * or arguments being the wrong type.
		 */
		BadArgumentError = 3,
		
		/**
		 * The invocation failed as the signal or slot is unavailable,
		 * e.g. because it is not implemented.
		 */
		UnavailableError = 4,
		
		/**
		 * A unrecoverable failure occured while processing the slot,
		 * leading to a exception. The 'value' argument of the
		 * InvokeCallback will contain a QString containing the
		 * stacktrace, or some other kind of human-readable error
		 * message.
		 */
		ExceptionError = 5,
		
		/**
		 * A resource that was somehow referenced towards a slot has
		 * not been found.
		 */
		ResourceNotFoundError = 6,
		
		/**
		 * The service offering a resource is currently unavailable.
		 */
		ServiceNotAvailableError = 7,
		
		/**
		 * The invocation failed as the timeout has been reached.
		 */
		TimeoutError = 30,
		
		/** Some kind of generic invocation error occured. */
		InvokeError = 50,
		
		/**
		 * Base value for user-defined error codes. Codes equal to or
		 * greater than this value are free to be defined by user code.
		 */
		UserError = 1000
	};
	
	/** Map for mapping argument names (key) to a type (value). */
	typedef QMap< QString /* name */, int /* Qt type id */ > ArgumentTypeMap;
	
	/**
	 * \brief Describes a property of a Resource.
	 * 
	 * Properties are signals and slots of a resource, and this structure
	 * stores meta-data about them.
	 * 
	 * \sa Resource::properties
	 */
	class Property {
	public:
		
		/** Property types */
		enum Type {
			Invalid = 0, /// Invalid instance
			Signal = 1, /// Signal
			Slot = 2, /// Slot
		};
		
		/** Constructor for an invalid instance. */
		Property ();
		
		/** Copy constructor. */
		Property (const Property &other);
		
		/** Assignment operator. */
		Property &operator= (const Property &other);
		
		/** Constructs a valid instance. */
		Property (Type type, const QString &name, const ArgumentTypeMap &arguments,
		          int resultType = QMetaType::Void);
		
		/** Destructor. */
		~Property ();
		
		/** Returns \c true if this is a valid instance. */
		bool isValid () const;
		
		/** Returns the type of this property. */
		Type type () const;
		
		/** Returns this property's name. */
		QString name () const;
		
		/** Returns this property's argument description map. */
		ArgumentTypeMap arguments () const;
		
		/** Returns the datatype this property returns. */
		int resultType () const;
		
	private:
		QExplicitlySharedDataPointer< ResourcePropertyPrivate > d;
	};
	
	/** List of properties. */
	typedef QVector< Property > PropertyList;
	
	/**
	 * Type of the callback argument of invoke().
	 * 
	 * The callback gets two arguments. The first one is of type
	 * InvokeResultState, whose value indicates the state of the invocation
	 * itself. The second argument is a QVariant, containing the result of
	 * the invocation, or if the first argument is un-equal to Success,
	 * optionally some kind of error.
	 */
	typedef std::function< void(InvokeResultState /* resultState */, const QVariant &/* result */) > InvokeCallback;
	
	/** Constructor. */
	explicit Resource (QObject *parent = nullptr);
	
	/** Destructor. */
	~Resource () override;
	
	/**
	 * Returns the name of the interface this resource implements. Can be
	 * one of the default ones or a custom one.
	 */
	virtual QString interfaceName () const = 0;
	
	/**
	 * Returns \c true if this resource is serializable. The default
	 * implementation returns \c false.
	 */
	virtual bool isSerializable () const;
	
	/**
	 * Serializes the instance and returns it as QByteArray.
	 * The default implementation returns a empty one.
	 */
	virtual QByteArray serialize ();
	
	/**
	 * Deserializes the instance from \a data. \c true is returned on
	 * success.
	 * The default implementation does nothing and returns \c false.
	 */
	virtual bool deserialize (const QByteArray &data);
	
	/**
	 * Requests the resource to invoke \a slot with \a arguments. On
	 * completion, \a callback is called (Its arguments are outlined in
	 * the InvokeCallback documentation).
	 * 
	 * The optional \a timeout argument is a operation timeout in
	 * \b milli-seconds. It defaults to \c -1, which tells the
	 * resource to choose a appropriate timeout value by itself.
	 * 
	 * The returned Invocation instance points to the slot invocation,
	 * which may be returned through it. If the Invocation is destructed,
	 * the slot is \b not cancelled.
	 * 
	 * \note \a callback may be called at any time, including from inside
	 * invoke() itself.
	 * 
	 */
	Invocation invoke (const QString &slot, const QVariantMap &arguments, InvokeCallback callback,
	                   int timeout = -1);
	
	/**
	 * Connects to \a signal of this resource, invoking \a callback for
	 * each signal emission. The returned Invocation instance can be used
	 * to control the signal connection. If the Invocation instance itself
	 * gets destroyed, the connection is \b not cancelled.
	 * 
	 * On error, \a callback may be called with a 'resultState' un-equal to
	 * Success. In this case, the signal connection is \b cancelled
	 * automatically by the resource.
	 * 
	 * The 'result' argument of \a callback will be of type QVariantMap when
	 * 'resultState' is equal to Success. It will contain the signal
	 * arguments in a name to value mapping.
	 * 
	 * The default implementation calls \a callback with UnavailableError
	 * and returns a invalid Invocation instance. This is sufficient for
	 * resources which don't offer signals.
	 */
	Invocation listen (const QString &signal, InvokeCallback callback);
	
	/**
	 * Convenience function for invoke(), returning a InvocationResult which
	 * allows to use the API in a synchronous fashion.
	 * 
	 * \sa InvocationResult
	 */
	template< typename T >
	InvocationResult< T > invoke (const QString &slot, const QVariantMap &arguments, int timeout = -1);
	
	/**
	 * Acquires the list of properties of the resource.
	 * 
	 * It returns a Invocation instance and calls \a callback with the
	 * result of the operation. \a callback is called on success with a
	 * result of type PropertyList.
	 * 
	 * This method is also always available as property, named "" (empty
	 * string). As such, implementations may choose to include or exclude
	 * this property from the property list.
	 * 
	 * This method must be implemented by sub-classes.
	 */
	virtual Invocation properties (InvokeCallback callback, int timeout = -1) = 0;
	
	/**
	 * Given \a state, returns the human-readable name of it. If \a state
	 * is unknown, returns "<Unknown:STATE>", with \c STATE being the
	 * integer representation of \a state.
	 */
	static QString resultStateName (InvokeResultState state);
	
protected:
	
	/**
	 * See invoke() for general behaviour information.
	 * 
	 * The default implementation will call properties() if \a slot is "".
	 * Else, it invokes \a callback with a UnknownError.
	 * 
	 * Implementations may throw a ResourceHandlerException.
	 */
	virtual Invocation invokeImpl (const QString &slot, const QVariantMap &arguments, InvokeCallback callback,
	                               int timeout);
	
	/**
	 * See listen() for general behaviour information.
	 * 
	 * Implementations may throw a ResourceHandlerException.
	 */
	virtual Invocation listenImpl (const QString &signal, InvokeCallback callback);
	
	
};

/** Weak pointer to a Resource. */
typedef QPointer< Resource > ResourcePointer;

/** \brief Base class for InvocationResult */
class InvocationResultBase {
public:
	
	/** Modes of waiting implementations for waitForFinished(). */
	enum WaitMode {
		
		/**
		 * Default mode. Chooses Blocking if the processing resource
		 * lives in a different thread than the current one. Else, 
		 * EventLoop is chosen.
		 */
		Automatic = 0,
		
		/**
		 * Uses a temporary Qt event loop to block the caller, but
		 * continue processing in the current thread. Works for
		 * resources which live inside and outside of the current
		 * thread.
		 */
		EventLoop = 1,
		
		/**
		 * Uses a mutex to lock the current thread to wait. For this
		 * to work the processing resource \b must live in another
		 * thread, else the thread will \b deadlock.
		 */
		Blocking = 2,
	};
	
	/** Copy constructor. */
	InvocationResultBase (const InvocationResultBase &other);
	
	/** Assignment operator. */
	InvocationResultBase &operator= (const InvocationResultBase &other);
	
	/** Returns the result of this operation. */
	QVariant result () const;
	
	/**
	 * Returns \c true when the invocation has been finished.
	 * \sa hasError.
	 */
	bool hasFinished () const;
	
	/** Returns \c true if an error occured. */
	bool hasError () const;
	
	Resource::InvokeResultState resultState () const;
	
	/** Returns the Invocation instance. */
	Invocation invocation () const;
	
	/**
	 * Blocks the caller until the invocation has finished according to
	 * \a waitMode. Returns the invocation state.
	 * 
	 * \sa WaitMode
	 */
	Resource::InvokeResultState waitForFinished (WaitMode waitMode = Automatic);
	
protected:
	friend class Resource;
	struct Data;
	
	QExplicitlySharedDataPointer< Data > d;
	
	InvocationResultBase ();
	~InvocationResultBase ();
	void init (const Invocation &invocation);
	
	void callback (Resource::InvokeResultState resultState, const QVariant &result);
	void wait (WaitMode waitMode);
	
};

/**
 * \brief Convenience class for signal/slot invocations.
 * 
 * This class is std::future-esque to allow using Resource::invoke() in a
 * synchronous fashion. Example code:
 * \code
 * Nuria::InvocationResult< QString > result = resource->invoke< QString > ("getString", { });
 * if (result.wait () != Nuria::Resource::Success) { .. error handling .. }
 * QString str = result;
 * \endcode
 * 
 * \note When \c T is \c void, this class offers no methods on its own.
 * 
 * Also see InvocationResultBase for methods.
 */
template< typename T >
class InvocationResult : public InvocationResultBase {
public:
	
	/** Returns the result. \sa hasFinished hasError waitForFinished */
	operator T () const
	{ return value (); }
	
	/** Returns the result of this operation. */
	T value () const // For some reason gcc rejects the following line without the cast.
	{ return static_cast< QVariant > (result ()).value< T > (); }
	
};

// Specialisation for T = void
template< >
class InvocationResult< void > : public InvocationResultBase {
	// 
};

/**
 * \brief Convenience class for writers of Resource implementations
 * 
 * This class can be thrown Resource::invokeImpl() and Resource::listenImpl() to
 * Resource::invoke() or Resource::listen() respectively, both of which catch
 * this type of exception.
 * 
 * \note This exception is only intended for internal use to make it ease
 * writing resources.
 * 
 * \warning As exceptions are quite uncommon in Qt code, please make sure that
 * when using this exception that your code is exception-safe!
 */
class ResourceHandlerException : public std::exception {
public:
	
	/** Constructor. */
	ResourceHandlerException (Resource::InvokeResultState state, const QVariant &result = QVariant ()) noexcept;
	
	/** Destructor. */
	~ResourceHandlerException ();
	
	// std::exception
	
	/** Returns an empty C-string. */
	const char *what () const noexcept;
	
private:
	friend class Resource;
	
	Resource::InvokeResultState m_state;
	QVariant m_result;
};

template< typename T >
InvocationResult< T > Resource::invoke (const QString &slot, const QVariantMap &arguments, int timeout) {
	InvocationResult< T > result;
	auto cb = [result](Resource::InvokeResultState resultState, const QVariant &r) mutable {
		result.callback (resultState, r);
	};
	
	// 
	result.init (invoke (slot, arguments, cb, timeout));
	return result;
}

}

Q_DECLARE_METATYPE(Nuria::Resource::InvokeResultState);
Q_DECLARE_METATYPE(Nuria::Resource::InvokeCallback);
Q_DECLARE_METATYPE(Nuria::Resource::Property);
Q_DECLARE_METATYPE(Nuria::Resource::PropertyList);
Q_DECLARE_METATYPE(Nuria::ResourcePointer);
Q_DECLARE_METATYPE(Nuria::Resource*);

#endif // NURIA_RESOURCE_HPP
