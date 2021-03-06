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

#include "nuria/serializer.hpp"
#include "nuria/variant.hpp"
#include <QVector>

namespace Nuria {
class SerializerPrivate {
public:
	
	Serializer::InstanceCreator factory;
	Serializer::MetaObjectFinder finder;
	Serializer::ValueConverter converter;
	
	QVector< QByteArray > excluded;
	QVector< QByteArray > additionalTypes;
	QStringList failed;
	int maxDepth = Serializer::NoRecursion;
	
	int curDepth = 0;
	
};

}

Nuria::Serializer::Serializer (MetaObjectFinder metaObjectFinder, InstanceCreator instanceCreator,
                               ValueConverter valueConverter)
	: d (new SerializerPrivate)
{
	
	this->d->factory = instanceCreator;
	this->d->finder = metaObjectFinder;
	this->d->converter = valueConverter;
	
}

Nuria::Serializer::~Serializer () {
	delete this->d;
}

QVector< QByteArray > Nuria::Serializer::exclude () const {
	return this->d->excluded;
}

void Nuria::Serializer::setExclude (const QVector< QByteArray > &list) {
	this->d->excluded = list;
	std::sort (this->d->excluded.begin (), this->d->excluded.end ());
}

QVector< QByteArray > Nuria::Serializer::allowedTypes () const {
	return this->d->additionalTypes;
}

void Nuria::Serializer::setAllowedTypes (const QVector< QByteArray > &list) const {
	this->d->additionalTypes = list;
}

QStringList Nuria::Serializer::failedFields () const {
	return this->d->failed;
}

int Nuria::Serializer::recursionDepth () const {
	return this->d->maxDepth;
}

void Nuria::Serializer::setRecursionDepth (int maxDepth) {
	this->d->maxDepth = maxDepth;
}

void *Nuria::Serializer::deserialize (const QVariantMap &data, Nuria::MetaObject *meta) {
	
	QVariantMap fields = data;
	void *instance = this->d->factory (meta, fields);
	
	if (!instance) {
		return nullptr;
	}
	
	// 
	populate (instance, meta, fields);
	return instance;
	
}

void *Nuria::Serializer::deserialize (const QVariantMap &data, const QByteArray &typeName) {
	MetaObject *meta = this->d->finder (typeName);
	
	if (!meta) {
		return nullptr;
	}
	
	return deserialize (data, meta);
}

static bool isAllowedType (int id) {
	switch (id) {
	case QMetaType::Bool:
	case QMetaType::Int:
	case QMetaType::Float:
	case QMetaType::Double:
	case QMetaType::LongLong:
	case QMetaType::ULongLong:
	case QMetaType::UInt:
	case QMetaType::QByteArray:
	case QMetaType::QString:
	case QMetaType::QStringList:
	case QMetaType::QVariantList:
	case QMetaType::QVariantMap:
		return true;
	}
	
	return false;
}

// Puts 'object' into 'variant', moving ownership without copying any data.
// If 'pointerId' is not zero, then the pointer to the object will be stored
// instead.
void putObjectIntoVariant (QVariant &variant, void *object, int typeId, int pointerId) {
	if (pointerId) {
		variant = QVariant (pointerId, &object, true);
		return;
	}
	
	// 
	variant.clear ();
	
	QVariant::DataPtr &data = variant.data_ptr ();
	data.is_shared = true;
	data.is_null = false;
	data.type = typeId;
	data.data.shared = new QVariant::PrivateShared (object);
	
}

// 
static bool castVariant (QVariant &value, int targetType, bool toPointer) {
	if (toPointer) {
		return false;
	}
	
	// We can cast T* -> T, but with the current API, not T -> T*,
	// as the value we need to point to needs to be owned by someone
	// for it to be freed later ..
	// TODO: Embed the casting more deeply for less overhead and more options
	
	void *ptr = Nuria::Variant::getPointer (value);
	value = QVariant (targetType, ptr); // Copy constructor
	return value.isValid ();
}

// Check for T -> T* or T* -> T casts
static bool tryPointerConversion (QVariant &value, int sourceId, int targetId, const QByteArray &targetType) {
	const char *sourceNamePtr = QMetaType::typeName (sourceId);
	QByteArray sourceType = QByteArray::fromRawData (sourceNamePtr, strlen (sourceNamePtr));
	
	bool sourcePtr = sourceType.endsWith ('*');
	bool targetPtr = targetType.endsWith ('*');
	
	// Preliminary check if both are be 'T' length-wise
	if (sourcePtr != targetPtr &&
	    sourceType.length () - sourcePtr == targetType.length () - targetPtr) {
		int len = std::min (sourceType.length (), targetType.length ());
		if (!::memcmp (sourceType.constData (), targetType.constData (), len)) {
			return castVariant (value, targetId, targetPtr);
		}
		
	}
	
	// 
	return false;
}

