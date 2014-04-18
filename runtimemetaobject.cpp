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

#include "runtimemetaobject.hpp"

#include <algorithm>
#include <QMultiMap>
#include <QVector>

enum Categories {
	ObjectCategory = 0,
	MethodCategory = 1,
	FieldCategory = 2,
	EnumCategory = 3
};

struct MethodData {
	Nuria::MetaMethod::Type type;
	Nuria::RuntimeMetaObject::AnnotationMap annotations;
	QByteArray name;
	QByteArray returnType;
	QVector< QByteArray > argNames;
	QVector< QByteArray > argTypes;
	Nuria::RuntimeMetaObject::InvokeCreator creator;
};

struct EnumData {
	Nuria::RuntimeMetaObject::AnnotationMap annotations;
	QMap< QByteArray, int > elements;
};

struct FieldData {
	Nuria::MetaField::Access access;
	QByteArray valueType;
	Nuria::RuntimeMetaObject::AnnotationMap annotations;
	Nuria::RuntimeMetaObject::FieldGetter getter;
	Nuria::RuntimeMetaObject::FieldSetter setter;
};

namespace Nuria {
class RuntimeMetaObjectPrivate {
public:
	QByteArray className;
	int valueTypeId = 0;
	int poinerTypeId = 0;
	
	QVector< QByteArray > bases;
	RuntimeMetaObject::InstanceDeleter deleter;
	
	QVector< MethodData > methods;
	QMap< QByteArray, EnumData > enums;
	QMap< QByteArray, FieldData > fields;
	
	RuntimeMetaObject::AnnotationMap annotations;
};

}

static void defaultInstanceDeleter (void *) {}
static bool defaultFieldSetter (void *, const QVariant &) { return false; }

Nuria::RuntimeMetaObject::RuntimeMetaObject (const QByteArray &name)
	: d (new RuntimeMetaObjectPrivate)
{
	
	this->d->className = name;
	this->d->deleter = defaultInstanceDeleter;
	
}

Nuria::RuntimeMetaObject::~RuntimeMetaObject () {
	delete this->d;
}

void Nuria::RuntimeMetaObject::setQtMetaTypeId (int valueTypeId) {
	this->d->valueTypeId = valueTypeId;
}

void Nuria::RuntimeMetaObject::setQtMetaTypePointerId (int pointerTypeId) {
	this->d->poinerTypeId = pointerTypeId;
}

void Nuria::RuntimeMetaObject::setAnnotations (const AnnotationMap &annotations) {
	this->d->annotations = annotations;
}

void Nuria::RuntimeMetaObject::setBaseClasses (const QVector< QByteArray > &bases) {
	this->d->bases = bases;
}

void Nuria::RuntimeMetaObject::setInstanceDeleter (Nuria::RuntimeMetaObject::InstanceDeleter deleter) {
	this->d->deleter = deleter;
}

void Nuria::RuntimeMetaObject::addMethod (Nuria::MetaMethod::Type type, const QByteArray &name,
					  const QByteArray &returnType, const QVector< QByteArray > &argumentNames,
					  const QVector< QByteArray > &argumentTypes, const AnnotationMap &annotations,
					  Nuria::RuntimeMetaObject::InvokeCreator invokeCreator) {
	MethodData data;
	data.type = type;
	data.annotations = annotations;
	data.name = name;
	data.returnType = returnType;
	data.argNames = argumentNames;
	data.argTypes = argumentTypes;
	data.creator = invokeCreator;
	
	this->d->methods.append (data);
}

void Nuria::RuntimeMetaObject::addEnum (const QByteArray &name, const AnnotationMap &annotations,
					const QMap< QByteArray, int > &keyValueMap) {
	EnumData data;
	data.annotations = annotations;
	data.elements = keyValueMap;
	
	this->d->enums.insert (name, data);
	
}

void Nuria::RuntimeMetaObject::addField (const QByteArray &name, const QByteArray &valueType,
					 const AnnotationMap &annotations, Nuria::RuntimeMetaObject::FieldGetter getter,
					 Nuria::RuntimeMetaObject::FieldSetter setter) {
	FieldData data;
	data.access = MetaField::ReadWrite;
	data.valueType = valueType;
	data.annotations = annotations;
	data.getter = getter;
	data.setter = setter;
	
	this->d->fields.insert (name, data);
	
}

