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

#ifndef NURIA_LAZYEVALUATION_HPP
#define NURIA_LAZYEVALUATION_HPP

#include "essentials.hpp"
#include "callback.hpp"
#include "variant.hpp"
#include <QSharedData>
#include <QMetaType>
#include <QVariant>
#include <QDebug>

namespace Nuria {

class AbstractConditionEvaluator;
class LazyConditionPrivate;
class Field;

/**
 * \brief LazyCondition offers lazily evaluated conditions for C++.
 * 
 * \par Usage
 * LazyCondition tries to be as simple to use as possible, thus it mimics the
 * behaviour to C++. This means that you can write conditions like you're used
 * to from C++. Please be aware of possible problems when mixing lazy and strict
 * evaluation by accident. Expressions that are written without telling the
 * compiler about LazyCondition will be evaluated \b before it is passed to
 * LazyCondition! For this reason, you need to use Nuria::val to start a lazy
 * condition.
 * 
 * This class also acts as functor, meaning that you can later call it like a
 * method. This also means that you can pass a instance to every function which
 * expects a functor, like STL algorithms.
 * 
 * You can reference to arguments by index (Starting from 0) using Nuria::arg.
 * 
 * \par Example
 * The following example removes the first two elements from 'list' using
 * std::remove_if and a lazy condition:
 * 
 * \code
 * QList< int > list { 1, 2, 3, 4, 5 };
 * std::remove_if (list.begin (), list.end (), arg(0) < 3);
 * qDebug() << list;
 * \endcode
 */
class NURIA_CORE_EXPORT LazyCondition {
public:
	
	/**
	 * Available types of conditions.
	 */
	enum Type {
		Empty = 0, /// Invalid instance, evaluates to \c false.
		Single, /// Evaluates to (left).
		Equal, /// Evaluates to (left == right).
		NonEqual, /// Evaluates to (left != right).
		Greater, /// Evaluates to (left > right).
		GreaterEqual, /// Evaluates to (left >= right).
		Less, /// Evaluates to (left < right).
		LessEqual, /// Evaluates to (left <= right).
		LogicAnd, /// Evaluates to (left && right).
		LogicOr /// Evaluates to (left || right).
	};
	
	/** Constructs a instance of type Empty. */
	LazyCondition ();
	
	/** Destructor. */
	~LazyCondition ();
	
	/** Constructs a instance of type Single. */
	LazyCondition (const Field &field);
	
	/** Constructs a instance of type Single. */
	LazyCondition (const QVariant &single);
	
	/** Copy constructor. */
	LazyCondition (const LazyCondition &other);
	
	/** Constructor. */
	LazyCondition (const QVariant &left, Type type, const QVariant &right);
	
	/** Assignment operator. */
	LazyCondition &operator= (const LazyCondition &other);
	
	/**
	 * Returns \c true if the type of this condition is not Empty.
	 */
	bool isValid () const;
	
	/**
	 * Returns the type of this instance.
	 * \sa Type
	 */
	Type type () const;
	
	/** Returns the left-hand side value of the condition. */
	const QVariant &left () const;
	
	/** Returns the right-hand side value of the condition. */
	const QVariant &right () const;
	
	/** Allow implicit conversion to QVariant. */
	operator QVariant () const;
	
	/**
	 * Returns a LazyCondition with a logical \b and conjunction between
	 * this instance (on the left-hand side) and \a other (on the
	 * right-hand side).
	 */
	LazyCondition operator&& (const LazyCondition &other);
	
	/**
	 * Returns a LazyCondition with a logical \b or conjunction between
	 * this instance (on the left-hand side) and \a other (on the
	 * right-hand side).
	 */
	LazyCondition operator|| (const LazyCondition &other);
	
	/**
	 * Evaluates the condition. 
	 */
	template< typename ... Args >
	inline bool operator() (const Args & ... args)
	{ return evaluate (Variant::buildList (args ...)); }
	
	/**
	 * Evaluates the condition with \a arguments as input.
	 * If not already done the condition will be first compiled.
	 * \sa compile
	 */
	bool evaluate (const QVariantList &arguments) const;
	
	/**
	 * Compiles the condition using \a evaluator. If no \a evaluator is
	 * given, the default implementation will be used. Ownership of
	 * \a evaluator will be transferred to this instance.
	 * 
	 * \sa AbstractConditionEvaluator ConditionEvaluator
	 * 
	 * \note You only need to use this method explicitly if you want to use
	 * a custom evaluator.
	 */
	void compile (AbstractConditionEvaluator *evaluator = nullptr);
	
private:
	QSharedDataPointer< LazyConditionPrivate > d;
};

/**
 * \brief Field encapsulates a value-field for LazyCondition.
 * 
 * This class acts as a generic container for LazyConditions. You usually don't
 * use it explicitly, but instead use the already defined methods to create
 * instances of this class.
 * 
 * \sa val arg
 */
class NURIA_CORE_EXPORT Field {
public:
	
	/**
	 * Field types.
	 */
	enum Type {
		/** Empty, invalid field. */
		Empty = 0,
		
		/** Value field. \sa val */
		Value,
		
		/**
		 * Argument reference, the value will be of type int.
		 * \sa arg
		 */
		Argument,
		
		/**
		 * A call to a named method. To be defined by the evaluator.
		 */
		TestCall,
		
		/**
		 * A custom type used in domain-specific environments, like
		 * references to table columns. This acts as a base value,
		 * users of this features should try to use unique type values.
		 * 
		 * Example:
		 * \codeline enum { MyField = Nuria::Field::Custom + 1 };
		 */
		Custom = 50
	};
	
	/** Creates a instance of type Empty. */
	Field ();
	
	/** Creates a instance of \type with value \a data. */
	Field (int type, const QVariant &data);
	
