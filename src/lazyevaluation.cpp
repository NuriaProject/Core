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

#include "lazyevaluation.hpp"
#include "conditionevaluator.hpp"
#include "debug.hpp"

namespace Nuria {

class LazyConditionPrivate : public QSharedData {
public:
	LazyConditionPrivate (LazyCondition::Type t = LazyCondition::Empty,
			      const QVariant &l = QVariant (),
			      const QVariant &r = QVariant ())
		: type (t), left (l), right (r), evaluator (nullptr)
	{
	}
	
	~LazyConditionPrivate () {
		delete this->evaluator;
	}
	
	LazyCondition::Type type;
	QVariant left;
	QVariant right;
	
	mutable AbstractConditionEvaluator *evaluator;
};

}

Nuria::LazyCondition::LazyCondition ()
	: d (new LazyConditionPrivate (Empty))
{
}

Nuria::LazyCondition::~LazyCondition () {
	// 
}

Nuria::LazyCondition::LazyCondition(const Nuria::Field &field)
	: d (new LazyConditionPrivate (Single, field.toVariant ()))
{
	
}

Nuria::LazyCondition::LazyCondition (const QVariant &single)
	: d (new LazyConditionPrivate (Single, single))
{
}

Nuria::LazyCondition::LazyCondition (const Nuria::LazyCondition &other)
	: d (other.d)
{
}

Nuria::LazyCondition::LazyCondition (const QVariant &left, Nuria::LazyCondition::Type type, const QVariant &right)
	: d (new LazyConditionPrivate (type, left, right))
{
}

Nuria::LazyCondition &Nuria::LazyCondition::operator= (const Nuria::LazyCondition &other) {
	this->d = other.d;
	return *this;
}

bool Nuria::LazyCondition::isValid () const {
	return (this->d->type != Empty);
}

Nuria::LazyCondition::Type Nuria::LazyCondition::type () const {
	return this->d->type;
}

const QVariant &Nuria::LazyCondition::left () const {
	return this->d->left;
}

const QVariant &Nuria::LazyCondition::right () const {
	return this->d->right;
}

Nuria::LazyCondition Nuria::LazyCondition::operator&& (const Nuria::LazyCondition &other) {
	return LazyCondition (*this, LogicAnd, other);
}

Nuria::LazyCondition Nuria::LazyCondition::operator|| (const Nuria::LazyCondition &other) {
	return LazyCondition (*this, LogicOr, other);
}

bool Nuria::LazyCondition::evaluate (const QVariantList &arguments) const {
	if (!this->d->evaluator) {
		const_cast< LazyCondition * > (this)->compile ();
	}
	
	bool error = false;
	bool result = this->d->evaluator->evaluate (arguments, error);
	
	if (error) {
		nError() << "Failed to execute condition" << *this;
		return false;
	}
	
	return result;
}

void Nuria::LazyCondition::compile (Nuria::AbstractConditionEvaluator *evaluator) {
	if (!evaluator) {
		evaluator = new ConditionEvaluator;
	}
	
	delete this->d->evaluator;
	this->d->evaluator = evaluator;
	if (!evaluator->compile (*this)) {
		nError() << "Failed to compile condition" << *this;
	}
	
}

Nuria::LazyCondition Nuria::Field::operator== (const Nuria::Field &other) {
	return LazyCondition (toVariant (), LazyCondition::Equal, other.toVariant ());
}

Nuria::LazyCondition Nuria::Field::operator!= (const Nuria::Field &other) {
	return LazyCondition (toVariant (), LazyCondition::NonEqual, other.toVariant ());
}

Nuria::LazyCondition Nuria::Field::operator< (const Nuria::Field &other) {
	return LazyCondition (toVariant (), LazyCondition::Less, other.toVariant ());
}

Nuria::LazyCondition Nuria::Field::operator<= (const Nuria::Field &other) {
	return LazyCondition (toVariant (), LazyCondition::LessEqual, other.toVariant ());
}

Nuria::LazyCondition Nuria::Field::operator> (const Nuria::Field &other) {
	return LazyCondition (toVariant (), LazyCondition::Greater, other.toVariant ());
}

