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

#include "metaobject.hpp"

#include <QReadWriteLock>
#include <functional>

#include "debug.hpp"

enum Categories {
	ObjectCategory = 0,
	MethodCategory = 1,
	FieldCategory = 2,
	EnumCategory = 3
};

// 
static QReadWriteLock g_lock;
static Nuria::MetaObjectMap g_metaObjects;

// Binary find in the range 0 to total. If not found, returns the position
// the element would've been.
// f(index, value) returns the result of storage[index] < value.
template< typename T, typename Func >
static int binaryFind (int total, const T &value, Func f) {
	if (!total) {
		return -1;
	}
	
	int min = 0;
	while (min < total) {
		int mid = min + (total - min) / 2;
		if (f (mid, value)) {
			min = mid + 1;
		} else {
			total = mid;
		}
		
	}
	
	return (total == min) ? min : -1;
}

#define RETURN_CALL_GATE(Method, Category, Index, Nth) \
	if (!this->m_meta) return result; \
	this->m_meta->gateCall (Method, Category, Index, Nth, &result); \
	return result;

// 
Nuria::MetaObject *Nuria::MetaObject::byName (const QByteArray &type) {
	MetaObject *meta = nullptr;
	
	g_lock.lockForRead ();
	meta = g_metaObjects.value (type);
	g_lock.unlock ();
	
	return meta;
}

Nuria::MetaObjectMap Nuria::MetaObject::typesInheriting (const QByteArray &typeName) {
	MetaObjectMap map;
	
	g_lock.lockForRead ();
	auto it = g_metaObjects.constBegin ();
	auto end = g_metaObjects.constEnd ();
	
	for (; it != end; ++it) {
		QVector< QByteArray > parents = it.value ()->parents ();
		if (std::binary_search (parents.constBegin (), parents.constEnd (), typeName)) {
			map.insert (it.key (), it.value ());
		}
		
	}
	
	g_lock.unlock ();
	
	return map;
}

Nuria::MetaObjectMap Nuria::MetaObject::typesWithAnnotation (const QByteArray &name) {
	MetaObjectMap map;
	
	auto compare = [&name](int i, MetaObject *obj) {
		return obj->annotation (i).name () < name;
	};
	
	g_lock.lockForRead ();
	auto it = g_metaObjects.constBegin ();
	auto end = g_metaObjects.constEnd ();
	
	for (; it != end; ++it) {
		MetaObject *cur = *it;
		int idx = binaryFind (cur->annotationCount (), cur, compare);
		
		if (idx != -1 && cur->annotation (idx).name () == name) {
			map.insert (it.key (), cur);
		}
		
	}
	
	g_lock.unlock ();
	
	return map;
}

Nuria::MetaObjectMap Nuria::MetaObject::allTypes () {
	return g_metaObjects;
}

void Nuria::MetaObject::registerMetaObject (Nuria::MetaObject *object) {
	QByteArray name = object->className ();
	
//	nDebug() << "Registering" << object << name;
	
	g_lock.lockForWrite ();
	if (g_metaObjects.value (name, object) != object) {
		nWarn() << "Registering already registered type" << name;
	}
	
	g_metaObjects.insert (name, object);
	g_lock.unlock ();
	
}

QByteArray Nuria::MetaObject::className () {
	QByteArray name;
	gateCall (GateMethod::ClassName, 0, 0, 0, &name);
	return name;
}

int Nuria::MetaObject::metaTypeId () {
	int type = 0;
	gateCall (GateMethod::MetaTypeId, 0, 0, 0, &type);
	return type;
}

int Nuria::MetaObject::pointerMetaTypeId () {
	int type = 0;
	gateCall (GateMethod::PointerMetaTypeId, 0, 0, 0, &type);
	return type;
}

QVector< QByteArray > Nuria::MetaObject::parents () {
	QVector< QByteArray > bases;
	gateCall (GateMethod::BaseClasses, 0, 0, 0, &bases);
	return bases;
	
}

int Nuria::MetaObject::annotationCount () {
	int count = 0;
	gateCall (GateMethod::AnnotationCount, ObjectCategory, 0, 0, &count);
	return count;
}

