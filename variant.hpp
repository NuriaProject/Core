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

#ifndef NURIA_VARIANT_HPP
#define NURIA_VARIANT_HPP

#include "essentials.hpp"
#include <QStringList>
#include <QMetaType>
#include <QVariant>

namespace Nuria {
class VariantIteratorPrivate;

/**
 * @brief The Variant class provides utilities for working with QVariants.
 * Working with QVariants can be tedious if you have to work with them without
 * knowing the structure of the data they hold. So, if you need to do convert
 * a QVariant or want to compare them look no futher.
 * 
 * \warning Do not use types with this class which use virtual methods as this
 * can lead to crashes. If you want to use a type like this, register the
 * pointer-type instead.
 * 
 * \par Comparisons
 * Variant provides multiple ways to compare two QVariants.
 * First, if you haven't defined \b NURIA_NO_TEMPLATE_COMPARISON you're able
 * to simply compare a QVariant to any class you registered to the Qt meta
 * system:
 * \codeline if (myQVariant < currentIndex) { // ...
 * 
 * Another possibility is to use the comparison methods as provided by Variant
 * like equal or lessThan:
 * \codeline if (Nuria::Variant::lessThan (myQVariant, currentIndex)) { // ...
 * 
 * To be able to compare QVariants you need to register your classes by using
 * registerType.
 * \note All registered classes must overload operator==() and operator<()
 * correctly.
 * 
 * Comparisons are alyways 'smart'. If you try to compare two QVariants of
 * non-comparable types, it will first try to convert the RHS QVariant to
 * the type of the LHS QVariant. If this fails, it tries to convert the RHS
 * to something the LHS can compare to. If this still fails, LHS is checked
 * if it can be compared to QVariants. If yes these operators are used, if
 * not \c false is returned.
 * To check if two types are comparable use one of the many canCompare
 * overloads.
 * 
 * \sa canCompare
 * 
 * \warning A failed comparison always returns \c false!
 * \note The LHS won't be touched. All conversions are done to the RHS only.
 * 
 * \par Conversions
 * Another feature that QVariant is lacking is the ability to convert a QVariant
 * to or from a user type. This means you can register conversion functions
 * which take type A and return a pointer to an instance of type B.
 * \sa convert canConvert registerConversion
 * 
 * It is also possible to check beforehand if a certain conversion is possible
 * at all using one of the various canConvert() overloads:
 * - By using the template version
 * \codeline if (Nuria::Variant::canConvert< QString > (myQVariant)) { // ...
 * 
 * - By using the type id of to target type
 * \codeline if (Nuria::Variant::canConvert (myQVariant, QMetaType::QString)) { // ...
 * 
 * - By using the type id of both the source and target type
 * \codeline if (Nuria::Variant::canConvert (QMetaType::Int, QMetaType::QString)) { // ...
 * 
 * \note Custom conversions take precedence over the default Qt ones!
 * 
 * \par Iterating over a variant
 * A variant can hold multiple items if it contains a QStringList or a 
 * QVariantList. Sometimes it would be neat if you could iterate over this list
 * without knowing its exact type, right? This is where VARIANT_FOREACH comes
 * in. This little macro works like Q_FOREACH, with the exception it always
 * expects a QVariant instead of a container. VARIANT_FOREACH iterates over any
 * iteratorable type.
 * \sa registerIterators
 * 
 * Example:
 * \code
 * QVariant a ("Hello"), b (QStringList() << "One" << "Two" << "Three");
 * // Outputs: "Hello"
 * VARIANT_FOREACH(const QVariant &cur, a) qDebug() << cur;
 * 
 * //Outputs: "One" "Two" "Three" (on a line of their own)
 * VARIANT_FOREACH(const QVariant &cur, b) qDebug() << cur;
 * \endcode
 * 
 * If you need more control though, there is Variant::Iterator for you.
 * It works like all Qt STL-style iterators.
 * \sa begin end Iterator
 * 
 */
class NURIA_CORE_EXPORT Variant {
public:
	
	/**
	 * Prototype of a comparison function.
	 */
	typedef bool (*Comparison)(const void *, const void *);
	
