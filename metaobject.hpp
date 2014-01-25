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

#ifndef NURIA_METAOBJECT_HPP
#define NURIA_METAOBJECT_HPP

#include "essentials.hpp"
#include "callback.hpp"

#include <QVariant>
#include <QString>

namespace Nuria {

class MetaObject;
class MetaMethod;
class MetaField;
class MetaEnum;

/**
 * \brief The MetaAnnotation class allows access to annotations.
 * 
 * Annotations can be used to store user-defined meta-data for classes,
 * enums and methods.
 * \sa MetaObject::annotation MetaMethod::annotation
 * 
 * For an example see MetaObject.
 */
class NURIA_CORE_EXPORT MetaAnnotation {
public:
	
	/** Returns a invalid instance. */
	MetaAnnotation ();
	
	/** Returns \c true if this instance is valid. */
	bool isValid () const;
	
	/** Returns the name of the annotation. */
	QByteArray name () const;
	
	/** Returns the value of the annotation. */
	QVariant value () const;
	
private:
	friend class MetaObject;
	friend class MetaMethod;
	friend class MetaField;
	friend class MetaEnum;
	
	inline MetaAnnotation (MetaObject *meta, short category, short index, short nth)
		: m_meta (meta), m_category (category), m_index (index), m_nth (nth)
	{}
	
	mutable MetaObject *m_meta;
	short m_category;
	short m_index;
	short m_nth;
};

/**
 * \brief The MetaMethod class lets you access methods from registered types.
 * 
 */
class NURIA_CORE_EXPORT MetaMethod {
public:
	
	/** Different types of methods. */
	enum Type {
		
		/** Member method. \b Requires a valid instance. */
		Method = 0,
		
		/** Static method. */
		Static = 1,
		
		/** Constructor. */
		Constructor = 2
	};
	
	/** Creates a invalid instance. */
	MetaMethod ();
	
	/** Returns \c true if this instance is valid. */
	bool isValid () const;
	
	/** Returns the name of the method. */
	QByteArray name () const;
	
	/** Returns the type of this method. */
	Type type () const;
	
	/** Returns the result type name. */
	QByteArray returnType () const;
	
	/** Returns a list of all argument types. */
	QVector< QByteArray > argumentTypes () const;
	
	/** Returns a list of all argument names. */
	QVector< QByteArray > argumentNames () const;
	
	/**
	 * Returns a callback of this method. Invoking it will call the
	 * method itself. If type() is \c Method, then \a instance \b must point
	 * to a valid instance of the type this instance is pointing to. Not
	 * doing so will lead to undefined behaviour and probably crash the
	 * application.
	 * 
	 * The returned callback will also validate arguments if the method was
	 * annotated using NURIA_REQUIRE.
	 * 
	 * \sa Callback unsafeCallback
	 */
	Callback callback (void *instance = nullptr) const;
	
	/**
	 * Returns a callback pointing to the method. The only difference to
	 * callback() is that this method will \b not validate any arguments.
	 * 
	 * Use with care.
	 * \sa callback
	 */
	Callback unsafeCallback (void *instance = nullptr) const;
	
	/**
	 * Returns a callback which returns \c true when calling with arguments
	 * which would pass NURIA_REQUIRE tests. If the method doesn't have
	 * a NURIA_REQUIRE annotation, the returned callback is invalid.
	 */
	Callback testCallback (void *instance = nullptr) const;
	
	/**
	 * Returns the number of known annotations.
	 * \sa annotation
	 */
	int annotationCount () const;
	
	/**
	 * Returns the MetaAnnotation instance for the annotation at \a idx.
	 * \sa annotationCount
	 */
	MetaAnnotation annotation (int idx);	
	
private:
	friend class MetaObject;
	
	inline MetaMethod (MetaObject *meta, int index)
		: m_meta (meta), m_index (index)
	{}
	
	mutable MetaObject *m_meta;
	int m_index;
	
};

/**
 * \brief The MetaField class lets you access fields of types.
 * 
 * \par Access
 * This class allows read and write access. This will work on NURIA_FIELDs
 * and on 'pure' fields, that is, fields which are public but don't have
 * explicit getters or setters associated. In the latter case Tria will
 * auto-generate these functions.
 * 
 */
class NURIA_CORE_EXPORT MetaField {
public:
	/** Access right flags. */
	enum Access {
		NoAccess = 0x00,
		ReadOnly = 0x01,
		WriteOnly = 0x02,
		ReadWrite = ReadOnly | WriteOnly
	};
	
	/** Creates a invalid instance. */
	MetaField ();
	
	/** Returns \c true if this instance is valid. */
	bool isValid () const;
	
	/** Returns the name of the field. */
	QByteArray name () const;
	