Nuria::MetaAnnotation Nuria::MetaObject::annotation(int idx) {
	return MetaAnnotation (this, ObjectCategory, 0, idx);
}

int Nuria::MetaObject::methodCount () {
	int count = 0;
	gateCall (GateMethod::MethodCount, 0, 0, 0, &count);
	return count;
}

Nuria::MetaMethod Nuria::MetaObject::method (int idx) {
	return MetaMethod (this, idx);
}

int Nuria::MetaObject::methodLowerBound (const QByteArray &name) {
	auto compare = [&name](int i, MetaObject *obj) {
		return obj->method (i).name () < name;
	};
	
	int total = methodCount ();
	int mid = binaryFind (total, this, compare);
	
	if (mid < 0 || MetaMethod (this, mid).name () != name) {
		return -1;
	}
	
	for (mid--; mid >= 0 && method (mid).name () == name; mid--);
	return mid + 1;
	
}

int Nuria::MetaObject::methodUpperBound (const QByteArray &name) {
	auto compare = [&name](int i, MetaObject *obj) {
		return obj->method (i).name () < name;
	};
	
	int total = methodCount ();
	int mid = binaryFind (total, this, compare);
	
	if (mid < 0 || MetaMethod (this, mid).name () != name) {
		return -1;
	}
	
	for (mid++; mid < total && method (mid).name () == name; mid++);
	return mid - 1;
	
}

inline static bool methodArgumentCheck (const QVector< QByteArray > &prototype,
					const QVector< QByteArray > &arguments) {
	int i = 0;
	for (; i < arguments.length () && arguments.at (i) == prototype.at (i + 1); i++);
	return (i == arguments.length ());
}

Nuria::MetaMethod Nuria::MetaObject::method (const QVector< QByteArray > &prototype) {
	int lowerBound = methodLowerBound (prototype.first ());
	int upperBound = methodUpperBound (prototype.first ());
	
	// Found?
	if (lowerBound < 0) {
		return MetaMethod ();
	}
	
	// Not overloaded?
	if (lowerBound == upperBound) {
		QVector< QByteArray > args = MetaMethod (this, lowerBound).argumentTypes ();
		if (args.length () + 1 == prototype.length () &&
		    methodArgumentCheck (prototype, args)) {
			return MetaMethod (this, lowerBound);
			
		}
		
		return MetaMethod ();
	}
	
	// Argument count
	int argumentCount = prototype.length () - 1;
	int leastArguments = MetaMethod (this, lowerBound).argumentTypes ().length ();
	int maxArguments = MetaMethod (this, upperBound).argumentTypes ().length ();
	
	if (argumentCount < leastArguments || argumentCount > maxArguments) {
		return MetaMethod ();
	}
	
	// Test possible methods
	for (int i = lowerBound; i <= upperBound; i++) {
		QVector< QByteArray > args = MetaMethod (this, i).argumentTypes ();
		
		// Test argument count and types
		if (args.length () == argumentCount &&
		    methodArgumentCheck (prototype, args)) {
			return MetaMethod (this, i);
		}
		
	}
	
	// Not found.
	return MetaMethod ();
	
}

void Nuria::MetaObject::destroyInstance (void *instance) {
	gateCall (GateMethod::DestroyInstance, 0, 0, 0, nullptr, instance);
}

int Nuria::MetaObject::fieldCount () {
	int count = 0;
	gateCall (GateMethod::FieldCount, 0, 0, 0, &count);
	return count;
}

Nuria::MetaField Nuria::MetaObject::field (int idx) {
	return MetaField (this, idx);
}

Nuria::MetaField Nuria::MetaObject::fieldByName (const QByteArray &name) {
	auto compare = [&name](int i, MetaObject *obj) {
		return obj->field (i).name () < name;
	};
	
	int index = binaryFind (fieldCount (), this, compare);
	if (index < 0 || MetaField (this, index).name () != name) {
		return MetaField ();
	}
	
	return MetaField (this, index);
}