	/**
	 * Prototype of a conversion function.
	 */
	typedef void *(*Conversion)(const void *);
	
	// Destructor
	typedef void (*IteratorDtorFunc)(void *);
	
	// Copy constructor
	typedef void *(*IteratorCopyFunc)(void *);
	
	// For value() and key()
	typedef QVariant (*IteratorValueFunc)(void *, int);
	
	// For operator++ and operator--
	typedef void (*IteratorIncFunc)(void *);
	
	// For operator+ and operator-
	typedef void *(*IteratorMoveFunc)(void *, int);
	
	// For operator-(Iterator)
	typedef int (*IteratorDistanceFunc)(void *, void *);
	
	// For begin and end
	typedef void *(*IteratorStartEndFunc)(void *);
	
	// For operator==
	typedef bool (*IteratorComparisonFunc)(void *, void *);
	
	// For count()
	typedef int (*IteratorCountFunc)(void *);
	
	// For find()
	typedef void *(*IteratorFindFunc)(void *, const QVariant &);
	
	/**
	 * \defgroup Comparison methods
	 * @{
	 */
	static bool equal (const QVariant &left, const QVariant &right);
	static bool notEqual (const QVariant &left, const QVariant &right);
	
	static bool lessThan (const QVariant &left, const QVariant &right);
	static bool greaterThan (const QVariant &left, const QVariant &right);
	
	static bool lessEqualThan (const QVariant &left, const QVariant &right);
	static bool greaterEqualThan (const QVariant &left, const QVariant &right);
	
	/** @} */
	
	/**
	 * \defgroup Comparison check methods
	 * @{
	 */
	
	/**
	 * Returns \c true if \a left can be compared to \a right.
	 */
	inline static bool canCompare (const QVariant &left, const QVariant &right)
	{ return canCompare (left.userType (), right.userType ()); }
	
	/**
	 * \overload
	 */
	inline static bool canCompare (const QVariant &left, int rightType)
	{ return canCompare (left.userType (), rightType); }
	
	/**
	 * \overload
	 */
	template< typename Right > inline static bool canCompare (const QVariant &left)
	{ return canCompare (left.userType (), qMetaTypeId< Right > ()); }
	
	/**
	 * \overload
	 */
	template< typename Left, typename Right > inline static bool canCompare ()
	{ return canCompare (qMetaTypeId< Left > (), qMetaTypeId< Right > ()); }
	
	/**
	 * \overload
	 */
	static bool canCompare (int leftType, int rightType);
	
	/** @} */
	
	/**
	 * \defgroup Convert methods
	 * @{
	 */
	
	/**
	 * Tries to convert \a variant to \a type.
	 * If conversion fails a invalid QVariant is returned.
	 */
	static QVariant convert (const QVariant &variant, int type);
	
	/**
	 * \overload
	 */
	template< typename T > static QVariant convert (const QVariant &variant)
	{ return convert (variant, qMetaTypeId< T > ()); }
	
	/** @} */
	
	/**
	 * \defgroup Convert check methods
	 * @{
	 */
	
	/**
	 * Returns \c true if the conversion is possible.
	 * \warning This method shares the semantics with QVariant::canConvert.
	 * This means that the conversion itself is possible, but not if a
	 * conversion would be successful!
	 */
	template< typename T > inline static bool canConvert (const QVariant &variant)
	{ return canConvert (variant.userType (), qMetaTypeId< T > ()); }
	
	/**
	 * \overload
	 * Returns \c true if \a variant can be converted to \a toType.
	 */
	inline static bool canConvert (const QVariant &variant, int toType)
	{ return canConvert (variant.userType (), toType); }
	
	/**
	 * \overload
	 */
	static bool canConvert (int fromType, int toType);
	
	/** @} */
	
	/**
	 * Same as QVariant::value, but this method tries to convert using
	 * the convert method instead.
	 */
	template< typename T > static T toValue (const QVariant &variant) {
		QVariant result = convert (variant, qMetaTypeId< T > ());
		
		if (result.isValid ()) {
			return result.value< T > ();
		}
		
		return T ();
	}
	
