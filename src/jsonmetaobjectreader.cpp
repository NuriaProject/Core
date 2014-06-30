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

#include "jsonmetaobjectreader.hpp"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>

#include "runtimemetaobject.hpp"

typedef QMap< QString, Nuria::MetaObjectMap > FileMetaObjectMap;

namespace Nuria {
class JsonMetaObjectReaderPrivate {
public:
	
	void clearData () {
		for (const Nuria::MetaObjectMap &metaObjects : objects) {
			qDeleteAll (metaObjects);
		}		
	}
	
	~JsonMetaObjectReaderPrivate () {
		clearData ();
		
	}
	
	FileMetaObjectMap objects;
	
};
}

// Invoke creator. JSON meta-data can't contain instantiators.
static Nuria::Callback invalidCallbackCreator (void *, Nuria::RuntimeMetaObject::InvokeAction) {
	return Nuria::Callback ();
}

// No-op getters and setters
static QVariant invalidGetter (void *) {
	return QVariant ();
}

static bool invalidSetter (void *, const QVariant &) {
	return false;
}

// 
Nuria::JsonMetaObjectReader::JsonMetaObjectReader (QObject *parent)
	: QObject (parent), d_ptr (new JsonMetaObjectReaderPrivate)
{
	
}

Nuria::JsonMetaObjectReader::~JsonMetaObjectReader () {
	// 
}

static Nuria::JsonMetaObjectReader::Error parseAnnotationsArray (const QJsonArray &array,
								 Nuria::RuntimeMetaObject::AnnotationMap &annotations) {
	for (int i = 0; i < array.size (); i++) {
		QJsonValue cur = array.at (i);
		if (!cur.isObject ()) {
			return Nuria::JsonMetaObjectReader::AnnotationIsNotAnObject;
		}
		
		// 
		QJsonObject obj = cur.toObject ();
		QJsonValue name = obj.value (QStringLiteral("name"));
		QJsonValue value = obj.value (QStringLiteral("value"));
		
		if (!name.isString ()) return Nuria::JsonMetaObjectReader::AnnotationNameIsNotAString;
		if (value.isNull ()) return Nuria::JsonMetaObjectReader::AnnotationValueHasUnknownType;
		
		annotations.insert (name.toString ().toUtf8 (), value.toVariant ());
		
	}
	
	// 
	return Nuria::JsonMetaObjectReader::NoError;
	
}

static Nuria::JsonMetaObjectReader::Error parseMethodArgumentList (const QJsonArray &names, const QJsonArray &types,
								   QVector< QByteArray > &outNames,
								   QVector< QByteArray > &outTypes) {
	if (names.size () != types.size ()) {
		return Nuria::JsonMetaObjectReader::MethodArgumentsHaveDifferentLengths;
	}
	
	// 
	for (int i = 0; i < names.size (); i++) {
		QJsonValue curName = names.at (i);
		QJsonValue curType = types.at (i);
		
		if (!curName.isString ()) return Nuria::JsonMetaObjectReader::MethodArgumentNamesContainsNonString;
		if (!curType.isString ()) return Nuria::JsonMetaObjectReader::MethodArgumentTypesContainsNonString;
		
		// 
		outNames.append (curName.toString ().toLatin1 ());
		outTypes.append (curType.toString ().toLatin1 ());
		
	}
	
	// 
	return Nuria::JsonMetaObjectReader::NoError;
	
}