	/** Returns the type name of the field. */
	QByteArray typeName () const;
	
	/** Returns access flags for this field. */
	Access access () const;
	
	/** Attempts to read the current value from \a instance. */
	QVariant read (void *instance) const;
	
	/**
	 * Attempts to set the current value int \a instance.
	 * On failure \c false is returned.
	 */
	bool write (void *instance, const QVariant &value);
	
	/**
	 * Returns the number of known annotations.
	 * \sa annotation
	 */
	int annotationCount () const;
	
	/**
	 * Returns the MetaAnnotation instance for the annotation at \a idx.
	 * \sa annotationCount
	 */
	MetaAnnotation annotation (int idx);
	
private:
	friend class MetaObject;
	
	inline MetaField (MetaObject *meta, short index)
		: m_meta (meta), m_index (index)
	{}
	
	mutable MetaObject *m_meta;
	short m_index;
};

/**
 * \brief The MetaEnum class provides access to enum's in a MetaObject.
 * 
 * 
 */
class NURIA_CORE_EXPORT MetaEnum {
public:
	
	/** Creates a invalid instance. */
	MetaEnum ();
	
	/** Returns \c true if this instance is valid. */
	bool isValid () const;
	
	/** Returns the name of the field. */
	QByteArray name () const;
	
	/** Returns the number of elements. */
	int elementCount () const;
	
	/** Returns the name of element \a at. */
	QByteArray key (int at) const;
	
	/** Returns the value of element \a at. */
	int value (int at) const;
	
	/**
	 * Returns the first key which points to \a value.
	 * If no key points to \a value, a empty QByteArray is returned.
	 */
	QByteArray valueToKey (int value) const;
	
	/**
	 * Returns the value of \a key. If there's no value for \a key,
	 * \c -1 is returned.
	 */
	int keyToValue (const QByteArray &key) const;
	
	/**
	 * Returns the number of known annotations.
	 * \sa annotation
	 */
	int annotationCount () const;
	
	/**
	 * Returns the MetaAnnotation instance for the annotation at \a idx.
	 * \sa annotationCount
	 */
	MetaAnnotation annotation (int idx);
	
private:
	friend class MetaObject;
	
	inline MetaEnum (MetaObject *meta, int index)
		: m_meta (meta), m_index (index)
	{}
	
	mutable MetaObject *m_meta;
	int m_index;
	
};

/** Map of MetaObjects. Key = className(). */
typedef QMap< QByteArray, MetaObject * > MetaObjectMap;

/**
 * \brief The MetaObject class provides access to meta-data generated by Tria.
 * 
 * \par Order of elements
 * All elements, that is, parent-class names, annotations, methods, fields and
 * enums are sorted by name by the MetaObject. Methods are sorted by name too,
 * but methods with the same name are sorted by argument count.
 * Annotations of enums, fields and methods are sorted too, as are enum keys.
 * 
 * This means that all elements are sorted in ascending order, which allows you
 * to use binary search algorithms.
 */
class NURIA_CORE_EXPORT MetaObject {
public:
	
	/**
	 * Methods for gateCall. You only need to know about this if you intend
	 * to sub-class MetaObject yourself.
	 */
	enum class GateMethod {
		ClassName = 0, // QByteArray
		MetaTypeId = 1, // int
		PointerMetaTypeId = 2, // int
		BaseClasses = 3, // QVector< QByteArray >
		AnnotationCount = 4, // int
		MethodCount = 5, // int
		FieldCount = 6, // int
		EnumCount = 7, // int
		
		AnnotationName = 10, // QByteArray
		AnnotationValue = 11, // QVariant
		
		MethodName = 20, // QByteArray
		MethodType = 21, // QByteArray
		MethodReturnType = 22, // QByteArray
		MethodArgumentNames = 23, // QVector< QByteArray >
		MethodArgumentTypes = 24, // QVector< QByteArray >
		MethodCallback = 25, // Nuria::Callback, additional = void *instance
		MethodUnsafeCallback = 26, // Nuria::Callback, additional = void *instance
		MethodArgumentTest = 27, // Nuria::Callback, additional = void *instance
		
		FieldName = 30, // QByteArray
		FieldType = 31, // QByteArray
		FieldRead = 32, // QVariant, additional = void *instance
		FieldWrite = 33, // bool, additional = void** = { instance, value }
		FieldAccess = 34, // MetaField::Access
		
		EnumName = 40, // QByteArray
		EnumElementCount = 41, // int
		EnumElementKey = 42, // QByteArray
		EnumElementValue = 43, // int
		
		DestroyInstance = 50 // void, additional = void *instance
	};
	
	/** C++ access specifiers. */
	enum Access {
		Public = 0,
		Private,
		Protected
	};
	