	/**
	 * Registers a class to the system. After this method has been is called
	 * it is possible to compare QVariants containing instances of this
	 * class.
	 * \note It is safe to call this method multiple times on the same
	 * class.
	 * \note If you want to compare against plain QVariants see the
	 * overloads of this method.
	 * 
	 * A good place to do this would be for example the constructor of the
	 * specific class. 
	 */
	template< typename T > inline static void registerType ()
	{ registerType< T, T > (); }
	
	/**
	 * \overload
	 * Same as the other registerType method, except that you can specify
	 * the type you want to compare your type to
	 */
	template< typename T, typename Right > inline static void registerType () {
		typedef bool(T::*Func)(const Right &) const;
		Func equal = &T::operator==;
		Func less = &T::operator<;
		registerType (qMetaTypeId< T > (), qMetaTypeId< Right > (),
			      (Comparison)equal, (Comparison)less);
	}
	
	/**
	 * \overload
	 * Registers a class without using templates.
	 * If you have implementations of all comparison operators for this
	 * type use the other overload of this method.
	 * \a type is the usertype id as reported by the Qt meta system.
	 */
	static void registerType (int type, int rightType, Comparison equal, Comparison lessThan);
	
	/**
	 * \overload
	 * Registers a class without using templates.
	 * If you have implemented all comparison operators use this method.
	 */
	static void registerType (int type, int rightType, Comparison equal, Comparison notEqual,
				  Comparison less, Comparison greater, Comparison lessEqual,
				  Comparison greaterEqual);
	
	/**
	 * Registers a possible conversion from type \a From to type \a To.
	 * This method takes a pointer to a conversion method. This method
	 * must be static and must take an instance of <i>const From &</i> as
	 * only argument. The method must return a pointer to an instance of
	 * \a To. If conversion fails the function should return \c 0.
	 */
	template< typename From, typename To >
	static void registerConversion (To *(*func)(const From &))
	{ registerConversion (qMetaTypeId< From > (), qMetaTypeId< To > (), (Conversion)func); }
	
	/**
	 * \overload
	 * Registers a automatically generated conversion function, which can
	 * convert a QVariant containing a instance of \a From to a QVariant
	 * containing a instance of \a To. This is possible if \a To has a
	 * constructor which accepts a instance of \a To. Prototype:
	 * \codeline From (const To &);
	 * or
	 * \codeline From (To);
	 */
	template< typename From, typename To >
	static void registerConversion () {
		auto func = &autoConvTempl< From, To >;
		registerConversion (qMetaTypeId< From > (), qMetaTypeId< To > (), (Conversion)func);
	}
	
	/**
	 * \overload
	 * Registers a possible conversion from \a from to \a to.
	 * The semantics of \a func are the same.
	 */
	static void registerConversion (int from, int to, Conversion func);
	
	/**
	 * Registers a iteratorable type to the system so it can be used by
	 * Variant::Iterator and VARIANT_FOREACH.
	 */
	template< typename T >
	static void registerIterators (typename T::mapped_type *a = (typename T::mapped_type *)0) {
		Q_UNUSED(a)
		int keyType = qMetaTypeId< typename T::key_type > ();
		int valueType = qMetaTypeId< typename T::mapped_type > ();
		
		registerIterator (qMetaTypeId< T > (), valueType, keyType,
				  (IteratorDtorFunc)&iteratorDtor< T >,
				  (IteratorCopyFunc)&iteratorCopy< T >,
				  (IteratorStartEndFunc)&iteratorBegin< T >,
				  (IteratorStartEndFunc)&iteratorEnd< T >,
				  (IteratorIncFunc)&iteratorNext< T >,
				  (IteratorIncFunc)&iteratorPrev< T >,
				  (IteratorMoveFunc)&iteratorMove< T >,
				  (IteratorDistanceFunc)&iteratorDistance< T >,
				  (IteratorValueFunc)&iteratorValue< T >,
				  (IteratorValueFunc)&iteratorKey< T >,
				  (IteratorComparisonFunc)&iteratorEqual< T >,
				  (IteratorCountFunc)&iteratorCount< T >,
				  (IteratorFindFunc)&iteratorFindMap< T, typename T::key_type >);
		
	}
	
