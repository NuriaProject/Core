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

#include "serializer.hpp"

#include <QMetaMethod>
#include "debug.hpp"

// 
static QMap< QString, int > variantMapToTypeMap (const QVariantMap &data) {
	QMap< QString, int > map;
	
	auto it = data.constBegin ();
	auto end = data.constEnd ();
	for (; it != end; ++it) {
		map.insert (it.key (), it->userType ());
	}
	
	return map;
}

static inline bool isQObject (const char *name) {
	return QMetaType (QMetaType::type (name)).flags () & QMetaType::PointerToQObject;
}

static inline bool isQObject (int typeId) {
	return QMetaType (typeId).flags () & QMetaType::PointerToQObject;
}

static bool constructorMatch (const QMetaMethod &method, const QMap< QString, int > &typeMap, bool recursive) {
	
	// Test data types
	QList< QByteArray > names = method.parameterNames ();
        int argCount = method.parameterCount ();
	
	for (int i = 0; i < argCount; i++) {
		int type = typeMap.value (names.at (i), -1);
		if (type != method.parameterType (i) && !recursive && !isQObject (type)) {
			return false;
		}
		
	}
	
	return true;
}

static QMetaMethod findConstructor (const QMetaObject *meta, const QMap< QString, int > &typeMap, bool recursive) {
	QMetaMethod bestMatch;
	
	if (meta->constructorCount () == 0) {
		nWarn() << "There's no constructor available for class" << meta->className ();
	}
	
	for (int i = 0; i < meta->constructorCount (); i++) {
		QMetaMethod cur = meta->constructor (i);
		
		int curArgs = cur.parameterCount ();
		int bestArgs = bestMatch.parameterCount ();
		
		if ((!bestMatch.isValid () || curArgs > bestArgs) &&
		    constructorMatch (cur, typeMap, recursive)) {
			bestMatch = cur;
		}
		
	}
	
	return bestMatch;
}

static QVariant variantFromSerializedData (int targetType, const QVariant &data, int recursion) {
	if (targetType == -1 || data.userType () == targetType) {
		return data;
	}
	
	// QObject?
	if (recursion != 0 && data.userType () == QMetaType::QVariantMap && isQObject (targetType)) {
		const QMetaObject *meta = QMetaType::metaObjectForType (targetType);
		QObject *deserialized = Nuria::Serializer::objectFromMap (meta, data.toMap (), recursion - 1);
		return QVariant (targetType, &deserialized);
	}
	
	// Failed
	return QVariant ();
}

static void prepareGenericArguments (QGenericArgument *arr, QVariantList &argList,
				     const QMetaMethod &method, QVariantMap &args,
				     int recursion) {
	
	QList< QByteArray > names = method.parameterNames ();
	for (int i = 0; i < method.parameterCount (); i++) {
		int typeId = method.parameterType (i);
		const char *typeName = QMetaType::typeName (typeId);
		auto it = args.find (names.at (i));
		
		argList.append (variantFromSerializedData (typeId, *it, recursion));
		arr[i] = QGenericArgument (typeName, argList.at (i).constData ());
		
		args.erase (it);
	}
	
}

static inline bool hasMetaInfo (const QMetaObject *meta, const char *classInfoName) {
	for (int i = QObject::staticMetaObject.classInfoCount (); i < meta->classInfoCount (); i++) {
		QMetaClassInfo info (meta->classInfo (i));
		if (!::strcmp (info.name (), classInfoName)) {
			return true;
		}
		
	}
	
	return false;
}

static inline void findExcludedProperties (const QString &prefix, const QMetaObject *meta, QStringList &list) {
	int i = QObject::staticMetaObject.propertyCount ();
	for (; i < meta->propertyCount (); i++) {
		QMetaProperty cur = meta->property (i);
		QString name (QLatin1String (cur.name ()));
		QString metaInfoName = prefix + name;
		
		if (hasMetaInfo (meta, qPrintable(metaInfoName))) {
			list.append (name);
		}
		
	}
	
}