void Nuria::RuntimeMetaObject::addField (const QByteArray &name, const QByteArray &valueType,
					 const AnnotationMap &annotations,
					 Nuria::RuntimeMetaObject::FieldGetter getter) {
	FieldData data;
        data.access = MetaField::ReadOnly;
        data.valueType = valueType;
        data.annotations = annotations;
        data.getter = getter;
        data.setter = defaultFieldSetter;
        
        this->d->fields.insert (name, data);
	
}

static bool methodLess (const MethodData &lhs, const MethodData &rhs) {
	if (lhs.name == rhs.name) {
		return lhs.argTypes.length () < rhs.argTypes.length ();
	}
	
	return lhs.name < rhs.name;
}

void Nuria::RuntimeMetaObject::finalize () {
	
	// Sort methods (See MetaObject for details)
	std::sort (this->d->methods.begin (), this->d->methods.end (), methodLess);
	
	// Sort bases
	std::sort (this->d->bases.begin (), this->d->bases.end ());
	
}

// This macro makes gateCall() a whole lot easier to read.
#define RESULT(Type) *reinterpret_cast< Type * > (result)
void Nuria::RuntimeMetaObject::gateCall (GateMethod method, int category, int index, int nth,
					 void *result, void *additional) {
	switch (method) {
	case Nuria::MetaObject::GateMethod::ClassName:
		RESULT(QByteArray) = this->d->className;
		break;
		
	case Nuria::MetaObject::GateMethod::MetaTypeId:
		RESULT(int) = this->d->valueTypeId;
		break;
		
	case Nuria::MetaObject::GateMethod::PointerMetaTypeId:
		RESULT(int) = this->d->poinerTypeId;
		break;
		
	case Nuria::MetaObject::GateMethod::BaseClasses:
		RESULT(QVector< QByteArray >) = this->d->bases;
		break;
		
	case Nuria::MetaObject::GateMethod::AnnotationCount:
		RESULT(int) = runtimeAnnotationCount (category, index);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodCount:
		RESULT(int) = this->d->methods.size ();
		break;
		
	case Nuria::MetaObject::GateMethod::FieldCount:
		RESULT(int) = this->d->fields.size ();
		break;
		
	case Nuria::MetaObject::GateMethod::EnumCount:
		RESULT(int) = this->d->enums.size ();
		break;
		
	case Nuria::MetaObject::GateMethod::AnnotationName:
		RESULT(QByteArray) = runtimeAnnotationName (category, index, nth);
		break;
		
	case Nuria::MetaObject::GateMethod::AnnotationValue:
		RESULT(QVariant) = runtimeAnnotationValue (category, index, nth);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodName:
		RESULT(QByteArray) = runtimeMethodName (index);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodType:
		RESULT(Nuria::MetaMethod::Type) = runtimeMethodType (index);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodReturnType:
		RESULT(QByteArray) = runtimeMethodReturnType (index);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodArgumentNames:
		RESULT(QVector< QByteArray >) = runtimeMethodArgumentNames (index);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodArgumentTypes:
		RESULT(QVector< QByteArray >) = runtimeMethodArgumentTypes (index);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodCallback:
		RESULT(Nuria::Callback) = runtimeMethodCallback (additional, index);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodUnsafeCallback:
		RESULT(Nuria::Callback) = runtimeMethodUnsafeCallback (additional, index);
		break;
		
	case Nuria::MetaObject::GateMethod::MethodArgumentTest:
		RESULT(Nuria::Callback) = runtimeMethodArgumentTest (additional, index);
		break;
		
	case Nuria::MetaObject::GateMethod::FieldName:
		RESULT(QByteArray) = runtimeFieldName (index);
		break;
		
	case Nuria::MetaObject::GateMethod::FieldType:
		RESULT(QByteArray) = runtimeFieldType (index);
		break;
		
	case Nuria::MetaObject::GateMethod::FieldRead:
		RESULT(QVariant) = runtimeFieldRead (index, additional);
		break;
		
	case Nuria::MetaObject::GateMethod::FieldWrite: {
		void **argData = reinterpret_cast< void ** > (additional);
		const QVariant &value = *reinterpret_cast< QVariant * > (argData[1]);
		RESULT(bool) = runtimeFieldWrite (index, argData[0], value);
	} break;
		
	case Nuria::MetaObject::GateMethod::FieldAccess:
		RESULT(Nuria::MetaField::Access) = runtimeFieldAccess (index);
		break;
		
	case Nuria::MetaObject::GateMethod::EnumName:
		RESULT(QByteArray) = runtimeEnumName (index);
		break;
		
	case Nuria::MetaObject::GateMethod::EnumElementCount:
		RESULT(int) = runtimeEnumElementCount (index);
		break;
		
	case Nuria::MetaObject::GateMethod::EnumElementKey:
		RESULT(QByteArray) = runtimeEnumElementKey (index, nth);
		break;
		
	case Nuria::MetaObject::GateMethod::EnumElementValue:
		RESULT(int) = runtimeEnumElementValue (index, nth);
		break;
		
	case Nuria::MetaObject::GateMethod::DestroyInstance:
		this->d->deleter (additional);
		break;
		
	}
	
}