	/**
	 * \overload
	 * Overload for types which don't have keys (lists, vectors, ...)
	 */
	template< typename T > 
	static void registerIterators (typename T::value_type *a = (typename T::value_type *)0) {
		Q_UNUSED(a)
		int valueType = qMetaTypeId< typename T::value_type > ();
		
		registerIterator (qMetaTypeId< T > (), valueType, 0,
				  (IteratorDtorFunc)&iteratorDtor< T >,
				  (IteratorCopyFunc)&iteratorCopy< T >,
				  (IteratorStartEndFunc)&iteratorBegin< T >,
				  (IteratorStartEndFunc)&iteratorEnd< T >,
				  (IteratorIncFunc)&iteratorNext< T >,
				  (IteratorIncFunc)&iteratorPrev< T >,
				  (IteratorMoveFunc)&iteratorMove< T >,
				  (IteratorDistanceFunc)&iteratorDistance< T >,
				  (IteratorValueFunc)&iteratorValue< T >,
				  0,
				  (IteratorComparisonFunc)&iteratorEqual< T >,
				  (IteratorCountFunc)&iteratorCount< T >,
				  (IteratorFindFunc)&iteratorFindList< T, typename T::value_type >);
		
	}
	
	/**
	 * Iterator class for iterating over QVariants.
	 * This class works like all STL-style iterators.
	 * \sa begin end
	 * 
	 * \warning This iterator does not support modifiying items.
	 */
	class Iterator {
	public:
		
		/**
		 * Constructs an invalid iterator.
		 * See Variant::begin and Variant::end.
		 */
		Iterator ();
		Iterator (const Iterator &other);
		~Iterator ();
		
		bool isValid () const;
		
		const Iterator &operator= (const Iterator &other);
		
		bool operator== (const Iterator &other) const;
		inline bool operator!= (const Iterator &other) const
		{ return !(*this == other); }
		
		Iterator &operator++ ();
		inline Iterator &operator++ (int) { return operator++ (); }
		
		Iterator &operator-- ();
		inline Iterator &operator-- (int) { return operator--(); }
		const Iterator &operator+= (int n);
		const Iterator &operator-= (int n);
		Iterator operator+ (int n) const;
		Iterator operator- (int n) const;
		int operator- (const Iterator &other) const;
		
		/**
		 * Returns the key of the current item. If the iterated variant
		 * doesn't contain a map-type an invalid QVariant is returned.
		 * \sa isMap
		 */
		QVariant key () const;
		QVariant value () const;
		
		inline QVariant operator* () const
		{ return value (); }
		
	private:
		friend class Nuria::Variant;
		Iterator (const QVariant &variant, void *iter, void *info, int pos);
		VariantIteratorPrivate *d_ptr;
		
	};
	
	/**
	 * Returns the begin iterator of \a variant.
	 * If \a variant is \c invalid, the returned iterator will be equal
	 * to the end iterator.
	 */
	static Iterator begin (QVariant &variant);
	
	/**
	 * Returns the end iterator of \a variant.
	 */
	static Iterator end (QVariant &variant);
	
	/**
	 * Returns an iterator pointing to the item with key
	 * \a ident if \a variant contains a map-like type.
	 * If \a variant contains a list-like type, it'll search for
	 * an item which matches \a ident. If \a variant is generic,
	 * the value will be matched to \a ident. Whatever check is
	 * done, if \a ident can't be found a \c invalid Iterator instance
	 * is returned.
	 * \warning This function does \b not work like its STL-style
	 * counterpart!
	 * \sa Iterator::isValid
	 */
	static Iterator find (QVariant &variant, QVariant ident);
	
	/**
	 * Returns the item count in a QVariant.
	 * If \a variant contains a non-iterable type \c 1 is returned.
	 * If \a variant is \a invalid \c 0 is returned.
	 * If \a variant contains a iterable type the item count is returned.
	 */
	static int itemCount (const QVariant &variant);
	
