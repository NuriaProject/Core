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
#include "debug.hpp"

namespace Nuria {
class SerializerPrivate {
public:
	
	Serializer::InstanceCreator factory;
	Serializer::MetaObjectFinder finder;
	
	QVector< QByteArray > excluded;
	QVector< QByteArray > additionalTypes;
	QStringList failed;
	int maxDepth = Serializer::NoRecursion;
	
	int curDepth = 0;
	
};

}

Nuria::Serializer::Serializer (MetaObjectFinder metaObjectFinder, InstanceCreator instanceCreator)
	: d (new SerializerPrivate)
{
	
	this->d->factory = instanceCreator;
	this->d->finder = metaObjectFinder;
	
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
	case QMetaType::QString:
	case QMetaType::QStringList:
	case QMetaType::QVariantList:
	case QMetaType::QVariantMap:
		return true;
	}
	
	return false;
}

// Steals the pointer to an object from 'variant'
static void *stealPointer (QVariant &variant) {
	QVariant::Private &data = variant.data_ptr ();
	void *result = data.data.ptr;
	
	if (data.is_shared) {
		nCritical() << "The variant" << variant
			    << "must NOT contain a non-pointer type!! - Aborting";
		abort ();
	}
	
	data.is_shared = false;
	data.is_null = true;
	data.type = QVariant::Invalid;
	data.data.ptr = nullptr;
	
	return result;
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

bool Nuria::Serializer::variantToField (QVariant &value, const QByteArray &targetType,
					int targetId, int sourceId, int pointerId,
					bool &ignored) {
	
	if (sourceId == QMetaType::QVariantMap) {
		if (this->d->curDepth == 1) {
			ignored = true;
			return true;
		}
		
		// 
		MetaObject *meta = this->d->finder (targetType);
		QVariantMap data = value.toMap ();
		void *obj = this->d->factory (meta, data);
		
		if (meta && populateImpl (obj, meta, data)) {
			putObjectIntoVariant (value, obj, targetId, pointerId);
			return true;
		}
		
	}
	
	// Try Variant::convert() which internally triggers QVariant conversion
	QVariant result = Variant::convert (value, targetId);
	if (result.isValid ()) {
		value.swap (result);
		return true;
	}
	
	return false;
}

bool Nuria::Serializer::fieldToVariant (QVariant &value, bool &ignore) {
	QByteArray typeName = QByteArray (value.typeName (), -1);
	MetaObject *meta = this->d->finder (typeName);
	
	if (meta) {
		void *dataPtr = value.data ();
		
		if (typeName.endsWith ('*')) {
			dataPtr = *reinterpret_cast< void ** > (dataPtr);
		}
		
		if (this->d->curDepth == 1) {
			ignore = true;
		} else {
			value = serializeImpl (dataPtr, meta);
		}
		
		return true;
	}
	
	// Variant::convert() triggers QVariant conversion internally
	QVariant conv = Nuria::Variant::convert (value, QMetaType::QString);
	if (conv.isValid ()) {
		value = conv;
		return true;
	}
	
	return false;
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
	if (fieldToVariant (value, ignore)) {
		
		if (!ignore) {
			data.insert (name, value);
		}
		
		return true;
	}
	
	return false;
}

bool Nuria::Serializer::writeField (void *object, Nuria::MetaField &field, const QVariantMap &data) {
	QVariant value = data.value (QString::fromLatin1 (field.name ()));
	QByteArray typeName = field.typeName ();
	bool isPointer = typeName.endsWith ('*');
	bool ignored = false;
	int pointerId = 0;
	
	if (isPointer) {
		pointerId = QMetaType::type (typeName.constData ());
		typeName.chop (1);
	}
	
	int sourceId = value.userType ();
	int targetId = QMetaType::type (typeName.constData ());
	
	if (sourceId != targetId) {
		variantToField (value, typeName, targetId, sourceId, pointerId, ignored);
		
		if (ignored) {
			return true;
		}
		
	}
	
	if ((isPointer && value.isValid ()) || value.userType () == targetId) {
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
	return this->d->failed.isEmpty ();
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

void *Nuria::Serializer::defaultInstanceCreator (Nuria::MetaObject *metaObject, QVariantMap &data) {
	Q_UNUSED(data);
	
	MetaMethod ctor = metaObject->method (QVector< QByteArray > { QByteArray () });
	
	if (!ctor.isValid ()) {
		return nullptr;
	}
	
	QVariant instance = ctor.callback () ();
	return stealPointer (instance);
}