bool Nuria::Serializer::variantToField (QVariant &value, const QByteArray &targetType,
					int targetId, int sourceId, int pointerId,
					bool &ignored) {
	
	if (sourceId == QMetaType::QVariantMap) {
		if (this->d->curDepth == 1) {
			ignored = true;
			return false;
		}
		
		// 
		MetaObject *meta = this->d->finder (targetType);
		QVariantMap data = value.toMap ();
		void *obj = this->d->factory (meta, data);
		
		if (meta && populateImpl (obj, meta, data)) {
			putObjectIntoVariant (value, obj, targetId, pointerId);
			return true;
		}
		
	} else if (tryPointerConversion (value, sourceId, targetId, targetType)) {
		return true;
	}
	
	// Convert using the user converter
	return this->d->converter (value, targetId);
}

bool Nuria::Serializer::fieldToVariant (QVariant &value, bool &ignore) {
	QByteArray typeName = QByteArray (value.typeName ());
	MetaObject *meta = this->d->finder (typeName);
	
	if (meta) {
		void *dataPtr = value.data ();
		
		if (typeName.endsWith ('*')) {
			dataPtr = *reinterpret_cast< void ** > (dataPtr);
		}
		
		if (this->d->curDepth == 1) {
			ignore = true;
			return false;
		}
		
		value = serializeImpl (dataPtr, meta);
		return true;
	}
	
	// Convert using the user converter
	return this->d->converter (value, QMetaType::QString);
}

bool Nuria::Serializer::readField (void *object, Nuria::MetaField &field, QVariantMap &data) {
	QVariant value = field.read (object);
	QString name = QString::fromLatin1 (field.name ());
	
	if (isAllowedType (value.userType ()) ||
	    this->d->additionalTypes.contains (field.typeName ())) {
		data.insert (name, value);
		return true;
	}
	
	bool ignore = false;
	if (!fieldToVariant (value, ignore)) {
		return ignore;
	}
	
	data.insert (name, value);
	return true;
}

bool Nuria::Serializer::writeField (void *object, Nuria::MetaField &field, const QVariantMap &data) {
	QVariant value = data.value (QString::fromLatin1 (field.name ()));
	QByteArray typeName = field.typeName ();
	bool isPointer = typeName.endsWith ('*');
	int sourceId = value.userType ();
	bool ignored = false;
	int pointerId = 0;
	
	if (!value.isValid ()) {
		return true;
	}
	
	if (isPointer) {
		pointerId = QMetaType::type (typeName.constData ());
		typeName.chop (1);
		
		// Ignore the field if it's a pointer type and the value is 'invalid'.
		if (!value.isValid () || sourceId == pointerId) {
			return field.write (object, value);
		}
		
	}
	
	int targetId = QMetaType::type (typeName.constData ());
	if (sourceId == QMetaType::UnknownType || targetId == QMetaType::UnknownType) {
		return false;
	}
	
	if (sourceId != targetId && targetId != QMetaType::QVariant) {
		if (!variantToField (value, typeName, targetId, sourceId, pointerId, ignored)) {
			return ignored;
		}
		
		// 
		sourceId = targetId;
	}
	
	if ((isPointer && value.isValid ()) || sourceId == targetId || targetId == QMetaType::QVariant) {
		return field.write (object, value);
	}
	
	// 
	return false;
	
}

bool Nuria::Serializer::populate (void *object, Nuria::MetaObject *meta, const QVariantMap &data) {
	this->d->failed.clear ();
	this->d->curDepth = this->d->maxDepth + 2;
	
	return populateImpl (object, meta, data);
}

bool Nuria::Serializer::populateImpl (void *object, Nuria::MetaObject *meta, const QVariantMap &data) {
	this->d->curDepth--;
	int failedCount = this->d->failed.length ();
	
	if (!this->d->curDepth) {
		this->d->curDepth++;
		return false;
	}
	
	// 
	int fields = meta->fieldCount ();
	for (int i = 0; i < fields; i++) {
		MetaField field = meta->field (i);
		
		bool ignore = std::binary_search (this->d->excluded.constBegin (),
						  this->d->excluded.constEnd (),
						  field.name ());
		
		if (!ignore && !writeField (object, field, data)) {
			this->d->failed.append (field.name ());
		}
		
	}
	
	this->d->curDepth++;
	return (this->d->failed.length () == failedCount);
}