	/**
	 * Returns \c true if \a variant contains a iterable list.
	 * \note A list in the sense of: Doesn't have a key.
	 */
	static bool isList (const QVariant &variant);
	
	/** \overload */
	static bool isList (int typeId);
	
	/**
	 * Returns \c true if \a variant contains a iterable map.
	 */
	static bool isMap (const QVariant &variant);
	
	/** \overload */
	static bool isMap (int typeId);
	
	/**
	 * Returns \c true if \a variant contains a generic type.
	 * A generic type is a type which isn't a map nor a list.
	 */
	static bool isGeneric (const QVariant &variant);
	
	/** \overload */
	static bool isGeneric (int typeId);
	
	/**
	 * Tries to convert \a variant to a QVariantList.
	 * - If \a variant is not a list nor a map, a list containing
	 *   \a variant is returned.
	 * - If \a variant is a list, its contents are copied over.
	 * - If \a variant is a map, all values are copied to a list.
	 * - If \a variant is \a invalid, an empty QVariantList is returned.
	 */
	static QVariantList toList (QVariant variant);
	
	/**
	 * Tries to convert \a variant to a QVariantMap.
	 * This is only successful if:
	 * - \a variant contains a map of some type.
	 * - The key-type of the map can be converted to a QString.
	 * 
	 * If one of these rules fail, an empty QVariantMap is returned.
	 * \sa isMap
	 */
	static QVariantMap toMap (QVariant variant);
	
	/**
	 * Builds a QVariantList out of all arguments passed to the method.
	 * This does the same as
	 * \codeline QVariantList () << QVariant::fromValue (first) << ...
	 * But it's easier and shorter to read and write.
	 * 
	 * \note This method implicitly converts passed arguments of type
	 * const char* to QString. You can disable this by defining
	 * NURIA_NO_CHAR_ARRAY_TO_QSTRING
	 */
	template< typename ... Items >
	static QVariantList buildList (const Items &... items) {
		QVariantList list;
		buildListImpl (list, items ...);
		return list;
	}
	
private:
	
	/**
	 * \internal
	 * \defgroup Iterator helper functions
	 * @{
	 */
	
	// We use C-style casts here so this code doesn't get more unreadable...
	template< typename T > inline static void iteratorDtor (void *t)
	{ delete (typename T::const_iterator *)t; }
	
	template< typename T > inline static void *iteratorCopy (void *t)
	{ return (void *)(new typename T::const_iterator (*(typename T::const_iterator *)t)); }
	
	template< typename T > inline static void *iteratorBegin (void *t)
	{ return (void *)(new typename T::const_iterator (((T *)t)->constBegin ())); }
	
	template< typename T > inline static void *iteratorEnd (void *t)
	{ return (void *)(new typename T::const_iterator (((T *)t)->constEnd ())); }
	
	template< typename T > inline static void iteratorMove (void *t, int n)
	{ (*static_cast< typename T::const_iterator * > (t)) += n; }
	
	template< typename T > inline static int iteratorDistance (void *t, void *o) {
		typename T::const_iterator *l = static_cast< typename T::const_iterator * > (t);
		typename T::const_iterator *r = static_cast< typename T::const_iterator * > (o);
		return std::distance (*l, *r);
	}
	
	template< typename T > inline static void iteratorNext (void *t)
	{ ++(*static_cast< typename T::const_iterator * > (t)); }
	
	template< typename T > inline static void iteratorPrev (void *t)
	{ --(*static_cast< typename T::const_iterator * > (t)); }
	
	template< typename T > inline static QVariant iteratorKey (void *t, int type)
	{ return QVariant::fromValue (static_cast< typename T::const_iterator * > (t)->key ()); }
	
	template< typename T > inline static QVariant iteratorValue (void *t, int) {
		return QVariant::fromValue (*(*static_cast< typename T::const_iterator * > (t)));
	}
	
	template< typename T > inline static bool iteratorEqual (void *t, void *r) {
		typename T::const_iterator &left = *static_cast< typename T::const_iterator * > (t);
		typename T::const_iterator &right = *static_cast< typename T::const_iterator * > (r);
		return (left == right);
	}
	
