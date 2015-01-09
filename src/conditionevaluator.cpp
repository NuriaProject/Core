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

#include "nuria/conditionevaluator.hpp"

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
		if (cur.userType () != qMetaTypeId< Nuria::Field > () &&
		    cur.userType () != qMetaTypeId< Nuria::LazyCondition > ()) {
			continue;
		}
		
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
		if (lhs.userType () == QMetaType::Bool) {
			return lhs.toBool ();
		}
		
		if (lhs.canConvert< bool > ()) {
			return lhs.value< bool > ();
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
	case LazyCondition::Equal: return (left == right);
	case LazyCondition::NonEqual: return (left != right);
	case LazyCondition::Greater: return (left > right);
	case LazyCondition::GreaterEqual: return (left >= right);
	case LazyCondition::Less: return (left < right);
	case LazyCondition::LessEqual: return (left <= right);
	case LazyCondition::LogicAnd: return (left.toBool () && right.toBool ());
	case LazyCondition::LogicOr: return (left.toBool () || right.toBool ());
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
