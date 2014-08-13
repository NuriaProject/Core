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

#include "nuria/lazyconditionwalker.hpp"

typedef QMap< int, Nuria::Callback > HandlerMap;

namespace Nuria {
class LazyConditionWalkerPrivate : public QSharedData {
public:
	
	HandlerMap condition;
	HandlerMap field;
	HandlerMap variant;
	
};

}

Nuria::LazyConditionWalker::LazyConditionWalker ()
	: d (new LazyConditionWalkerPrivate)
{
}

Nuria::LazyConditionWalker::LazyConditionWalker (const Nuria::LazyConditionWalker &other)
	: d (other.d)
{
	
}

Nuria::LazyConditionWalker::~LazyConditionWalker () {
	// 
}

Nuria::LazyConditionWalker &Nuria::LazyConditionWalker::operator= (const Nuria::LazyConditionWalker &other) {
	this->d = other.d;
	return *this;
}

void Nuria::LazyConditionWalker::onCondition (Nuria::LazyCondition::Type type, const Callback &method) {
	this->d->condition.insert (type, method);
}

void Nuria::LazyConditionWalker::onField (int fieldType, const Callback &method) {
	this->d->field.insert (fieldType, method);
}

void Nuria::LazyConditionWalker::onVariant (int userType, const Nuria::Callback &method) {
	this->d->variant.insert (userType, method);
}

Nuria::LazyCondition Nuria::LazyConditionWalker::walk (const Nuria::LazyCondition &condition,
						       bool walkArguments) {
	QVariantList stack;
	QVariant conditionVariant (QVariant::fromValue (condition));
	
	walkVariant (conditionVariant, stack, walkArguments);
	return conditionVariant.value< LazyCondition > ();
}

static bool invokeHandler (QVariant &t, const HandlerMap &handlers, int type, const QVariantList &stack) {
	using namespace Nuria;
	auto it = handlers.constFind (type);
	
	if (it != handlers.constEnd ()) {
		Callback cb (*it);
		QVariant result = cb (stack, t);
		
		if (result.isValid ()) {
			t = result;
			return true;
		}
		
	}
	
	// 
	return false;
}

bool Nuria::LazyConditionWalker::walkCondition (QVariant &conditionVariant, QVariantList &stack, bool walkArguments) {
	LazyCondition condition (conditionVariant.value< LazyCondition > ());
	
	QVariant left (condition.left ());
	QVariant right (condition.right ());
	
	bool changedLeft = walkVariant (left, stack, walkArguments);
	bool changedRight = walkVariant (right, stack, walkArguments);
	
	if (changedLeft || changedRight) {
		conditionVariant = QVariant::fromValue (LazyCondition (left, condition.type (), right));
	}
	
	bool changed = invokeHandler (conditionVariant, this->d->condition, condition.type (), stack);
	return (changedLeft || changedRight || changed);
}

bool Nuria::LazyConditionWalker::walkField (QVariant &fieldVariant, QVariantList &stack, bool walkArguments) {
	bool changed = false;
	Field field (fieldVariant.value< Field > ());
	
	if (walkArguments && field.type () == Field::TestCall) {
		QVariant value (field.value ());
		if (walkTestCall (value, stack, walkArguments)) {
			changed = true;
			fieldVariant = QVariant::fromValue (Field (Field::TestCall, value));
		}
		
	}
	
	// 
	bool handlerChange = invokeHandler (fieldVariant, this->d->field, field.customType (), stack);
	return (handlerChange || changed);
}

bool Nuria::LazyConditionWalker::walkTestCall (QVariant &callVariant, QVariantList &stack, bool walkArguments) {
	TestCall call (callVariant.value< TestCall > ());
	
	QVariantList args (call.arguments ());
	
	int changed = 0;
	int count = args.length ();
	
	for (int i = 0; i < count; i++) {
		if (walkVariant (args[i], stack, walkArguments)) {
			changed++;
		}
		
	}
	
	// Has a argument changed?
	if (changed > 0) {
		callVariant = QVariant::fromValue (TestCall (call.name (), args));
		return true;
	}
	
	return false;
}

bool Nuria::LazyConditionWalker::walkVariant (QVariant &variant, QVariantList &stack, bool walkArguments) {
	bool changed = false;
	int type = variant.userType ();
	stack.append (variant);
	
	if (type == qMetaTypeId< LazyCondition > ()) {
		changed = walkCondition (variant, stack, walkArguments);
	} else if (type == qMetaTypeId< Field > ()) {
		changed = walkField (variant, stack, walkArguments);
	} else {
		changed = invokeHandler (variant, this->d->variant, variant.userType (), stack);
	}
	
	// Pop stack and return
	stack.removeLast ();
	return changed;
}
