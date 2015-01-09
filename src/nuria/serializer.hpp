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

#ifndef NURIA_SERIALIZER_HPP
#define NURIA_SERIALIZER_HPP

#include "essentials.hpp"
#include "metaobject.hpp"
#include <QStringList>
#include <QVariant>
#include <QObject>

namespace Nuria {

class SerializerPrivate;

/**
 * \brief (De-)Serializer for arbitary types based on Nuria::MetaObject.
 * 
 * This class offers methods to easily (de-)serialize structures from and to
 * QVariantMaps, which can be e.g. (de-)serialized to JSON or other data
 * structures.
 * 
 * \par Usage
 * This class makes heavy use of the Nuria meta system. Therefore it's necessary
 * to have Nuria::MetaObjects for all involved types. Though the only thing you
 * really need to do is using Tria (Or Nuria::QtMetaObjectWrapper as fallback).
 * 
 * \par Deserialization flow
 * When a fields type does not match the one in the variant map, the following
 * steps in the specified order are tried to resolve it:
 * 
 * - If it's a QVariantMap, then it'll be tried to find a MetaObject for the
 *   type of the field and tried to deserialize.
 * - The converter is invoked. The default one uses QVariant::convert().
 * 
 * If all above steps fail, it'll be noted in the failed list.
 * \sa failedFields
 * 
 * \note Tria will automatically try to register conversions through the
 * generated code.
 * 
 * \par Serialization flow
 * For serialization, the following steps are tried:
 * 
 * - If the field type is a POD number type, QString, QVariantList or
 *   QVariantMap, it'll be used.
 * - If the field type is in the allowed types vector, it'll be used.
 * - If the field type is known to the Nuria meta system, serialize() will
 * recurse into it
 * - The converter is invoked to convert it to a QString. The default one uses
 * QVariant::convert().
 * 
 * If all above steps fail, it'll be noted in the failed list.
 * \sa failedFields
 * 
 */
class NURIA_CORE_EXPORT Serializer {
public:
	
	enum RecursionDepth {
		NoRecursion = 0,
		InfiniteRecursion = -3
	};
	
	/**
	 * Prototype for a method which returns the Nuria::MetaObject for a
	 * named type. If no type can be found for that name, \c nullptr should
	 * be returned instead.
	 */
	typedef std::function< MetaObject *(const QByteArray & /* typeName */) > MetaObjectFinder;
	
	/**
	 * Prototype for a method which uses the two arguments \a metaObject
	 * and \a data to return a new instance of \a metaObject. \a data may
	 * be manipulated by the method freely if so desired (E.g. to filter
	 * fields or to add or remove fields for other reasons).
	 * 
	 * If creation fails, \c nullptr should be returned to indicate an
	 * error.
	 */
	typedef std::function< void *(MetaObject * /* metaObject */, QVariantMap & /* data */) > InstanceCreator;
	
	/**
	 * Prototype for the value converter in the deserialization process.
	 * The passed variant is to be converted to the Qt type id as given in
	 * the passed integer. Return \c true on success.
	 */
	typedef std::function< bool(QVariant & /* variant */, int /* toType */) > ValueConverter;
	
	/**
	 * Constructor.
	 */
	Serializer (MetaObjectFinder metaObjectFinder = defaultMetaObjectFinder,
		    InstanceCreator instanceCreator = defaultInstanceCreator,
	            ValueConverter valueConverter = defaultValueConverter);
	
	/** Destructor. */
	~Serializer ();
	
	/**
	 * Returns a list of excluded fields. The default list is empty.
	 */
	QVector< QByteArray > exclude () const;
	
	/**
	 * Sets the list of excluded fields. When (de-)serializing, all fields
	 * whose names match (case-sensitive) a entry in \a list will be
	 * ignored.
	 */
	void setExclude (const QVector< QByteArray > &list);
	
	/**
	 * Returns the list of allowed data types for serialization.
	 * The default list is empty.
	 */
	QVector< QByteArray > allowedTypes () const;
	
