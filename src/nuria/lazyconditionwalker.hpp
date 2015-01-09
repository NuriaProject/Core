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

#ifndef NURIA_LAZYCONDITIONWALKER_HPP
#define NURIA_LAZYCONDITIONWALKER_HPP

#include <QSharedData>

#include "lazyevaluation.hpp"
#include "essentials.hpp"
#include "callback.hpp"

namespace Nuria {

class LazyConditionWalkerPrivate;

/**
 * \brief The LazyConditionWalker class lets you walk over a LazyCondition tree
 * to make modifications to it.
 * 
 * Using the on* methods, you can register handler methods. That method will be
 * passed two arguments: First a QVariantList, which contains the stack to the
 * particular item, the last element being the item itself. Second, the item
 * itself.
 * 
 * The method may return a value, which will replace the previous value. Further
 * checks don't take place on validity, so make sure that your result is legal
 * in the position.
 * 
 * All nodes are walked depth-first. Children of a returned item are skipped.
 */
class NURIA_CORE_EXPORT LazyConditionWalker {
public:
	
	/** Constructs a empty walker. */
	LazyConditionWalker ();
	
	/** Copy constructor. */
	LazyConditionWalker (const LazyConditionWalker &other);
	
	/** Destructor. */
	~LazyConditionWalker ();
	
	/** Assignment operator. */
	LazyConditionWalker &operator= (const LazyConditionWalker &other);
	
	/**
	 * Registers \a method as handler for conditions of \a type.
	 * \note There can only be one handler for \a type. If there is already
	 * one, then it will be replaced by \a method.
	 */
	void onCondition (LazyCondition::Type type, const Callback &method);
	
	/**
	 * Registers \a method as handler for fields of \a fieldType. This also
	 * works with custom types.
	 * \note There can only be one handler for \a fieldType. If there is
	 * already one, then it will be replaced by \a method.
	 */
	void onField (int fieldType, const Callback &method);
	
	/**
	 * Register \a method as handler for variants of type \a userType.
	 * 
	 * Registering a handler for LazyCondition or Field has no effect.
	 * 
	 * \note There can only be one handler for \a userType. If there is
	 * already one, then it will be replaced by \a method.
	 */
	void onVariant (int userType, const Callback &method);
	
	/**
	 * Walks over \a condition and returns the resulting condition.
	 * 
	 * When passing \c true for \a walkArguments, arguments of TestCalls are
	 * also visited. Be aware that this feature increases the complexity.
	 * 
	 * \sa onCondition onField
	 */
	LazyCondition walk (const LazyCondition &condition, bool walkArguments = false);
	
private:
	bool walkVariant (QVariant &variant, QVariantList &stack, bool walkArguments);
	bool walkCondition (QVariant &conditionVariant, QVariantList &stack, bool walkArguments);
	bool walkField (QVariant &fieldVariant, QVariantList &stack, bool walkArguments);
	bool walkTestCall (QVariant &callVariant, QVariantList &stack, bool walkArguments);
	
	QSharedDataPointer< LazyConditionWalkerPrivate > d;
	
};

}

#endif // NURIA_LAZYCONDITIONWALKER_HPP
