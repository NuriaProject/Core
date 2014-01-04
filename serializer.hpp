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

#ifndef NURIA_SERIALIZER_HPP
#define NURIA_SERIALIZER_HPP

#include "essentials.hpp"
#include <QStringList>
#include <QVariant>
#include <QObject>

namespace Nuria {

/**
 * \brief (De-)Serializer for QObject sub-classes.
 * 
 * This class offers methods to easily (de-)serialize QObject based structures
 * from and to QVariantMaps, which can be e.g. (de-)serialized to JSON or other
 * data structures.
 * 
 * \par Usage
 * This class makes heavy use of the Qt meta system. When using custom QObject
 * subclasses it's therefore necessary to register the \b pointer using the
 * Q_DECLARE_METATYPE macro and also qRegisterMetaType.
 * 
 * Additionally, all exposed variables must be accessible through properties.
 * When possible, Serializer::objectFromMap will use a constructor with as many
 * matching arguments as possible. A matching constructor is one, where all
 * arguments are available in the given data and are of the same type.
 * 
 * \note To expose a constructor you have to prefix it with Q_INVOKABLE.
 * 
 * \warning A type which is not correctly registered will lead to undefined
 * behaviour!
 * 
 * \par Example
 * \code
 * // Sample class
 * class MyClass : public QObject {
 *   Q_OBJECT
 *   Q_PROPERTY(int foo READ foo WRITE setFoo)
 * public:
 *   Q_INVOKABLE (int foo);
 *   // ...
 * };
 * 
 * // Register the pointer type
 * Q_DECLARE_METATYPE(MyClass*)
 * 
 * // Serializing and deserializing a instance
 * qRegisterMetaType< MyClass * > ();
 * QVariantMap serialized = Nuria::Serializer::objectToMap (myClassInstance);
 * QObject *myDeserializedInstance = Nuria::Serializer::objectFromMap (serialized);
 * \endcode
 * 
 */
class NURIA_CORE_EXPORT Serializer {
public:
	/*
	enum ConversionPolicy {
		NoConversion,
		QVariantConversion,
		NuriaConversion
	};
	*/
	
	/**
	 * Creates a QObject of type \a meta from \a data.
	 * Keys in \a data are interpreted to be properties in the QObject.
	 * First, a suitable constructor is searched. A constructor is deemed
	 * suitable if all argument name are present in \a data and the value
	 * types match (i.e. the data type of the value in \a data is the same
	 * as the type of the argument). The constructor with the most matches
	 * is chosen. 
	 * All key value pairs from \a data which were not already passed to the
	 * constructor will be set using QObject::setProperty().
	 * 
	 * Keys in \a exclude are ignored. It's also possible to always ignore a
	 * certain property by creating a Q_CLASSINFO named
	 * "objectFromMap.exclude.<Property>". \a failedProperties contains a
	 * list of keys which were not able to be set (For example if the
	 * datatype didn't match). 
	 * 
	 * If a property has a QObject-based type, the method can recurse into
	 * the structure when finding it in the given map. For this to work, the
	 * value for this property must be stored as QVariantMap inside \a data
	 * and \a recursion must be unequal to zero. A negative value for
	 * \a recursion is equal to infinite.
	 * 
	 * If all attempts to create a instance failed, \c nullptr is returned.
	 * 
	 * \note For this to work, there needs to be at least one constructor
	 * which is marked as Q_INVOKABLE.
	 * 
	 * \note To get a value for \a meta use <YourType>::staticMetaObject.
	 */
	static QObject *objectFromMap (const QMetaObject *meta, const QVariantMap &data,
				       int recursion = 0, /*ConversionPolicy conversion = NoConversion,*/
				       const QStringList &exclude = QStringList(),
				       QStringList *failedProperties = nullptr);
	
	/**
	 * Reads all properties from \a obj and puts them into a QVariantMap.
	 * Properties can be excluded by either using the \a exclude string list
	 * or by creating a Q_CLASSINFO named "objectToMap.exclude.<Property>".
	 * If the name of a property is found in either of those, it will \b not
	 * be exported.
	 * 
	 * If a property is a pointer which is a QObject, the method will
	 * recurse into it until \a recursion is \c 0, in which case the
	 * property is silently skipped. The default value for \a recursion is
	 * \c 0, thus by default the method will not recurse.
	 * 
	 * \note A value of recursion lower than \c 0 is considered to be
	 * unlimited.
	 * 
	* \note When recursing, \a exclude is passed on.
	 */
	static  QVariantMap objectToMap (const QObject *obj, int recursion = 0,
					 const QStringList &exclude = QStringList());
	
private:
	Serializer () = delete;
	
};

}

#endif // NURIA_SERIALIZER_HPP