	template< typename T > inline static int iteratorCount (void *t)
	{ return static_cast< T * > (t)->count (); }
	
	template< typename T, typename Key > inline static void *iteratorFindMap (void *t, const QVariant &what) {
		typename T::const_iterator it = static_cast< T * > (t)
						->find (*static_cast< const Key * >(what.constData ()));
		if (it == static_cast< T * > (t)->end ())
			return 0;
		return (void *)(new typename T::const_iterator (it));
	}
	
	template< typename T, typename Key > inline static void *iteratorFindList (void *t, const QVariant &what) {
		T *list = static_cast< T * > (t);
		const Key *item = static_cast< const Key * > (qMetaTypeId< Key > () == QMetaType::QVariant
							      ? &what : what.constData ());
		
		typename T::const_iterator it = list->begin ();
		typename T::const_iterator end = list->end ();
		
		if (qMetaTypeId< Key > () != QMetaType::QVariant) {
			for (; it != end; ++it) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
				if (equal (*it, what))
#else
				if ((*it).operator== (*item))
#endif
					return new typename T::const_iterator (it);
			}
		} else {
			for (; it != end; ++it) {
				if (equal (*it, *item))
					return new typename T::const_iterator (it);
			}
		}
		
		return 0;
	}
	
	/** @} */
	
	/** \internal Auto conversion function template. */
	template< typename From, typename To >
	static To *autoConvTempl (From *from)
	{ return new To (*from); }
	
	/**
	 * \internal
	 */
	static void registerIterator (int type, int valueType, int keyType,
				      IteratorDtorFunc dtor, IteratorCopyFunc copy,
				      IteratorStartEndFunc start, IteratorStartEndFunc end,
				      IteratorIncFunc increment, IteratorIncFunc decrement,
				      IteratorMoveFunc move, IteratorDistanceFunc distance,
				      IteratorValueFunc value, IteratorValueFunc key,
				      IteratorComparisonFunc equal,
				      IteratorCountFunc count, IteratorFindFunc find);
	
	/** \internal Helper for buildListImpl. */
	template< typename T > static inline const T &passThrough (const T &t) { return t; }
	
	// Simplifies usage for buildList.
#ifndef NURIA_NO_CHAR_ARRAY_TO_QSTRING
	static inline QString passThrough (const char *str) { return QString (str); }
#endif
	
	/** \internal Helper for buildList() */
	static inline void buildListImpl (QVariantList &) { }
	
	// 
	template< typename T, typename ... Items >
	static inline void buildListImpl (QVariantList &list, const T &cur, const Items & ... items) {
		list.append (QVariant::fromValue (passThrough (cur)));
		buildListImpl (list, items ...);
	}
	
	/** \internal Not used */
	Variant ();
	
};

}

#ifndef NURIA_NO_VARIANT_COMPARISON
inline bool operator== (const QVariant &left, const QVariant &right)
{ return Nuria::Variant::equal (left, right); }

inline bool operator!= (const QVariant &left, const QVariant &right)
{ return Nuria::Variant::notEqual (left, right); }

inline bool operator< (const QVariant &left, const QVariant &right)
{ return Nuria::Variant::lessThan (left, right); }

inline bool operator> (const QVariant &left, const QVariant &right)
{ return Nuria::Variant::greaterThan (left, right); }

inline bool operator<= (const QVariant &left, const QVariant &right)
{ return Nuria::Variant::lessEqualThan (left, right); }

inline bool operator>= (const QVariant &left, const QVariant &right)
{ return Nuria::Variant::greaterEqualThan (left, right); }
#endif

/**
 * Iterates over \a variant. Use it like Q_FOREACH.
 */
#define VARIANT_FOREACH(variable, variant) \
	for (Nuria::Variant::Iterator _niter_ = Nuria::Variant::begin (variant), \
	_nend_ = Nuria::Variant::end (variant); \
	_niter_ != _nend_; ++_niter_) \
	if (!(variable = _niter_.value()).isNull ())

#endif // NURIA_VARIANT_HPP
