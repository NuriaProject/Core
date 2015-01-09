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
 * enums, methods and fields.
 * 
 * \par Usage
 * Annotations are basically a key-value pair. The key is always a QByteArray,
 * while the value can be of arbitary type. This means you can also store
 * complex types (Other than integers and strings) as long the type is
 * registered in Qts meta system using Q_DECLARE_METATYPE.
 * 
 * The primary macro for this is NURIA_ANNOTATE. This macro takes two arguments,
 * first being the key written as symbol and the second one being the value.
 * 
 * Annotations can be accessed using annotationCount() and annotation() in
 * Nuria::Meta* classes.
 * 
 * \par Order of annotations
 * Annotations are always sorted by key in ascending order. There may be
 * multiple annotations of the same key in any given element. If there are
 * multiple annotations with the same key, the order they were written in
 * is retained.
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
 * By default, constructors, static and member methods are exported.
 * 
 * \par Order of methods
 * Methods are sorted by name first and then by argument count in ascending
 * order. If there are more than one methods with the same name and same
 * argument count, only differing in argument types expected, the order is
 * undefined.
 * 
 * \par Methods with default argument values
 * Methods with default argument values are "expanded", meaning that for each
 * possible function prototype a method is exposed of the same name.
 * 
 * Thus, the method
 * \codeline int foo (int a = 1, int b = 2, int c = 3);
 * would be expanded into four methods:
 * \code
 * int foo ();
 * int foo (int a);
 * int foo (int a, int b);
 * int foo (int a, int b, int c);
 * \endcode
 * 
 * \par Usage
 * You can use callback() and unsafeCallback() to retrieve a Callback instance
 * which, when invoked, will call the actual method. Those methods are only
 * different when using NURIA_REQUIRE. When a method have a NURIA_REQUIRE
 * annotation attached, it'll be used to validate arguments on invocation.
 * Arguments can be checked by their name. 
 * 
 * \note Returned Callback instances are bound to whatever instance you passed
 * to the getter method.
 * 
 * \par Constructors
 * Unlike the Qt API, constructors are exposed as methods and to be used like
 * static methods. The returned QVariant will report a type of \c Type*.
 * 
 * \note To get the pointer to the instance easily, use Variant::stealPointer().
 * 
 * \par Behaviour of NURIA_REQUIRE
 * NURIA_REQUIRE gets evaluated before calling the method itself. If the call
 * fails a invalid QVariant is returned. To check if a void method succeeded or
 * not you'll have to use testCallback(). To avoid further checks after you did
 * it yourself, you can use unsafeCallback().
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
	MetaAnnotation annotation (int idx) const;	
	
	/**
	 * Returns the index of the first annotation \a name.
	 * Returns \c -1 if there's no annotation called like that.
	 */
	int annotationLowerBound (const QByteArray &name) const;
	
	/**
	 * Returns the index of the last annotation \a name.
	 * Returns \c -1 if there's no annotation called like that.
	 */
	int annotationUpperBound (const QByteArray &name) const;
	
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
 * \note Static variables are ignored.
 * 
 * \par NURIA_REQUIRE
 * Fields support NURIA_REQUIRE backed value checks prior setting a value.
 * Both raw fields and user-defined ones are supported. You can access other
 * member variables and methods as well as the new value itself, which is called
 * like the raw field, or like the first argument in a setter.
 * 
 * A new value is only set if the condition is \c true. If it doesn't, write()
 * fails and nothing happens.
 * 
 * Please see MetaObject for further information on NURIA_REQUIRE.
 * 
 * \par Types
 * Used types must be registered to the Qt meta system using Q_DECLARE_METATYPE.
 * Please note that the usual restrictions apply, meaning you should use a
 * typedef when registering a template type with more than one template
 * parameters. Not registering types won't stop Tria nor the project from
 * compiling though, as Tria will do it for you - But this won't apply to your
 * sources as it's only in the generated output.
 * 
 * \par Raw fields
 * When Tria stumbles upon a public member variable, it'll auto-generate
 * accessors for it if it's not annotated using NURIA_SKIP (In which case
 * it's ignored completely).
 * 
 * \par User-defined accessor methods
 * Nuria allows the use of user-defiend accessor methods (That is, getter and
 * setter) through the use of the NURIA_READ and NURIA_WRITE annotations. A
 * field may only have one or both defined. Both macros take exactly one
 * argument: The field name. The datatype is deduced by the accessor methods
 * itself.
 * 
 * \note Tria will raise an error if getter and setter expect different types.
 * 
 * The getter must return the field value and may take optional arguments.
 * The setter must take the field value as first argument and may take
 * additional optional arguments. A setter is allowed to return either void
 * or bool. In the latter case, a result value of \c true indicates success
 * and \c false indicates failure. This is passed through to the caller through
 * write().
 * 
 * Accessor methods won't be exposed as usual methods. Custom annotations tacked
 * to accessor methods are applied to the field.
 * 
 * \note As usual, accessor methods must be public.
 * \note Redefining accessors for a field will yield a warning.
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
	MetaAnnotation annotation (int idx) const;
	
	/**
	 * Returns the index of the first annotation \a name.
	 * Returns \c -1 if there's no annotation called like that.
	 */
	int annotationLowerBound (const QByteArray &name) const;
	
	/**
	 * Returns the index of the last annotation \a name.
	 * Returns \c -1 if there's no annotation called like that.
	 */
	int annotationUpperBound (const QByteArray &name) const;
	
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
 * \par Order of elements
 * Elements in a enum are always sorted by key in ascending order.
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
	MetaAnnotation annotation (int idx) const;
	
	/**
	 * Returns the index of the first annotation \a name.
	 * Returns \c -1 if there's no annotation called like that.
	 */
	int annotationLowerBound (const QByteArray &name) const;
	
	/**
	 * Returns the index of the last annotation \a name.
	 * Returns \c -1 if there's no annotation called like that.
	 */
	int annotationUpperBound (const QByteArray &name) const;
	
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
 * \brief The MetaObject class provides access to meta-data of types at
 * run-time.
 * 
 * Instances of this class are usually generated by Tria, the code-generator
 * of the Nuria Framework. While this is the primary use-case, you're free
 * to sub-class this class yourself to create types at run-time.
 * 
 * Using Tria you're able to "expose" arbitary structures to be used dynamically
 * at run-time. 
 * 
 * \par How to use Tria
 * Tria is a code-generator comparable to Qt's meta object compiler (moc). And
 * like moc, Tria will be run on all header files in a project. All you need to
 * do for this is to include the Nuria Framework in your project using the known
 * mechanism:
 * 
 * \code
 * CONFIG += nuria
 * NURIA  += core
 * \endcode
 * 
 * If you want to use tria without qmake, please see the nuria.prf file and
 * tria itself for information (Calling it with "-h" as argument).
 * 
 * \par Preparing structures for Tria
 * To tell Tria you want to a structure to be exported you'll use the
 * NURIA_INTROSPECT annotation. Keep in mind that attributes of any kind
 * go \b after the terminal for structs/classes and enums and go \b before
 * methods and fields:
 * 
 * \code
 * // structs/classes and enums expect annotations *after* the terminal.
 * struct NURIA_INTROSPECT NURIA_ANNOTATE(IsAClass, true) {
 *   enum NURIA_ANNOTATE(IsAEnum, 2 / 2) Foo { };
 * 
 *   // Methods and fields expect annotations in front.
 *   NURIA_ANNOTATE(IsAMethod, "yes")
 *   QVariant iAmAMethod ();
 *   
 *   NURIA_ANNOTATE(IsAField, true)
 *   int field;
 * };
 * \endcode
 * 
 * That already covers the basics. You should probably Q_DECLARE_METATYPE your
 * types. Tria will automatically do this in the generated code for all needed
 * types though.
 * 
 * \par Automatically exported things
 * Tria will export \b public methods (member methods, static methods and
 * constructors), enums (Including elements) and fields ("raw fields" and
 * user-defined ones with accessor methods). Everything else, including
 * private and protected elements, is ignored. To explicitly ignore a element,
 * annotate it with NURIA_SKIP.
 * 
 * \par Annotations in overview
 * There are currently multiple annotations available.
 * 
 * \c NURIA_INTROSPECT will mark a struct or a class to be exported. It's
 * ignored on anything else.
 * 
 * \c NURIA_SKIP can be used to explicitly mark elements (methods, enums,
 * fields) to be ignored by Tria.
 * 
 * \c NURIA_READ and \c NURIA_WRITE mark a method to be used as getter or
 * setter method for the field whose name is passed as only argument. Methods
 * with this annotation will \b not be exposed as methods themselves. Please
 * refer to MetaField for further information.
 * 
 * \c NURIA_REQUIRE is a powerful mechanism which lets you write a requirement
 * which needs to be fulfilled for the operation to succeed. Can be applied to
 * fields and methods (Of all kind). The macro itself takes a C++ condition
 * as check. The condition can call methods as well as use fields directly, as
 * long the method/field the macro is applied to requires a instance of the
 * object itself (E.g., it's a member method). These conditions are embedded
 * into the generated source by Tria, thus there's no notable overhead
 * introduced here.
 * 
 * \par Detection of conversions in Tria
 * Tria will automatically detect methods supporting conversions. These are:
 * - Constructors taking only one argument
 * - Static methods beginning with "from" taking one argument
 * - Member methods beginning with "to" taking one argument
 * - C++ conversion operators
 * 
 * Except for the conversion operators, all methods are still exposed as normal
 * methods. Methods may take additional optional arguments.
 * 
 * \note The conversion is registered in the QVariant conversion feature
 * introduced in Qt5.2
 * \note This is a feature of Tria, not of MetaObject. Thus, custom sub-classes
 * of MetaObject won't have this feature.
 * 
 * \par Order of elements
 * All elements, that is, parent-class names, annotations, methods, fields and
 * enums are sorted by name by the MetaObject.
 * Annotations of enums, fields and methods are sorted too, as are enum keys.
 * 
 * This means that all elements are sorted in ascending order, which allows you
 * to use binary search algorithms.
 * 
 * \par Creating types at run-time
 * You can sub-class MetaObject yourself if you need to create types at
 * run-time, e.g. when embedding a scripting language. You'll need to implement
 * the protected gateCall method. While it may look strange to only have a
 * single virtual method for everything, this method was chosen as it reduces
 * the size of a MetaObject significantly while offering marginal run-time
 * overhead. On top of that it ensures that MetaObject can be extended in the
 * future without breaking ABI or API compatibility.
 * 
 * Please refer to the GateMethod enum class. You can also read the source code
 * of MetaObject or open a file generated by Tria.
 * 
 * \sa Nuria::MetaObject::GateMethod
 * 
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
		MethodType = 21, // Nuria::MetaMethod::Type
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
	
	/**
	 * Returns the MetaObject of \c T. For this to work, \c T* has to be
	 * registered to the Qt meta system using
	 * \codeline Q_DECLARE_METATYPE(T*)
	 */
	template< typename T >
	static MetaObject *of () {
		int id = qMetaTypeId< T * > ();
		const char *typeName = QMetaType::typeName (id);
		return MetaObject::byName (QByteArray (typeName, ::strlen (typeName) - 1));
	}
	
	/**
	 * Returns the corresponding MetaObject instance for \a type.
	 * If \a type is not known, \c nullptr is returned.
	 */
	static MetaObject *byName (const QByteArray &type);
	
	/**
	 * Returns the MetaObject for the type \a typeId. If \a typeId may be
	 * the Qt type id for \c T* or \c T. If no matching meta object is
	 * found \c nullptr is returned. 
	 */
	static MetaObject *byTypeId (int typeId);
	
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
	
	/** Destructor, does nothing. */
	virtual ~MetaObject () {}
	
	/**
	 * Returns the class name of the represented type.
	 */
	QByteArray className ();
	
	/**
	 * Returns the Qt meta-type id of this type. Returns \c 0 if this type
	 * doesn't have value-semantics.
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
	MetaAnnotation annotation (int idx) const;
	
	/**
	 * Returns the index of the first annotation \a name.
	 * Returns \c -1 if there's no annotation called like that.
	 */
	int annotationLowerBound (const QByteArray &name) const;
	
	/**
	 * Returns the index of the last annotation \a name.
	 * Returns \c -1 if there's no annotation called like that.
	 */
	int annotationUpperBound (const QByteArray &name) const;
	
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
