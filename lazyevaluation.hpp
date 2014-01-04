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

#ifndef NURIA_LAZYEVALUATION_HPP
#define NURIA_LAZYEVALUATION_HPP

#include "essentials.hpp"
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
		Empty, /// Invalid instance, evaluates to \c false.
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

class NURIA_CORE_EXPORT Field {
public:
	
	enum Type {
		Empty,
		Value,
		Argument,
		TestCall
	};
	
	Field ();
	Field (Type type, const QVariant &data);
	Field (const Field &other);
	
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
	
	// 
	const QVariant &value () const;
	Type type () const;
	QVariant toVariant () const;
	
private:
	Type m_type;
	QVariant m_value;
};

class TestCall {
public:
	TestCall () {}
	TestCall (const QString &name, const QVariantList &args)
		: m_name (name), m_args (args)
	{}
	
	const QString &name () const
	{ return this->m_name; }
	
	const QVariantList &arguments () const
	{ return this->m_args; }
	
private:
	QString m_name;
	QVariantList m_args;
};

template< typename T > inline Field val (const T &value)
{ return Field (Field::Value, QVariant::fromValue (value)); }

NURIA_CORE_EXPORT Field arg (int index);

template< typename ... Args >
NURIA_CORE_EXPORT Field test (const QString &method, const Args &... args) {
	TestCall call (method, Variant::buildList (args ...));
	return Field (Field::TestCall, QVariant::fromValue (call));
}

}

NURIA_CORE_EXPORT QDebug operator<< (QDebug dbg, const Nuria::LazyCondition &condition);
NURIA_CORE_EXPORT QDebug operator<< (QDebug dbg, const Nuria::TestCall &call);
NURIA_CORE_EXPORT QDebug operator<< (QDebug dbg, const Nuria::Field &field);

// 
Q_DECLARE_METATYPE(Nuria::LazyCondition)
Q_DECLARE_METATYPE(Nuria::TestCall)
Q_DECLARE_METATYPE(Nuria::Field)

#endif // NURIA_LAZYEVALUATION_HPP