int Nuria::MetaObject::enumCount () {
	int count = 0;
	gateCall (GateMethod::EnumCount, 0, 0, 0, &count);
	return count;
}

Nuria::MetaEnum Nuria::MetaObject::enumAt (int idx) {
	return MetaEnum (this, idx);
}

Nuria::MetaEnum Nuria::MetaObject::enumByName (const QByteArray &name) {
	auto compare = [&name](int i, MetaObject *obj) {
		return MetaEnum (obj, i).name () < name;
	};
	
	int index = binaryFind (enumCount (), this, compare);
	if (index < 0 || MetaEnum (this, index).name () != name) {
		return MetaEnum ();
	}
	
	return MetaEnum (this, index);
}

Nuria::MetaAnnotation::MetaAnnotation ()
	: m_meta (nullptr), m_category (0), m_index (0), m_nth (0)
{
	
}

bool Nuria::MetaAnnotation::isValid () const {
	return (this->m_meta);
}

QByteArray Nuria::MetaAnnotation::name () const {
	QByteArray result;
	RETURN_CALL_GATE(MetaObject::GateMethod::AnnotationName, this->m_category,
			 this->m_index, this->m_nth);
}

QVariant Nuria::MetaAnnotation::value () const {
	QVariant result;
	RETURN_CALL_GATE(MetaObject::GateMethod::AnnotationValue, this->m_category,
			 this->m_index, this->m_nth);
}

Nuria::MetaMethod::MetaMethod ()
	: m_meta (nullptr), m_index (0)
{
	
}

bool Nuria::MetaMethod::isValid () const {
	return (this->m_meta);
}

QByteArray Nuria::MetaMethod::name () const {
	QByteArray result;
	RETURN_CALL_GATE(MetaObject::GateMethod::MethodName, 0, this->m_index, 0);
}

Nuria::MetaMethod::Type Nuria::MetaMethod::type () const {
	Type result = Method;
	RETURN_CALL_GATE(MetaObject::GateMethod::MethodType, 0, this->m_index, 0);
}

QByteArray Nuria::MetaMethod::returnType () const {
	QByteArray result;
	RETURN_CALL_GATE(MetaObject::GateMethod::MethodReturnType, 0, this->m_index, 0);
}

QVector< QByteArray > Nuria::MetaMethod::argumentTypes () const {
	QVector< QByteArray > result;
	RETURN_CALL_GATE(MetaObject::GateMethod::MethodArgumentTypes, 0, this->m_index, 0);
}

QVector< QByteArray > Nuria::MetaMethod::argumentNames () const {
	QVector< QByteArray > result;
	RETURN_CALL_GATE(MetaObject::GateMethod::MethodArgumentNames, 0, this->m_index, 0);
}

Nuria::Callback Nuria::MetaMethod::callback (void *instance) const {
	Callback result;
	if (!this->m_meta) {
		return result;
	}
	
	this->m_meta->gateCall (MetaObject::GateMethod::MethodCallback, 0,
				this->m_index, 0, &result, instance);
	return result;
}

Nuria::Callback Nuria::MetaMethod::unsafeCallback (void *instance) const {
	Callback result;
	if (!this->m_meta) {
		return result;
	}
	
	this->m_meta->gateCall (MetaObject::GateMethod::MethodUnsafeCallback,
				0, this->m_index, 0, &result, instance);
	return result;
}

Nuria::Callback Nuria::MetaMethod::testCallback (void *instance) const {
	Callback result;
	if (!this->m_meta) {
		return result;
	}
	
	this->m_meta->gateCall (MetaObject::GateMethod::MethodArgumentTest,
				0, this->m_index, 0, &result, instance);
	return result;
}

int Nuria::MetaMethod::annotationCount () const {
	int result = 0;
	RETURN_CALL_GATE(MetaObject::GateMethod::AnnotationCount, MethodCategory, this->m_index, 0);
}

Nuria::MetaAnnotation Nuria::MetaMethod::annotation (int idx) {
	if (!this->m_meta) {
		return MetaAnnotation ();
	}
	
	return MetaAnnotation (this->m_meta, MethodCategory, this->m_index, idx);
	
}

