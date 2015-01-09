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

#include "nuria/qtmetaobjectwrapper.hpp"

#include <QMetaMethod>
#include <cstring>
#include <QVector>

Nuria::QtMetaObjectWrapper::QtMetaObjectWrapper (const QMetaObject *metaObject)
	: RuntimeMetaObject (metaObject->className ())
{
	
	installDeleter ();
	populateAnnotations (metaObject);
	populateMethods (metaObject);
	populateEnums (metaObject);
	populateFields (metaObject);
	
	const QMetaObject *base = metaObject->superClass ();
	if (base) {
		setBaseClasses ({ base->className () });
	}
	
	finalize ();
	
}

Nuria::QtMetaObjectWrapper::~QtMetaObjectWrapper () {
	// 
}

void Nuria::QtMetaObjectWrapper::installDeleter () {
	
	auto deleter = [](void *inst) {
		delete reinterpret_cast< QObject * > (inst);
	};
	
	setInstanceDeleter (deleter);
	
}

void Nuria::QtMetaObjectWrapper::populateAnnotations (const QMetaObject *meta) {
	AnnotationMap map;
	
	for (int i = meta->classInfoOffset (); i < meta->classInfoCount (); i++) {
		QMetaClassInfo info = meta->classInfo (i);
		map.insert (info.name (), QString (info.value ()));
	}
	
	setAnnotations (map);
	
}

static bool returnTrue (void *) { return true; }

static Nuria::RuntimeMetaObject::InvokeCreator methodCreator (QMetaMethod m) {
	QByteArray name = m.methodSignature ();
	QByteArray slot (name.length () + 2, 0x0);
	
	slot[0] = '1'; // Ignored by Nuria::Callback
	std::memcpy (slot.data () + 1, name.constData (), name.size ());
	
	auto creator = [slot](void *inst, Nuria::RuntimeMetaObject::InvokeAction act) {
		if (act == Nuria::RuntimeMetaObject::InvokeAction::ArgumentTest) {
			return Nuria::Callback (returnTrue);
		}
		
		return Nuria::Callback ((QObject *)inst, slot.constData ());
	};
	
	return creator;
}

static void registerMethod (Nuria::RuntimeMetaObject *meta, QMetaMethod m) {
	using namespace Nuria;
	
	QByteArray returnType (QMetaType::typeName (m.returnType ()), -1);
        QList< QByteArray > pNames = m.parameterNames ();
	QList< QByteArray > pTypes = m.parameterTypes ();
	
	// 
	QVector< QByteArray > argNames (pNames.length ());
	QVector< QByteArray > argTypes (pNames.length ());
	
	for (int i = 0; i < pNames.length (); i++) {
		argNames[i] = pNames.at (i);
		argTypes[i] = pTypes.at (i);
	}
	
	// 
	auto creator = methodCreator (m);
	meta->addMethod (MetaMethod::Method, m.name (), returnType, argNames, argTypes, { }, creator);
	
}

void Nuria::QtMetaObjectWrapper::populateMethods (const QMetaObject *meta) {
	
	for (int i = meta->methodOffset (); i < meta->methodCount (); i++) {
		QMetaMethod m = meta->method (i);
		
		if (m.access () == QMetaMethod::Public && m.methodType () != QMetaMethod::Signal) {
			registerMethod (this, m);
		}
		
	}
	
}

static void registerEnum (Nuria::RuntimeMetaObject *meta, QMetaEnum e) {
	QMap< QByteArray, int > keyValue;
	for (int i = 0; i < e.keyCount (); i++) {
		keyValue.insert (e.key (i), e.value (i));
	}
	
	// 
	QByteArray name (e.name (), -1);
	meta->addEnum (name, { }, keyValue);
	
}

void Nuria::QtMetaObjectWrapper::populateEnums (const QMetaObject *meta) {
	
	for (int i = meta->enumeratorOffset (); i < meta->enumeratorCount (); i++) {
		registerEnum (this, meta->enumerator (i));
	}
	
}

static void registerProperty (Nuria::RuntimeMetaObject *meta, QMetaProperty p) {
	QByteArray name (p.name (), -1);
	QByteArray type (p.typeName (), -1);
	
	// 
	const char *rawName = p.name ();
	auto getter = [rawName](void *inst) {
		return reinterpret_cast< QObject * > (inst)->property (rawName);
	};
	
	// 
	if (p.isWritable ()) {
		auto setter = [rawName](void *inst, const QVariant &v) {
			return reinterpret_cast< QObject * > (inst)->setProperty (rawName, v);
		};
		
		meta->addField (name, type, { }, getter, setter);
	} else {
		meta->addField (name, type, { }, getter);
	}
	
}

void Nuria::QtMetaObjectWrapper::populateFields (const QMetaObject *meta) {
	
	for (int i = meta->propertyOffset (); i < meta->propertyCount (); i++) {
		registerProperty (this, meta->property (i));
	}
	
}
