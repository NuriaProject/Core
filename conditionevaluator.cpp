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

#include "conditionevaluator.hpp"

#include <QVector>


namespace Nuria {
class ConditionEvaluatorPrivate {
public:
	
	int neededArguments;
	QStringList neededMethods;
	LazyCondition condition;
	QMap< QString, Callback > methods;
	
};
}

Nuria::AbstractConditionEvaluator::AbstractConditionEvaluator () {
	
}

Nuria::AbstractConditionEvaluator::~AbstractConditionEvaluator () {
	
}

Nuria::ConditionEvaluator::ConditionEvaluator ()
	: d_ptr (new ConditionEvaluatorPrivate)
{
	this->d_ptr->neededArguments = 0;
	
}

Nuria::ConditionEvaluator::~ConditionEvaluator () {
	delete this->d_ptr;
}

bool Nuria::ConditionEvaluator::compile (const Nuria::LazyCondition &condition) {
	this->d_ptr->condition = condition;
	return true;
}

static QVariant variantValue (Nuria::ConditionEvaluatorPrivate *d, const QVariant &variant,
			      const QVariantList &arguments, bool &error);

static QVariant callMethod (Nuria::ConditionEvaluatorPrivate *d, Nuria::Callback callback,
			    QVariantList methodArgs, const QVariantList &conditionArgs, bool &error) {
	
	// Prepare arguments, replacing sub-fields
	for (int i = 0; i < methodArgs.length (); i++) {
		const QVariant &cur = methodArgs.at (i);
		methodArgs.replace (i, variantValue (d, cur, conditionArgs, error));
		if (error) {
			return false;
		}
		
	}
	
	// Invoke!
	return callback.invoke (methodArgs);
	
}

static QVariant variantValue (Nuria::ConditionEvaluatorPrivate *d, const QVariant &variant,
			      const QVariantList &arguments, bool &error) {
	if (variant.userType () != qMetaTypeId< Nuria::Field > ()) {
		return variant;
	}
	
	// 
	Nuria::Field f = variant.value< Nuria::Field > ();
	if (f.type () == Nuria::Field::Value) {
		return f.value ();
	} else if (f.type () == Nuria::Field::Argument) {
		int idx = f.value ().toInt ();
		if (idx < 0 || idx >= arguments.length ()) {
			error = true;
			return QVariant ();
		}
		
		return arguments.at (idx);
	} else if (f.type () == Nuria::Field::TestCall) {
		Nuria::TestCall method (f.value ().value< Nuria::TestCall > ());
		Nuria::Callback callback;
		
		if (method.isNative ()) {
			callback = method.callback ();
		} else {
			auto it = d->methods.constFind (method.name ());
			if (it == d->methods.constEnd ()) {
				error = true;
				return false;
			}
			
			callback = *it;
		}
		
		// 
		return callMethod (d, callback, method.arguments (), arguments, error);
		
	}
	
	// 
	return variant;
}

static bool runCondition (Nuria::ConditionEvaluatorPrivate *d, const Nuria::LazyCondition &condition,
			  const QVariantList &arguments, bool &error) {
	using namespace Nuria;
	if (condition.type () == LazyCondition::Empty) {
		return false;
	}
	
	// 
	QVariant lhs (condition.left ());
	bool leftIsCondition = (lhs.userType () == qMetaTypeId< LazyCondition > ());
	
	// 
	if (condition.type () == LazyCondition::Single) {
		if (leftIsCondition) {
			return runCondition (d, lhs.value< LazyCondition > (), arguments, error);
		}
		
		lhs = variantValue (d, lhs, arguments, error);
		if (lhs.type () == QVariant::Bool) {
			return lhs.toBool ();
		}
		
		if (Variant::canCompare (lhs, QMetaType::Bool)) {
			return Variant::equal (lhs, true);
		}
		
		return lhs.isValid ();
	}
	
	// 
	QVariant rhs (condition.right ());
	bool rightIsCondition = (rhs.userType () == qMetaTypeId< LazyCondition > ());
	
	bool leftError = false;
	bool rightError = false;
	
	// 
	QVariant left (leftIsCondition ? QVariant (runCondition (d, lhs.value< LazyCondition > (), arguments, leftError))
				       : variantValue (d, lhs, arguments, error));
	if (leftError) {
		error = true;
		return false;
	}
	
	// Short-circuit
	if (left.type () == QVariant::Bool) {
		if (condition.type () == LazyCondition::LogicOr && left.toBool ()) {
			return true;
		} else if (condition.type () == LazyCondition::LogicAnd && !left.toBool ()) {
			return false;
		}
		
	}
	
	// 
	QVariant right (rightIsCondition ? QVariant (runCondition (d, rhs.value< LazyCondition > (),
								   arguments, rightError))
					 : variantValue (d, rhs, arguments, error));
	if (rightError) {
		error = true;
		return false;
	}
	
	// 
	switch (condition.type ()) {
	case LazyCondition::Empty:
	case LazyCondition::Single:
		break;
	case LazyCondition::Equal:
		return Variant::equal (left, right);
	case LazyCondition::NonEqual:
		return Variant::notEqual (left, right);
	case LazyCondition::Greater:
		return Variant::greaterThan (left, right);
	case LazyCondition::GreaterEqual:
		return Variant::greaterEqualThan (left, right);
	case LazyCondition::Less:
		return Variant::lessThan (left, right);
	case LazyCondition::LessEqual:
		return Variant::lessEqualThan (left, right);
	case LazyCondition::LogicAnd:
		return left.toBool () && right.toBool ();
	case LazyCondition::LogicOr:
		return left.toBool () || right.toBool ();
	}
	
	// 
	return false;
}

bool Nuria::ConditionEvaluator::evaluate (const QVariantList &arguments, bool &error) {
	return runCondition (this->d_ptr, this->d_ptr->condition, arguments, error);
	
}

void Nuria::ConditionEvaluator::registerMethod (const QString &name, const Nuria::Callback &method) {
	this->d_ptr->methods.insert (name, method);
}