Nuria::LazyCondition Nuria::Field::operator>= (const Nuria::Field &other) {
	return LazyCondition (toVariant (), LazyCondition::GreaterEqual, other.toVariant ());
}

Nuria::LazyCondition::operator QVariant () const {
	return QVariant::fromValue (*this);
}

// 
Nuria::Field::Field ()
	: m_type (Empty)
{
}

Nuria::Field::Field (int type, const QVariant &data)
	: m_type (Type (type)), m_value (data)
{
}

Nuria::Field::Field (const Nuria::Field &other)
	: m_type (other.m_type), m_value (other.m_value)
{
}
const QVariant &Nuria::Field::value () const {
	return this->m_value;
}

Nuria::Field::Type Nuria::Field::type () const {
	if (this->m_type >= Custom) {
		return Custom;
	}
	
	return this->m_type;
}

int Nuria::Field::customType () const {
	return this->m_type;
}

QVariant Nuria::Field::toVariant () const {
	
	// Get rid of Fields which just hold a QVariant
	if (this->m_type == Field::Empty || this->m_type == Field::Value) {
		return this->m_value;
	}
	
	return QVariant::fromValue (*this);
}

// 
Nuria::TestCall::TestCall () {
	
}


Nuria::TestCall::TestCall(const QString &name, const QVariantList &args)
	: m_method (name), m_args (args)
{
}

Nuria::TestCall::TestCall(const Callback &callback, const QVariantList &args)
	: m_method (QVariant::fromValue (callback)), m_args (args)
{
}

QString Nuria::TestCall::name () const {
	return this->m_method.toString ();
}

Nuria::Callback Nuria::TestCall::callback () const {
	return this->m_method.value< Nuria::Callback > ();
}

bool Nuria::TestCall::isNative () const {
	return (this->m_method.userType () != QMetaType::QString);
}

const QVariantList &Nuria::TestCall::arguments () const {
	return this->m_args;
}

// 
static void writeDebugStream (QDebug &dbg, const QVariant &var) {
	int type = var.userType ();
	
	if (type == qMetaTypeId< Nuria::Field > ()) {
		dbg << var.value< Nuria::Field > ();
	} else if (type == qMetaTypeId< Nuria::LazyCondition > ()) {
		dbg << var.value< Nuria::LazyCondition > ();
	} else if (type == qMetaTypeId< Nuria::TestCall > ()) {
		dbg << var.value< Nuria::TestCall > ();
	} else {
		dbg << var;
	}
	
}

QDebug operator<< (QDebug dbg, const Nuria::LazyCondition &condition) {
	using namespace Nuria;
	
	const char *op = "";
	switch (condition.type ()) {
	case LazyCondition::Empty: break;
	case LazyCondition::Single: break;
	case LazyCondition::Equal: op = "=="; break;
	case LazyCondition::NonEqual: op = "!="; break;
	case LazyCondition::Greater: op = ">"; break;
	case LazyCondition::GreaterEqual: op = ">="; break;
	case LazyCondition::Less: op = "<"; break;
	case LazyCondition::LessEqual: op = "<="; break;
	case LazyCondition::LogicAnd: op = "&&"; break;
	case LazyCondition::LogicOr: op = "||"; break;
	}
	
	dbg << "{";
	writeDebugStream (dbg, condition.left ());
	dbg << op;
	writeDebugStream (dbg, condition.right ());
	dbg << "}";
	return dbg;
}

QDebug operator<< (QDebug dbg, const Nuria::TestCall &call) {
	dbg.nospace() << "TestCall<";
	
	if (call.isNative ()) {
		dbg << "Native";
	} else {
		dbg << call.name ();
	}
	
	dbg << "(";
	const QVariantList &args = call.arguments ();
	
	for (int i = 0; i < args.length (); i++) {
		const QVariant &cur = args.at (i);
		writeDebugStream (dbg, cur);
		
		if (i + 1 < args.length ())
			dbg.nospace() << ", ";
	}
	
	dbg.nospace() << ")>";
	
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const Nuria::Field &field) {
	dbg.nospace () << "Field(";
	writeDebugStream (dbg, field.value ());
	dbg << ")";
	return dbg.space ();
}

// 
Nuria::Field Nuria::arg (int index) {
	return Field (Field::Argument, index);
}