static int findTypeOfProperty (const QMetaObject *meta, const char *name) {
	for (int i = QObject::staticMetaObject.propertyCount (); i < meta->propertyCount (); i++) {
		QMetaProperty cur (meta->property (i));
		if (!::strcmp (cur.name (), name)) {
			return cur.userType ();
		}
		
	}
	
	return -1;
}

static void applyPropertiesFromMap (QObject *obj, const QVariantMap &map, int recursion,
				    QStringList *failedProperties) {
	auto it = map.constBegin ();
	auto end = map.constEnd ();
	
	for (; it != end; ++it) {
		// Defer type of property
		int targetType = -1;
		if (it->userType () == QMetaType::QVariantMap) {
			targetType = findTypeOfProperty (obj->metaObject (), qPrintable(it.key ()));
		}
		
		// Deserialize
		QVariant value = variantFromSerializedData (targetType, *it, recursion);
		if (!obj->setProperty (qPrintable(it.key ()), value) && failedProperties) {
			failedProperties->append (it.key ());
		}
		
	}
	
}

static QObject *createObject (const QMetaObject *meta, QVariantMap &args, int recursion) {

	// Find constructor
	QMetaMethod ctor = findConstructor (meta, variantMapToTypeMap (args), recursion != 0);
	
//	qDebug() << "Chosen constructor:" << ctor.name () << ctor.parameterNames () << ctor.parameterTypes ();
	if (!ctor.isValid ()) {
	        return nullptr;
	}
	
	// 
	QObject *result = nullptr;
	QVariantList tempArgumentList;
	QGenericArgument rawArgs[10];
	
	// Create arguments and create instance
	prepareGenericArguments (rawArgs, tempArgumentList, ctor, args, recursion);
	result = meta->newInstance (rawArgs[0], rawArgs[1], rawArgs[2],
			rawArgs[3], rawArgs[4], rawArgs[5], rawArgs[6],
			rawArgs[7], rawArgs[8], rawArgs[9]);
	
	// Done.
	return result;
}

QObject *Nuria::Serializer::objectFromMap (const QMetaObject *meta, const QVariantMap &data,
					   int recursion, const QStringList &exclude,
					   QStringList *failedProperties) {
	static const QString metaInfoPrefix = QStringLiteral("objectFromMap.exclude.");
	QVariantMap args (data);
	QStringList excluded (exclude);
	
	// Remove excluded properties
	findExcludedProperties (metaInfoPrefix, meta, excluded);
	for (const QString &removeMe : excluded) {
		args.remove (removeMe);
	}
	
	// Create instance.
	QObject *obj = createObject (meta, args, recursion);
	if (!obj) {
		return nullptr;
	}
	
	// Set additional properties
	applyPropertiesFromMap (obj, args, recursion, failedProperties);
	
	// Done
	return obj;
}

QVariantMap Nuria::Serializer::objectToMap (const QObject *obj, int recursion, const QStringList &exclude) {
	static const QString metaInfoPrefix = QStringLiteral("objectToMap.exclude.");
	const QMetaObject *meta = obj->metaObject ();
	QStringList excluded (exclude);
	QVariantMap map;
	
	// 
	findExcludedProperties (metaInfoPrefix, meta, excluded);
	
	// Iterate over properties, leaving out QObject's
	int i = QObject::staticMetaObject.propertyCount ();
	for (; i < meta->propertyCount (); i++) {
		QMetaProperty cur = meta->property (i);
		
		// Take name and check if it's skipped
		QString name (QLatin1String (cur.name ()));
		if (excluded.contains (name)) {
			continue;
		}
		
		// Is it a pointer?
		QVariant value = obj->property (cur.name ());
		if (::strchr (value.typeName (), '*')) {
			QObject *child = value.value< QObject * > ();
			if (recursion != 0 && child && isQObject (cur.userType ())) {
				value = objectToMap (child, recursion - 1, exclude);
			} else {
				continue;
			}
			
		}
		
		// Store
		map.insert (name, value);
		
	}
	
	return map;
}