Nuria::MetaField::MetaField ()
	: m_meta (0), m_index (0)
{
	
}

bool Nuria::MetaField::isValid () const {
	return (this->m_meta);
}

QByteArray Nuria::MetaField::name () const {
	QByteArray result;
	RETURN_CALL_GATE(MetaObject::GateMethod::FieldName, 0, this->m_index, 0);
}

QByteArray Nuria::MetaField::typeName () const {
	QByteArray result;
	RETURN_CALL_GATE(MetaObject::GateMethod::FieldType, 0, this->m_index, 0);
}

Nuria::MetaField::Access Nuria::MetaField::access() const {
	MetaField::Access result = NoAccess;
	RETURN_CALL_GATE(MetaObject::GateMethod::FieldAccess, 0, this->m_index, 0);
}

QVariant Nuria::MetaField::read (void *instance) const {
	QVariant result;
	if (!this->m_meta) {
		return result;
	}
	
	this->m_meta->gateCall (MetaObject::GateMethod::FieldRead, 0,
				this->m_index, 0, &result, instance);
	return result;
}

bool Nuria::MetaField::write (void *instance, const QVariant &value) {
	bool result = false;
	if (!this->m_meta) {
		return result;
	}
	
	void *additional[] = { instance, const_cast< QVariant * > (&value) };
	this->m_meta->gateCall (MetaObject::GateMethod::FieldWrite, 0,
				this->m_index, 0, &result, additional);
	return result;
	
}

int Nuria::MetaField::annotationCount () const {
	int result = 0;
	RETURN_CALL_GATE(MetaObject::GateMethod::AnnotationCount, FieldCategory, this->m_index, 0);
}

Nuria::MetaAnnotation Nuria::MetaField::annotation (int idx) {
	if (!this->m_meta) {
		return MetaAnnotation ();
	}
	
	return MetaAnnotation (this->m_meta, FieldCategory, this->m_index, idx);
}


Nuria::MetaEnum::MetaEnum ()
	: m_meta (nullptr), m_index (0)
{
	
}

bool Nuria::MetaEnum::isValid () const {
	return (this->m_meta);
}

QByteArray Nuria::MetaEnum::name () const {
	QByteArray result;
	RETURN_CALL_GATE(MetaObject::GateMethod::EnumName, 0, this->m_index, 0);
}

int Nuria::MetaEnum::elementCount () const {
	int result = 0;
	RETURN_CALL_GATE(MetaObject::GateMethod::EnumElementCount, 0, this->m_index, 0);
}

QByteArray Nuria::MetaEnum::key (int at) const {
	QByteArray result;
	RETURN_CALL_GATE(MetaObject::GateMethod::EnumElementKey, 0, this->m_index, at);
}

int Nuria::MetaEnum::value (int at) const {
	int result = 0;
	RETURN_CALL_GATE(MetaObject::GateMethod::EnumElementValue, 0, this->m_index, at);
}

QByteArray Nuria::MetaEnum::valueToKey (int value) const {
	if (!this->m_meta) {
		return QByteArray ();
	}
	
	int count = elementCount ();
	for (int i = 0; i < count; i++) {
		if (this->value (i) == value) {
			return key (i);
		}
		
	}
	
	return QByteArray ();
}

int Nuria::MetaEnum::keyToValue (const QByteArray &key) const {
	if (!this->m_meta) {
		return -1;
	}
	
	int count = elementCount ();
	for (int i = 0; i < count; i++) {
		if (this->key (i) == key) {
			return value (i);
		}
		
	}
	
	return -1;
	
}

int Nuria::MetaEnum::annotationCount () const {
	int result = 0;
	RETURN_CALL_GATE(MetaObject::GateMethod::AnnotationCount, EnumCategory, this->m_index, 0);
}

Nuria::MetaAnnotation Nuria::MetaEnum::annotation (int idx) {
	if (!this->m_meta) {
		return MetaAnnotation ();
	}
	
	return MetaAnnotation (this->m_meta, EnumCategory, this->m_index, idx);
}