	/** Copy constructor. */
	Field (const Field &other);
	
	/**
	 * \addtogroup Operators
	 * @{
	 */
	
	// 
	LazyCondition operator== (const Field &other);
	LazyCondition operator!= (const Field &other);
	LazyCondition operator< (const Field &other);
	LazyCondition operator<= (const Field &other);
	LazyCondition operator> (const Field &other);
	LazyCondition operator>= (const Field &other);
	
	// 
	template< typename T >
	LazyCondition operator== (const T &other)
	{ return LazyCondition (toVariant (), LazyCondition::Equal, QVariant::fromValue (other)); }
	
	template< typename T >
	LazyCondition operator!= (const T &other)
	{ return LazyCondition (toVariant (), LazyCondition::NonEqual, QVariant::fromValue (other)); }
	
	template< typename T >
	LazyCondition operator< (const T &other)
	{ return LazyCondition (toVariant (), LazyCondition::Less, QVariant::fromValue (other)); }
	
	template< typename T >
	LazyCondition operator<= (const T &other)
	{ return LazyCondition (toVariant (), LazyCondition::LessEqual, QVariant::fromValue (other)); }
	
	template< typename T >
	LazyCondition operator> (const T &other)
	{ return LazyCondition (toVariant (), LazyCondition::Greater, QVariant::fromValue (other)); }
	
	template< typename T >
	LazyCondition operator>= (const T &other)
	{ return LazyCondition (toVariant (), LazyCondition::GreaterEqual, QVariant::fromValue (other)); }
	
	/** @} */
	
	/**
	 * Returns the value of this instance.
	 */
	const QVariant &value () const;
	
	/**
	 * Returns the type of this instance. If this is a custom instance,
	 * Custom will be returned.
	 * \sa customType
	 */
	Type type () const;
	
	/**
	 * Returns the custom type id if this is a custom type, or the regular
	 * type.
	 */
	int customType () const;
	
	/**
	 * Puts this instance into a QVariant. If the type of this instance is
	 * of Empty or Value, the value itself will be returned. Else this
	 * instance will be put into a QVariant and then returned.
	 */
	QVariant toVariant () const;
	
private:
	Type m_type;
	QVariant m_value;
};

/**
 * \brief The TestCall class encapsulates a call to a test function for
 * LazyCondition.
 * 
 * \note Usually you don't interact with this class directly, instead see test.
 * 
 * This class makes it possible for LazyCondition and evaluators to interact
 * with test methods. Methods can be of two types: Named or native.
 * 
 * A named method can be used with any evaluator, but it's up to the evaluator
 * which methods are supported. A evaluator may support registering methods or
 * may only provide pre-specified ones.
 * 
 * Native methods on the other hand are stored as Callback and thus are only
 * usable if the evaluator is run in the application itself. These conditions
 * can't be serialized and should be used with care.
 * 
 * \sa Nuria::test
 */
class NURIA_CORE_EXPORT TestCall {
public:
	
	/** Creates a invalid instance. */
	TestCall ();
	
	/** Creates a instance refering to a \b named method. */
	TestCall (const QString &name, const QVariantList &args);
	
	/** Creates a instance refering to a \b native method. */
	TestCall (const Nuria::Callback &callback, const QVariantList &args);
	
	/**
	 * Returns \c true if this is a native method.
	 * \sa name callback
	 */
	bool isNative () const;
	
	/**
	 * Returns the name of the method if this is a named method. Else a
	 * empty string is returned. \sa isNative
	 */
	QString name () const;
	
	/**
	 * Returns the Callback instance if this is a native method. Else a
	 * invalid Callback is returned. \sa isNative
	 */
	Nuria::Callback callback () const;
	
	/**
	 * Returns the arguments which are passed to the method. May contain
	 * other Nuria::Field instances, including method calls.
	 */
	const QVariantList &arguments () const;
	
private:
	QVariant m_method;
	QVariantList m_args;
};

/**
 * Turns any \a value into a Field of type Field::Value. Use this to force the
 * compiler to use LazyCondition semantics.
 * \sa LazyCondition arg
 */
template< typename T > inline Field val (const T &value)
{ return Field (Field::Value, QVariant::fromValue (value)); }

/**
 * Returns a Field of type Field::Argument refering to the n'th argument at
 * \a index.
 */
NURIA_CORE_EXPORT Field arg (int index);

/**
 * Returns a Field of type Field::TestCall. This can be used to call named
 * methods in LazyConditions. Which methods are available is up to the
 * definition of the used evaluator.
 */
template< typename ... Args >
Field test (const QString &method, const Args &... args) {
	TestCall call (method, Variant::buildList (args ...));
	return Field (Field::TestCall, QVariant::fromValue (call));
}

/**
 * Returns a Field of type Field::TestCall. It will directly call \a callback
 * with \a args.
 * \warning This is only supported by native condition evaluators!
 */
template< typename ... Args >
Field test (const Nuria::Callback &method, const Args &... args) {
	TestCall call (method, Variant::buildList (args ...));
	return Field (Field::TestCall, QVariant::fromValue (call));
}

}

/**
 * \addtogroup Debug operators
 * @{
 */
NURIA_CORE_EXPORT QDebug operator<< (QDebug dbg, const Nuria::LazyCondition &condition);
NURIA_CORE_EXPORT QDebug operator<< (QDebug dbg, const Nuria::TestCall &call);
NURIA_CORE_EXPORT QDebug operator<< (QDebug dbg, const Nuria::Field &field);
/** @} */

// 
Q_DECLARE_METATYPE(Nuria::LazyCondition)
Q_DECLARE_METATYPE(Nuria::TestCall)
Q_DECLARE_METATYPE(Nuria::Field)

#endif // NURIA_LAZYEVALUATION_HPP
