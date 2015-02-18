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

#ifndef NURIA_INVOCATION_HPP
#define NURIA_INVOCATION_HPP

#include "essentials.hpp"
#include <QSharedPointer>
#include <QMetaType>

namespace Nuria {

class InvocationPrivate;
class Resource;

/**
 * \brief Tracks signal/slot invocations on Resource instances
 * 
 * Invocation is returned by Resource::invoke() and Resource::listen() and is
 * used to track the processing status of these invocations.
 * 
 * The class itself is only a interface to work with running invocations.
 * To obtain the result of one, see the methods in Resource or InvocationResult.
 * 
 * If this class is destroyed, the corresponding invocation is \b not cancelled.
 * 
 */
class NURIA_CORE_EXPORT Invocation {
public:
	
	/**
	 * Interface class to be implemented by Resource implementations.
	 * 
	 * \note Invocation instances pointing to a destroyed Resource will
	 * \b not call any of these methods.
	 */
	class Interface {
	public:
		
		/** Cancels the invocation. */
		virtual void cancel () = 0;
		
	};
	
	/** Constructs an invalid instance */
	Invocation ();
	
	/**
	 * Constructs a valid instance.
	 * 
	 * Resource implementors must sub-class Interface and pass an instance
	 * to this constructor as shared pointer.
	 * 
	 * Note that \a processingResource is the resource actually processing
	 * the invocation, it \b may be different to the resource that was
	 * initially called by the end-user! Doing so is important for
	 * InvocationResult to work properly.
	 * 
	 * If the resource implements no means interface for a particular signal
	 * or slot, it's sufficient to use the default argument of \a interface.
	 */
	Invocation (Resource *processingResource,
	            const QSharedPointer< Interface > &interface = QSharedPointer< Interface > ());
	
	/** Copy constructor. */
	Invocation (const Invocation &other);
	
	/** Assignment operator. */
	Invocation &operator= (const Invocation &other);
	
	/** Destructor. */
	~Invocation ();
	
	/**
	 * Returns the resource \b processing the invocation. If the resource
	 * has been destroyed in the meantime, this function returns \c nullptr.
	 */
	Resource *resource () const;
	
	/**
	 * Cancels the execution of the slot or stops listening on the signal.
	 * 
	 * If this actually works for slots is implementation-specific.
	 */
	void cancel ();
	
private:
	QExplicitlySharedDataPointer< InvocationPrivate > d;
};

}

Q_DECLARE_METATYPE(Nuria::Invocation);

#endif // NURIA_INVOCATION_HPP