static Nuria::JsonMetaObjectReader::Error parseMethodObject (Nuria::MetaMethod::Type type, const QJsonObject &method,
							     Nuria::RuntimeMetaObject *metaObject) {
	using namespace Nuria;
	
	QJsonValue annotationsValue = method.value (QStringLiteral("annotations"));
	QJsonValue argNamesValue = method.value (QStringLiteral("argumentNames"));
	QJsonValue argTypesValue = method.value (QStringLiteral("argumentTypes"));
	QJsonValue resultTypeValue = method.value (QStringLiteral("resultType"));
	QJsonValue nameValue = method.value (QStringLiteral("name"));
	
	// Type checks
	if (!annotationsValue.isArray ()) return JsonMetaObjectReader::AnnotationsIsNotAnArray;
	if (!argNamesValue.isArray ()) return JsonMetaObjectReader::MethodArgumentNamesIsNotAnArray;
	if (!argTypesValue.isArray ()) return JsonMetaObjectReader::MethodArgumentTypesIsNotAnArray;
	if (!resultTypeValue.isString ()) return JsonMetaObjectReader::MethodResultTypeIsNotAString;
	if (!nameValue.isString ()) return JsonMetaObjectReader::MethodNameIsNotAString;
	
	// Parse annotations
	Nuria::JsonMetaObjectReader::Error error;
	
	RuntimeMetaObject::AnnotationMap annotations;
	error = parseAnnotationsArray (annotationsValue.toArray (), annotations);
	if (error != Nuria::JsonMetaObjectReader::NoError) return error;
	
	// Arguments
	QVector< QByteArray > argNames;
	QVector< QByteArray > argTypes;
	
	error = parseMethodArgumentList (argNamesValue.toArray (), argTypesValue.toArray (), argNames, argTypes);
	if (error != Nuria::JsonMetaObjectReader::NoError) return error;
	
	// Base
	QByteArray name = nameValue.toString ().toLatin1 ();
	QByteArray resultType = resultTypeValue.toString ().toLatin1 ();
	
	// Store
	metaObject->addMethod (type, name, resultType, argNames, argTypes, annotations, invalidCallbackCreator);
	return Nuria::JsonMetaObjectReader::NoError;
}

static Nuria::JsonMetaObjectReader::Error parseMethodArray (Nuria::MetaMethod::Type type, const QJsonArray &methods,
							    Nuria::RuntimeMetaObject *metaObject) {
	Nuria::JsonMetaObjectReader::Error error;
	
	for (int i = 0; i < methods.size (); i++) {
	        QJsonValue value = methods.at (i);
		
		if (!value.isObject ()) {
			return Nuria::JsonMetaObjectReader::MethodIsNotAnObject;
		}
		
		// 
		error = parseMethodObject (type, value.toObject (), metaObject);
		if (error != Nuria::JsonMetaObjectReader::NoError) {
			return error;
		}
		
	}
	
	// 
	return Nuria::JsonMetaObjectReader::NoError;
	
}

static Nuria::JsonMetaObjectReader::Error parseEnumValues (const QJsonObject &obj, QMap< QByteArray, int > &map) {
	auto it = obj.constBegin ();
	auto end = obj.constEnd ();
	for (; it != end; ++it) {
		QString key = it.key ();
		QJsonValue jsonValue = it.value ();
		
		if (!jsonValue.isDouble ()) return Nuria::JsonMetaObjectReader::EnumValueObjectValueIsNotAnInteger;
		map.insert (key.toLatin1 (), jsonValue.toInt ());
		
	}
	
	// 
	return Nuria::JsonMetaObjectReader::NoError;
	
}

static Nuria::JsonMetaObjectReader::Error parseEnumObject (const QString &name, const QJsonObject &enumObj,
							   Nuria::RuntimeMetaObject *metaObject) {
	using namespace Nuria;
	
	QJsonValue annotationsValue = enumObj.value (QStringLiteral("annotations"));
	QJsonValue valuesValue = enumObj.value (QStringLiteral("values"));
	
	// Type check
	if (!annotationsValue.isArray ()) return JsonMetaObjectReader::AnnotationsIsNotAnArray;
	if (!valuesValue.isObject ()) return JsonMetaObjectReader::EnumValuesIsNotAnObject;
	
	// Parse annotations
	JsonMetaObjectReader::Error error;
	
	RuntimeMetaObject::AnnotationMap annotations;
	error = parseAnnotationsArray (annotationsValue.toArray (), annotations);
	if (error != JsonMetaObjectReader::NoError) return error;
	
	// Parse values
	QMap< QByteArray, int > keyValueMap;
	error = parseEnumValues (valuesValue.toObject (), keyValueMap);
	if (error != JsonMetaObjectReader::NoError) return error;
	
	// Store and done.
	metaObject->addEnum (name.toLatin1 (), annotations, keyValueMap);
	return JsonMetaObjectReader::NoError;
	
}

static Nuria::JsonMetaObjectReader::Error parseEnumsObject (const QJsonObject &enums,
							    Nuria::RuntimeMetaObject *metaObject) {
	Nuria::JsonMetaObjectReader::Error error = Nuria::JsonMetaObjectReader::NoError;
	auto it = enums.constBegin ();
	auto end = enums.constEnd ();
	for (; it != end && error == Nuria::JsonMetaObjectReader::NoError; ++it) {
		QString name = it.key ();
		QJsonValue value = *it;
		
		if (!value.isObject ()) return Nuria::JsonMetaObjectReader::EnumIsNotAnObject;
		error = parseEnumObject (name, value.toObject (), metaObject);
	}
	
	// 
	return error;
	
}

