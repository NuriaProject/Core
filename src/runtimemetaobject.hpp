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

#ifndef NURIA_RUNTIMEMETAOBJECT_HPP
#define NURIA_RUNTIMEMETAOBJECT_HPP

#include "metaobject.hpp"
#include <functional>

namespace Nuria {

class RuntimeMetaObjectPrivate;

/**
 * \brief RuntimeMetaObject lets you easily define types at run-time.
 * 
 * This class can be used when you want to quickly construct a type at run-time
 * without needing to implement the gateCall method yourself.
 * 
 * \par Usage
 * 1. Intantiate RuntimeMetaObject
 * 2. Use the add*() methods to set-up your virtual type
 * 3. Use finalize() to make the meta-object usable.
 * 
 * You may want to use MetaObject::registerMetaObject() if you want to make your
 * type known application-wide.
 * 
 * \par Behaviour on uniqueness
 * The add*() methods will always ensure uniqueness. If two elements are added
 * deemed equal by C++ rules, then the latter one replaces the one already
 * stored. This means when e.g. adding two enums sequentially with the same
 * name, only the latter will be used.
 * 
 * Methods are deemed equal if both have the same name and take the same amount
 * of arguments, all of which are the same type. This means that the following
 * two prototypes are equal to RuntimeMetaObject:
 * \code
 * int foo (QString a, int b);
 * static QByteArray foo (QString house, int mouse);
 * \endcode
 * 
 */
class NURIA_CORE_EXPORT RuntimeMetaObject : public MetaObject {
public:
	
	/**
	 * Possible invocation types for InvokeCreator.
	 */
	enum class InvokeAction {
		
		/** Invoke the method \b with argument validation. */
		Invoke,
		
		/** Invoke the method \b without argument validation. */
		UnsafeInvoke,
		
		/** Only do argument validation. */
		ArgumentTest
	};
	
	/** Map for annotations. */
	typedef QMultiMap< QByteArray, QVariant > AnnotationMap;
	
	/**
	 * Lambda which when called returns a Nuria::Callback which when invoked
	 * does the real call. The first argument passed to this is the instance
	 * the returned Callback should act on. It may be \c nullptr for
	 * constructors or static methods. The second argument is of type
	 * InvokeAction which defines the expected behaviouf of the returned
	 * callback.
	 * \sa addMethod
	 */
	typedef std::function< Callback(void *, InvokeAction) > InvokeCreator;
	
	/** \sa addField */
	typedef std::function< QVariant(void *) > FieldGetter;
	
	/** \sa addField */
	typedef std::function< bool(void *, const QVariant &) > FieldSetter;
	
	/**
	 * Deleter function which is supposed to destroy the instance passed
	 * as first argument and free all memory associated with it.
	 * \sa setDeleter
	 */
	typedef std::function< void(void *) > InstanceDeleter;
	
	/** Constructor. */
	RuntimeMetaObject (const QByteArray &name);
	
	/** Destructor. */
	~RuntimeMetaObject ();
	
	/**
	 * Sets the Qt meta-type id. The default is \c 0 which is correct if
	 * this type doesn't have value-semantics. Usage is something like:
	 * \code
	 * myRuntimeMetaObject->setQtMetaTypeId (qMetaTypeId< MyType > ());
	 * \endcode
	 */
	void setQtMetaTypeId (int valueTypeId);
	
	/**
	 * Sets the Qt meta-type id of the pointer type.
	 * Usage is something like:
	 * \code
	 * myRuntimeMetaObject->setQtMetaTypeId (qMetaTypeId< MyType * > ());
	 * \endcode
	 */
	void setQtMetaTypePointerId (int pointerTypeId);
	
	/** Sets the annotation of this type. */
	void setAnnotations (const AnnotationMap &annotations);
	
	/** Sets the base classes. */
	void setBaseClasses (const QVector< QByteArray > &bases);
	
	/**
	 * Uses \a deleter as method which is called when a instance of
	 * the type this meta object represents should be destroyed.
	 * 
	 * The default deleter does nothing.
	 * \sa InstanceDeleter
	 */
	void setInstanceDeleter (InstanceDeleter deleter);
	
	/**
	 * Adds method \a name as \a type method with signature \a returnType,
	 * \a argumentNames, \a argumentTypes to the meta object.
	 * \a argumentNames and \a argumentTypes \b must have the same amount of
	 * elements, not doing so will lead to undefined behaviour.
	 * \a invokeCreator is a method which when called returns a
	 * Nuria::Callback which when called does the real invocation of the
	 * method. Please refer to InvokeCreator for more details on this.
	 * 
	 * \sa InvokeCreator
	 */
	void addMethod (MetaMethod::Type type, const QByteArray &name, const QByteArray &returnType,
			const QVector< QByteArray > &argumentNames, const QVector< QByteArray > &argumentTypes,
			const AnnotationMap &annotations, InvokeCreator invokeCreator);
	
	/**
	 * Adds enum \a name with elements \a keyValueMap to the type.
	 */
	void addEnum (const QByteArray &name, const AnnotationMap &annotations,
		      const QMap< QByteArray, int > &keyValueMap);
	
	/**
	 * Adds a writable field \a name to this object of \a valueType.
	 * \a getter is called when the field is read and \a setter when it
	 * should be changed.
	 */
	void addField (const QByteArray &name, const QByteArray &valueType, const AnnotationMap &annotations, 
		       FieldGetter getter, FieldSetter setter);
	
	/**
	 * Adds a read-only field \a name to this object of \a valueType.
	 * \a getter is called when the field is read.
	 */
	void addField (const QByteArray &name, const QByteArray &valueType, const AnnotationMap &annotations,
		       FieldGetter getter);
	
	/**
	 * Calling this method will ensure that all assumptions MetaObject has
	 * are met - That is, everything that is sorted is sorted. You \b have
	 * to call this method before using this instance as meta-object.
	 */
	void finalize ();
	
protected:
	void gateCall (GateMethod method, int category, int index, int nth, void *result, void *additional) override;
	
private:
	int runtimeAnnotationCount (int category, int index);
	QByteArray runtimeAnnotationName (int category, int index, int nth);
	QVariant runtimeAnnotationValue (int category, int index, int nth);
	
	QByteArray runtimeMethodName (int index);
	MetaMethod::Type runtimeMethodType (int index);
	QByteArray runtimeMethodReturnType (int index);
	QVector< QByteArray > runtimeMethodArgumentNames (int index);
	QVector< QByteArray > runtimeMethodArgumentTypes (int index);
	Callback runtimeMethodCallback (void *instance, int index);
	Callback runtimeMethodUnsafeCallback (void *instance, int index);
	Callback runtimeMethodArgumentTest (void *instance, int index);
	
	QByteArray runtimeFieldName (int index);
	QByteArray runtimeFieldType (int index);
	QVariant runtimeFieldRead (int index, void *instance);
	bool runtimeFieldWrite (int index, void *instance, const QVariant &value);
	MetaField::Access runtimeFieldAccess (int index);
	
	QByteArray runtimeEnumName (int index);
	int runtimeEnumElementCount (int index);
	QByteArray runtimeEnumElementKey (int index, int nth);
	int runtimeEnumElementValue (int index, int nth);
	
	RuntimeMetaObjectPrivate *d;
	
};

}

#endif // RUNTIMEMETAOBJECT_HPP