	/**
	 * Sets the list of additional allowed types for serialization.
	 */
	void setAllowedTypes (const QVector< QByteArray > &list) const;
	
	/**
	 * Returns a list of fields which failed to (de-)serialize.
	 */
	QStringList failedFields () const;
	
	/**
	 * Returns the maximum recursion depth for (de-)serialization. Default
	 * value is \c NoRecursion (i.e. \c 0).
	 */
	int recursionDepth () const;
	
	/**
	 * Sets the maximum recursion depth.
	 */
	void setRecursionDepth (int maxDepth);
	
	/**
	 * Creates a instance of type \a meta from \a data.
	 * Elements in \a data are interpreted to be fields in the object.
	 * Elements of \a data which are not fields in \a object are silently
	 * ignored.
	 * 
	 * Fields that are of a custom type (E.g. not a POD-type or a Qt type)
	 * will be deserialized too if the used MetaObjectFinder and
	 * InstanceCreator can handle the type.
	 * 
	 * If all attempts to create a instance failed, \c nullptr is returned.
	 */
	void *deserialize (const QVariantMap &data, MetaObject *meta);
	
	/** \overload */
	void *deserialize (const QVariantMap &data, const QByteArray &typeName);
	
	/** \overload Works for types registered to the Qt meta sytem. */
	template< typename T >
	T *deserialize (const QVariantMap &data) {
		MetaObject *meta = MetaObject::of< T > ();
		return meta ? reinterpret_cast< T * > (deserialize (data, meta)) : nullptr;
	}
	
	/**
	 * Like deserialize, but instead takes a existing \a object and
	 * populates it with \a data. Returns \c true if no elements
	 * failed to deserialize.
	 */
	bool populate (void *object, MetaObject *meta, const QVariantMap &data);
	
	/** \overload */
	bool populate (void *object, const QByteArray &typeName, const QVariantMap &data);
	
	/** \overload Works for types registered to the Qt meta sytem. */
	template< typename T >
	bool populate (T *object, const QVariantMap &data) {
		MetaObject *meta = MetaObject::of< T > ();
		return meta ? populate (object, meta, data) : false;
	}
	
	/**
	 * Reads all fields from \a object and puts them into a QVariantMap.
	 * Fields can be excluded by using \a exclude.
	 * 
	 * If a field is a pointer which is a object known to , the method will
	 * recurse into it until \a recursion is \c 0, in which case the
	 * property is silently skipped. The default value for \a recursion is
	 * \c 0, thus by default the method will not recurse.
	 * 
	 * \note A value of recursion lower than \c 0 is considered to be
	 * unlimited.
	 * 
	* \note When recursing, \a exclude is passed on.
	 */
	QVariantMap serialize (void *object, MetaObject *meta);
	
	/** \overload */
	QVariantMap serialize (void *object, const QByteArray &typeName);
	
	/**
	 * Default meta object finder. \a typeName is expected to be known to
	 * the Nuria meta system.
	 */
	static MetaObject *defaultMetaObjectFinder (const QByteArray &typeName);
	
	/**
	 * Default instance creator. It will try to find the constructor of the
	 * type with the most arguments which have a key-value-pair in \a data.
	 */
	static void *defaultInstanceCreator (MetaObject *metaObject, QVariantMap &data);
	
	/**
	 * Default value converter. Tries QVariant::convert() to convert
	 * \a variant to \a toType.
	 */
	static bool defaultValueConverter (QVariant &variant, int toType);
	
private:
	SerializerPrivate *d;
	
	QVariantMap serializeImpl (void *object, MetaObject *meta);
	bool populateImpl (void *object, MetaObject *meta, const QVariantMap &data);
	bool variantToField (QVariant &value, const QByteArray &targetType,
			     int targetId, int sourceId, int pointerId, bool &ignored);
	bool fieldToVariant (QVariant &value, bool &ignore);
	bool readField (void *object, Nuria::MetaField &field, QVariantMap &data);
	bool writeField (void *object, Nuria::MetaField &field, const QVariantMap &data);
};

}

#endif // NURIA_SERIALIZER_HPP