static Nuria::JsonMetaObjectReader::Error parseFieldObject (const QString &name, const QJsonObject &field,
							    Nuria::RuntimeMetaObject *metaObject) {
	using namespace Nuria;
	
	QJsonValue annotationsValue = field.value (QStringLiteral("annotations"));
	QJsonValue readOnlyValue = field.value (QStringLiteral("readOnly"));
	QJsonValue typeValue = field.value (QStringLiteral("type"));
	
	// Type check
	if (!annotationsValue.isArray ()) return JsonMetaObjectReader::AnnotationsIsNotAnArray;
	if (!readOnlyValue.isBool ()) return JsonMetaObjectReader::FieldReadOnlyIsNotABoolean;
	if (!typeValue.isString ()) return JsonMetaObjectReader::FieldTypeIsNotAString;
	
	// Parse annotations
	JsonMetaObjectReader::Error error;
	
	RuntimeMetaObject::AnnotationMap annotations;
	error = parseAnnotationsArray (annotationsValue.toArray (), annotations);
	if (error != JsonMetaObjectReader::NoError) return error;
	
	// Store and done.
	QByteArray typeName = typeValue.toString ().toLatin1 ();
	
	if (readOnlyValue.toBool ()) {
		metaObject->addField (name.toLatin1 (), typeName, annotations, invalidGetter);
	} else {
		metaObject->addField (name.toLatin1 (), typeName, annotations, invalidGetter, invalidSetter);
	}
	
	return JsonMetaObjectReader::NoError;
}

static Nuria::JsonMetaObjectReader::Error parseFieldsObject (const QJsonObject &fields,
							     Nuria::RuntimeMetaObject *metaObject) {
	Nuria::JsonMetaObjectReader::Error error = Nuria::JsonMetaObjectReader::NoError;
        auto it = fields.constBegin ();
        auto end = fields.constEnd ();
        for (; it != end && error == Nuria::JsonMetaObjectReader::NoError; ++it) {
                QString name = it.key ();
                QJsonValue value = *it;
                
                if (!value.isObject ()) return Nuria::JsonMetaObjectReader::FieldIsNotAnObject;
                error = parseFieldObject (name, value.toObject (), metaObject);
        }
        
        // 
        return error;
}

static Nuria::JsonMetaObjectReader::Error parseBasesArray (const QJsonArray &bases,
							   Nuria::RuntimeMetaObject *metaObject) {
	QVector< QByteArray > baseNames;
	
	for (const QJsonValue &cur : bases) {
		if (!cur.isString ()) return Nuria::JsonMetaObjectReader::BasesContainsNonString;
		baseNames.append (cur.toString ().toLatin1 ());
		
	}
	
	metaObject->setBaseClasses (baseNames);
	return Nuria::JsonMetaObjectReader::NoError;
	
}