	/**
	 * Returns the corresponding MetaObject instance for \a type.
	 * If \a type is not known, \c nullptr is returned.
	 */
	static MetaObject *byName (const QByteArray &type);
	
	/**
	 * Returns all types which inherit \a typeName.
	 * \note A type does not inherit itself.
	 */
	static MetaObjectMap typesInheriting (const QByteArray &typeName);
	
	/**
	 * Returns all types which have a annotation called \a name stored
	 * in the type. This method does not search through methods, etc.
	 */
	static MetaObjectMap typesWithAnnotation (const QByteArray &name);
	
	/**
	 * Returns a map of all known types.
	 */
	static MetaObjectMap allTypes ();
	
	/**
	 * \brief Registers \a object to the global meta system.
	 * 
	 * This method will not take ownership of \a object, you'll need to make
	 * sure that it'll survive until the application quits.
	 * Please note that you rarely need to do this by hand.
	 * 
	 * \warning The class-name of \a object must be \b unique
	 * application-wide, else you'll override a already existing type.
	 */
	static void registerMetaObject (MetaObject *object);
	
	/** Constructor, does nothing. */
	inline MetaObject () {}
	
	/**
	 * Returns the class name of the represented type.
	 */
	QByteArray className ();
	
	/**
	 * Returns the Qt meta-type id of this type. Returns \c 0 if this type
	 * doesn't have value-type semantics.
	 */
	int metaTypeId ();
	
	/**
	 * Returns the Qt meta-type id of this type as pointer.
	 */
	int pointerMetaTypeId ();
	
	/**
	 * Returns a list of types this type inherits.
	 */
	QVector< QByteArray > parents ();
	
	/**
	 * Returns the number of known annotations.
	 * \sa annotation
	 */
	int annotationCount ();
	
	/**
	 * Returns the MetaAnnotation instance for the annotation at \a idx.
	 * \sa annotationCount
	 */
	MetaAnnotation annotation (int idx);
	
	/**
	 * Returns the number of known methods.
	 * \sa method
	 */
	int methodCount ();
	
	/**
	 * Returns the MetaMethod instance for the method at \a idx.
	 * \sa methodCount
	 */
	MetaMethod method (int idx);
	
	/**
	 * Returns the index of the first overload of method \a name.
	 * Returns \c -1 if there's no method called like that.
	 */
	int methodLowerBound (const QByteArray &name);
	
	/**
	 * Returns the index of the last overload of method \a name.
	 * Returns \c -1 if there's no method called like that.
	 */
	int methodUpperBound (const QByteArray &name);
	
	/**
	 * Tries to find the method \a prototype. The first item in \a prototype
	 * is the method name, all latter items are typenames of the arguments
	 * in the correct order.
	 * 
	 * The method name of a constructor is "" (Empty).
	 * If the method can't be found, the returned method is invalid.
	 */
	MetaMethod method (const QVector< QByteArray > &prototype);
	
	/**
	 * Destroys \a instance.
	 * \warning \a instance must be of the type the MetaObject represents,
	 * else the behaviour is undefined!
	 */
	void destroyInstance (void *instance);
	
	/**
	 * Returns the number of known fields.
	 * \sa field
	 */
	int fieldCount ();
	
	/**
	 * Returns the MetaField instance for the field at \a idx.
	 * \sa fieldCount
	 */
	MetaField field (int idx);
	
	/**
	 * Tries to find the field \a name. Returns a valid MetaField instance
	 * on success, else the instance is invalid.
	 * \sa MetaField::isValid
	 */
	MetaField fieldByName (const QByteArray &name);
	
	/**
	 * Returns the number of known enums.
	 * \sa enumAt
	 */
	int enumCount ();
	
	/**
	 * Returns the MetaEnum instance for the enum at \a idx.
	 * \sa enumCount
	 */
	MetaEnum enumAt (int idx);
	
	/**
	 * Tries to find the enum \a name. Returns a valid MetaEnum instance
	 * on success, else the instance is invalid.
	 * \sa MetaEnum::isValid
	 */
	MetaEnum enumByName (const QByteArray &name);
	
protected:
	
	/**
	 * Calls the back-end. \a method will be called, passing \a category,
	 * \a target and \a index, the result will be written to \a result.
	 * 
	 * This technique makes the vtable a lot smaller and allows further
	 * expansion in the future while not breaking the ABI.
	 */
	virtual void gateCall (GateMethod method, int category, int index, int nth,
			       void *result, void *additional = 0) = 0;
	
private:
	friend class MetaAnnotation;
	friend class MetaMethod;
	friend class MetaField;
	friend class MetaEnum;
	
};

}

#endif // NURIA_METAOBJECT_HPP