// Helper macros. Accessors for methods, fields and enums with out-of-bounds
// check.
#define BOUNDS_CHECK(Container, Index) (Index >= 0 && Index < Container.size ())
#define METHOD_ACCESS(Index, OnFail, OnSuccess) \
	if (Index < 0 || index >= this->d->methods.size ()) { \
	return OnFail; \
	} \
	return this->d->methods.at (Index).OnSuccess

#define GENERIC_ACCESS(Map, Index, OnFail, OnSuccess) \
	if (Index < 0 || index >= this->d->Map.size ()) { \
		return OnFail; \
	} \
	return (this->d->Map.constBegin () + Index)->OnSuccess

int Nuria::RuntimeMetaObject::runtimeAnnotationCount (int category, int index) {
	switch (Categories (category)) {
	case ObjectCategory:
		return this->d->annotations.size ();
	case MethodCategory:
		METHOD_ACCESS(index, 0, annotations.size ());
	case FieldCategory:
		GENERIC_ACCESS(fields, index, 0, annotations.size ());
	case EnumCategory:
		GENERIC_ACCESS(enums, index, 0, annotations.size ());
	}
	
	return 0;
}

QByteArray Nuria::RuntimeMetaObject::runtimeAnnotationName (int category, int index, int nth) {
	switch (Categories (category)) {
	case ObjectCategory:
		return BOUNDS_CHECK(this->d->annotations, nth)
				? (this->d->annotations.constBegin () + nth).key ()
				: QByteArray ();
	case MethodCategory:
		if (BOUNDS_CHECK(this->d->methods, index)) {
			const MethodData &cur = this->d->methods.at (index);
			if (BOUNDS_CHECK(cur.annotations, nth)) {
				return (cur.annotations.constBegin () + nth).key ();
			}
			
		}
		break;
	case FieldCategory:
		if (BOUNDS_CHECK(this->d->fields, index)) {
			const FieldData &cur = *(this->d->fields.constBegin () + index);
			if (BOUNDS_CHECK(cur.annotations, nth)) {
				return (cur.annotations.constBegin () + nth).key ();
			}
			
		}
		break;
	case EnumCategory:
		if (BOUNDS_CHECK(this->d->enums, index)) {
			const EnumData &cur = *(this->d->enums.constBegin () + index);
			if (BOUNDS_CHECK(cur.annotations, nth)) {
				return (cur.annotations.constBegin () + nth).key ();
			}
			
		}
		break;
	}
	
	return QByteArray ();
}

QVariant Nuria::RuntimeMetaObject::runtimeAnnotationValue (int category, int index, int nth) {
	switch (Categories (category)) {
	case ObjectCategory:
		return BOUNDS_CHECK(this->d->annotations, nth)
				? *(this->d->annotations.constBegin () + nth)
				: QVariant ();
	case MethodCategory:
		if (BOUNDS_CHECK(this->d->methods, index)) {
			const MethodData &cur = this->d->methods.at (index);
			if (BOUNDS_CHECK(cur.annotations, nth)) {
				return *(cur.annotations.constBegin () + nth);
			}
			
		}
		break;
	case FieldCategory:
		if (BOUNDS_CHECK(this->d->fields, index)) {
			const FieldData &cur = *(this->d->fields.constBegin () + index);
			if (BOUNDS_CHECK(cur.annotations, nth)) {
				return *(cur.annotations.constBegin () + nth);
			}
			
		}
		break;
	case EnumCategory:
		if (BOUNDS_CHECK(this->d->enums, index)) {
			const EnumData &cur = *(this->d->enums.constBegin () + index);
			if (BOUNDS_CHECK(cur.annotations, nth)) {
				return *(cur.annotations.constBegin () + nth);
			}
			
		}
		break;
	}
	
	return QVariant ();
}

