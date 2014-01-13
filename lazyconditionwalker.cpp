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

#include "lazyconditionwalker.hpp"

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

template< typename T >
static bool invokeHandler (T &t, const HandlerMap &handlers, int type, const QVariantList &stack) {
	using namespace Nuria;
	auto it = handlers.constFind (type);
	
	if (it != handlers.constEnd ()) {
		Callback cb (*it);
		QVariant result = cb (stack, t);
		
		if (result.userType () == qMetaTypeId< T > ()) {
			t = result.value< T > ();
			return true;
		}
		
	}
	
	// 
	return false;
}

bool Nuria::LazyConditionWalker::walkCondition (Nuria::LazyCondition &condition, QVariantList &stack, bool walkArguments) {
	QVariant left (condition.left ());
	QVariant right (condition.right ());
	
	bool changedLeft = walkVariant (left, stack, walkArguments);
	bool changedRight = walkVariant (right, stack, walkArguments);
	
	if (changedLeft || changedRight) {
		condition = LazyCondition (left, condition.type (), right);
	}
	
	bool changed = invokeHandler (condition, this->d->condition, condition.type (), stack);
	return (changedLeft || changedRight || changed);
}

bool Nuria::LazyConditionWalker::walkField (Nuria::Field &field, QVariantList &stack, bool walkArguments) {
	bool changed = false;
	
	if (walkArguments && field.type () == Field::TestCall) {
		TestCall call (field.value ().value< TestCall > ());
		if (walkTestCall (call, stack, walkArguments)) {
			field = Field (Field::TestCall, QVariant::fromValue (call));
			changed = true;
		}
		
	}
	
	// 
	bool handlerChange = invokeHandler (field, this->d->field, field.customType (), stack);
	return (handlerChange || changed);
}

bool Nuria::LazyConditionWalker::walkTestCall (Nuria::TestCall &call, QVariantList &stack, bool walkArguments) {
	QVariantList args (call.arguments ());
	int changed = 0;
	int count = args.length ();
	
	for (int i = 0; i < count; i++) {
		if (walkVariant (args[i], stack, walkArguments)) {
			changed++;
		}
		
	}
	
	return (changed != count);
}

bool Nuria::LazyConditionWalker::walkVariant (QVariant &variant, QVariantList &stack, bool walkArguments) {
	bool changed = false;
	int type = variant.userType ();
	stack.append (variant);
	
	if (type == qMetaTypeId< LazyCondition > ()) {
		LazyCondition condition (variant.value< LazyCondition > ());
		if (walkCondition (condition, stack, walkArguments)) {
			variant = QVariant::fromValue (condition);
			changed = true;
		}
		
	} else if (type == qMetaTypeId< Field > ()) {
		Field field (variant.value< Field > ());
		if (walkField (field, stack, walkArguments)) {
			variant = QVariant::fromValue (field);
			changed = true;
		}
		
	} else {
		changed = invokeHandler (variant, this->d->variant, variant.userType (), stack);
		
	}
	
	// Pop stack and return
	stack.removeLast ();
	return changed;
}