bool Nuria::Serializer::populate (void *object, const QByteArray &typeName, const QVariantMap &data) {
	MetaObject *meta = this->d->finder (typeName);
	
	if (!meta) {
		return false;
	}
	
	return populate (object, meta, data);
}

QVariantMap Nuria::Serializer::serialize (void *object, Nuria::MetaObject *meta) {
	this->d->failed.clear ();
	this->d->curDepth = this->d->maxDepth + 2;
	
	return serializeImpl (object, meta);
}

QVariantMap Nuria::Serializer::serializeImpl (void *object, Nuria::MetaObject *meta) {
	QVariantMap map;
	this->d->curDepth--;
	
	if (!this->d->curDepth) {
		this->d->curDepth++;
		return map;
	}
	
	// 
	int fields = meta->fieldCount ();
	for (int i = 0; i < fields; i++) {
		MetaField field = meta->field (i);
		
		bool ignore = std::binary_search (this->d->excluded.constBegin (),
						  this->d->excluded.constEnd (),
						  field.name ());
		
		if (!ignore && !readField (object, field, map)) {
			this->d->failed.append (field.name ());
		}
		
	}
	
	// 
	this->d->curDepth++;
	return map;
}

QVariantMap Nuria::Serializer::serialize (void *object, const QByteArray &typeName) {
	MetaObject *meta = this->d->finder (typeName);
	
	if (!meta) {
		return QVariantMap ();
	}
	
	return serialize (object, meta);
}

Nuria::MetaObject *Nuria::Serializer::defaultMetaObjectFinder (const QByteArray &typeName) {
	MetaObject *meta = MetaObject::byName (typeName);
	
	if (!meta && typeName.endsWith ('*')) {
		QByteArray objType = typeName.left (typeName.length () - 1);
		return MetaObject::byName (objType);
	}
	
	return meta;
}

template< typename Result, typename Map >
static Result mapFind (Map &map, const QByteArray &key) {
	auto end = map.end ();
	for (auto it = map.begin (); it != end; ++it) {
		if (it.key () == key) return it;
	}
	
	return end;
}

static inline bool mapContainsKey (const QVariantMap &map, const QByteArray &key) {
	return (mapFind< QVariantMap::const_iterator > (map, key) != map.end ());
}

static bool checkCtor (int idx, Nuria::MetaObject *meta, const QVariantMap &data) {
	Nuria::MetaMethod ctor = meta->method (idx);
	QVector< QByteArray > args = ctor.argumentNames ();
	
	// Skip copy constructors
	int count = args.length ();
	if (count == 1 && ctor.argumentTypes ().first () == meta->className ()) {
		return false;
	}
	
	// Check ..
	int i = 0;
	for (; i < count && mapContainsKey (data, args.at (i)); i++);
	return (i == count);
}

static QVariantList getConstructorArguments (const QVector< QByteArray > &names, QVariantMap &data) {
	QVariantList list;
	list.reserve (names.length ());
	
	// Transfer all 'names' elements from 'data' into 'list'.
	for (int i = 0, count = names.length (); i < count; i++) {
		auto it = mapFind< QVariantMap::iterator > (data, names.at (i));
		list.append (*it);
		data.erase (it);
	}
	
	return list;
}

void *Nuria::Serializer::defaultInstanceCreator (MetaObject *metaObject, QVariantMap &data) {
	int first = metaObject->methodLowerBound (QByteArray ());
	int last = metaObject->methodUpperBound (QByteArray ());
	
	if (first == -1) {
		return nullptr;
	}
	
	// Find constructor
	int i;
	for (i = last; i >= first && !checkCtor (i, metaObject, data); i--);
	
	// Nothing found?
	if (i < first) {
		return nullptr;
	}
	
	// 
	MetaMethod ctor = metaObject->method (i);
	QVector< QByteArray > names = ctor.argumentNames ();
	QVariant instance = ctor.callback ().invoke (getConstructorArguments (names, data));
	return Variant::stealPointer (instance);
}

bool Nuria::Serializer::defaultValueConverter (QVariant &variant, int toType) {
	return variant.convert (toType);
}