QByteArray Nuria::RuntimeMetaObject::runtimeMethodName (int index) {
	METHOD_ACCESS(index, QByteArray (), name);
}

Nuria::MetaMethod::Type Nuria::RuntimeMetaObject::runtimeMethodType (int index) {
	METHOD_ACCESS(index, MetaMethod::Method, type);
}

QByteArray Nuria::RuntimeMetaObject::runtimeMethodReturnType (int index) {
	METHOD_ACCESS(index, QByteArray (), returnType);
}

QVector< QByteArray > Nuria::RuntimeMetaObject::runtimeMethodArgumentNames (int index) {
	METHOD_ACCESS(index, QVector< QByteArray > (), argNames);
}

QVector< QByteArray > Nuria::RuntimeMetaObject::runtimeMethodArgumentTypes (int index) {
	METHOD_ACCESS(index, QVector< QByteArray > (), argTypes);
}

Nuria::Callback Nuria::RuntimeMetaObject::runtimeMethodCallback (void *instance, int index) {
	METHOD_ACCESS(index, Callback (), creator (instance, InvokeAction::Invoke));
}

Nuria::Callback Nuria::RuntimeMetaObject::runtimeMethodUnsafeCallback (void *instance, int index) {
	METHOD_ACCESS(index, Callback (), creator (instance, InvokeAction::UnsafeInvoke));
}

Nuria::Callback Nuria::RuntimeMetaObject::runtimeMethodArgumentTest (void *instance, int index) {
	METHOD_ACCESS(index, Callback (), creator (instance, InvokeAction::ArgumentTest));
}

QByteArray Nuria::RuntimeMetaObject::runtimeFieldName (int index) {
	return BOUNDS_CHECK(this->d->fields, index)
			? (this->d->fields.constBegin () + index).key ()
			: QByteArray ();
}

QByteArray Nuria::RuntimeMetaObject::runtimeFieldType (int index) {
	GENERIC_ACCESS(fields, index, QByteArray (), valueType);
}

QVariant Nuria::RuntimeMetaObject::runtimeFieldRead (int index, void *instance) {
	GENERIC_ACCESS(fields, index, QVariant (), getter (instance));
}

bool Nuria::RuntimeMetaObject::runtimeFieldWrite (int index, void *instance, const QVariant &value) {
	GENERIC_ACCESS(fields, index, false, setter (instance, value));
}

Nuria::MetaField::Access Nuria::RuntimeMetaObject::runtimeFieldAccess (int index) {
	GENERIC_ACCESS(fields, index, MetaField::NoAccess, access);
}

QByteArray Nuria::RuntimeMetaObject::runtimeEnumName (int index) {
	return BOUNDS_CHECK(this->d->enums, index)
			? (this->d->enums.constBegin () + index).key ()
			: QByteArray ();
}

int Nuria::RuntimeMetaObject::runtimeEnumElementCount (int index) {
	GENERIC_ACCESS(enums, index, 0, elements.size ());
}

QByteArray Nuria::RuntimeMetaObject::runtimeEnumElementKey (int index, int nth) {
	if (!BOUNDS_CHECK(this->d->enums, index)) {
		return QByteArray ();
	}
	
	const EnumData &cur = *(this->d->enums.constBegin () + index);
	return BOUNDS_CHECK(cur.elements, nth)
			? (cur.elements.constBegin () + nth).key ()
			: QByteArray ();
}

int Nuria::RuntimeMetaObject::runtimeEnumElementValue (int index, int nth) {
	if (!BOUNDS_CHECK(this->d->enums, index)) {
		return 0;
	}
	
	const EnumData &cur = *(this->d->enums.constBegin () + index);
	return BOUNDS_CHECK(cur.elements, nth)
			? *(cur.elements.constBegin () + nth)
			: 0;
}