static Nuria::JsonMetaObjectReader::Error parseTypeObject (const QByteArray &typeName, const QJsonObject &type,
							   Nuria::MetaObjectMap &objects) {
	using namespace Nuria;
	
	QJsonValue annotationsValue = type.value (QStringLiteral("annotations"));
	QJsonValue basesValue = type.value (QStringLiteral("bases"));
	QJsonValue memberMethodsValue = type.value (QStringLiteral("memberMethods"));
	QJsonValue staticMethodsValue = type.value (QStringLiteral("staticMethods"));
	QJsonValue constructorsValue = type.value (QStringLiteral("constructors"));
	QJsonValue enumsValue = type.value (QStringLiteral("enums"));
	QJsonValue fieldsValue = type.value (QStringLiteral("fields"));
	
	// Type checks
	if (!annotationsValue.isArray ()) return JsonMetaObjectReader::AnnotationsIsNotAnArray;
	if (!basesValue.isArray ()) return JsonMetaObjectReader::BasesIsNotAnArray;
	if (!memberMethodsValue.isArray ()) return JsonMetaObjectReader::MemberMethodsIsNotAnArray;
	if (!staticMethodsValue.isArray ()) return JsonMetaObjectReader::StaticMethodsIsNotAnArray;
	if (!constructorsValue.isArray ()) return JsonMetaObjectReader::ConstructorsIsNotAnArray;
	if (!enumsValue.isObject ()) return JsonMetaObjectReader::EnumsIsNotAnObject;
	if (!fieldsValue.isObject ()) return JsonMetaObjectReader::FieldsIsNotAnObject;
	
	// Create meta object
	JsonMetaObjectReader::Error error;
	RuntimeMetaObject *metaObject = new RuntimeMetaObject (typeName);
	
	// Parse bases
	error = parseBasesArray (basesValue.toArray (), metaObject);
	if (error != Nuria::JsonMetaObjectReader::NoError) return error;
	
	// Parse annotations
	RuntimeMetaObject::AnnotationMap annotations;
	error = parseAnnotationsArray (annotationsValue.toArray (), annotations);
	if (error != Nuria::JsonMetaObjectReader::NoError) return error;
	
	// Parse methods
	JsonMetaObjectReader::Error errorMembers;
	JsonMetaObjectReader::Error errorStatics;
	JsonMetaObjectReader::Error errorCtors;
	
	errorMembers = parseMethodArray (MetaMethod::Method, memberMethodsValue.toArray (), metaObject);
	errorStatics = parseMethodArray (MetaMethod::Static, staticMethodsValue.toArray (), metaObject);
	errorCtors = parseMethodArray (MetaMethod::Constructor, constructorsValue.toArray (), metaObject);
	
	if (errorMembers != JsonMetaObjectReader::NoError) return errorMembers;
	if (errorStatics != JsonMetaObjectReader::NoError) return errorStatics;
	if (errorCtors != JsonMetaObjectReader::NoError) return errorCtors;
	
	// Parse enums
	error = parseEnumsObject (enumsValue.toObject (), metaObject);
	if (error != Nuria::JsonMetaObjectReader::NoError) return error;
	
	// Parse fields
	error = parseFieldsObject (fieldsValue.toObject (), metaObject);
	if (error != Nuria::JsonMetaObjectReader::NoError) return error;
	
	// Store and done.
	metaObject->setAnnotations (annotations);
	metaObject->finalize ();
	
	objects.insert (typeName, metaObject);
	return Nuria::JsonMetaObjectReader::NoError;
}

static Nuria::JsonMetaObjectReader::Error parseTypesObject (const QJsonObject &types, Nuria::MetaObjectMap &objects) {
	Nuria::JsonMetaObjectReader::Error error = Nuria::JsonMetaObjectReader::NoError;
	
	auto it = types.constBegin ();
	auto end = types.constEnd ();
	for (; it != end && error == Nuria::JsonMetaObjectReader::NoError; ++it) {
		QString name = it.key ();
		QJsonValue typeValue = *it;
		
		if (!typeValue.isObject ()) return Nuria::JsonMetaObjectReader::TypeIsNotAnObject;
		error = parseTypeObject (name.toLatin1 (), typeValue.toObject (), objects);
		
	}
	
	// 
	return error;
}

Nuria::JsonMetaObjectReader::Error Nuria::JsonMetaObjectReader::parse (const QJsonObject &root) {
	Error error = NoError;
	
	auto it = root.constBegin ();
	auto end = root.constEnd ();
	for (; it != end && error == NoError; ++it) {
		QString fileName = it.key ();
		QJsonValue fileValue = *it;
		MetaObjectMap metaObjectMap;
		
		if (!fileValue.isObject ()) {
			error = FileIsNotAnObject;
			break;
		}
		
		error = parseTypesObject (fileValue.toObject (), metaObjectMap);
		
		// Store
		this->d_ptr->objects.insert (fileName, metaObjectMap);
		
	}
	
	// 
	if (error != NoError) {
		this->d_ptr->clearData ();
	}
	
	return error;
	
}

Nuria::JsonMetaObjectReader::Error Nuria::JsonMetaObjectReader::parse (const QJsonDocument &jsonDocument) {
	
	if (!jsonDocument.isObject ()) {
		return RootIsNotAnObject;
	}
	
	// 
	return parse (jsonDocument.object ());
	
}

Nuria::JsonMetaObjectReader::Error Nuria::JsonMetaObjectReader::parse (const QByteArray &jsonData) {
	
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson (jsonData, &error);
	
	if (error.error != QJsonParseError::NoError) {
		return JsonParseError;
	}
	
	// 
	return parse (doc);
}

QStringList Nuria::JsonMetaObjectReader::sourceFiles () {
	return this->d_ptr->objects.keys ();
}

Nuria::MetaObjectMap Nuria::JsonMetaObjectReader::metaObjects (const QString &sourceFile) {
	return this->d_ptr->objects.value (sourceFile);
}
