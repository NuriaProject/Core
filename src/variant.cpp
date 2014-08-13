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

#define NURIA_VARIANT_COMPARISON
#include "nuria/variant.hpp"

#include <QReadWriteLock>
#include <QDateTime>
#include <QRegExp>
#include <QList>
#include <QDate>
#include <QTime>

// Helper structure which stores registered types
struct TypeInfo {
	
	struct Comp {
		Nuria::Variant::Comparison equal;
		Nuria::Variant::Comparison notEqual;
		Nuria::Variant::Comparison less;
		Nuria::Variant::Comparison greater;
		Nuria::Variant::Comparison lessEqual;
		Nuria::Variant::Comparison greaterEqual;
	};
	
	QMap< int, Comp > map;
	
};

struct ConversionInfo {
	
	QMap< int, Nuria::Variant::Conversion > map;
	
};

struct IterInfo {
	
	int keyType;
	int valueType;
	Nuria::Variant::IteratorDtorFunc destroy;
	Nuria::Variant::IteratorCopyFunc copy;
	Nuria::Variant::IteratorStartEndFunc begin;
	Nuria::Variant::IteratorStartEndFunc end;
	Nuria::Variant::IteratorIncFunc increment;
	Nuria::Variant::IteratorIncFunc decrement;
	Nuria::Variant::IteratorMoveFunc move;
	Nuria::Variant::IteratorDistanceFunc distance;
	Nuria::Variant::IteratorValueFunc value;
	Nuria::Variant::IteratorValueFunc key;
	Nuria::Variant::IteratorComparisonFunc equal;
	Nuria::Variant::IteratorCountFunc count;
	Nuria::Variant::IteratorFindFunc find;
	
};

typedef QMap< int, TypeInfo > TypeMap;
typedef QMap< int, TypeInfo::Comp > CompMap;
typedef QMap< int, ConversionInfo > ConversionMap;
typedef QMap< int, IterInfo * > IteratorMap;

struct VariantPrivate {
	QReadWriteLock compLock;
	QReadWriteLock convLock;
	QReadWriteLock iterLock;
	TypeMap comparisons;
	ConversionMap conversions;
	IteratorMap iterators;
	
};

// Private iterator data
namespace Nuria {
class VariantIteratorPrivate {
public:
	
	int genericPos;
	QVariant variant;
	void *container;
	void *iterator;
	IterInfo *info;
	
};
}

// For compareVariants()
enum CompareOperators {
	Equal, NotEqual,
	Less, Greater,
	LessEqual, GreaterEqual
};

// Generic comparison functions
template< typename T > static bool genericEqual (const T &left, const T &right)
{ return left == right; }

template< typename T > static bool genericNotEqual (const T &left, const T &right)
{ return left != right; }

template< typename T > static bool genericLess (const T &left, const T &right)
{ return left < right; }

template< typename T > static bool genericGreater (const T &left, const T &right)
{ return left > right; }

template< typename T > static bool genericLessEqual (const T &left, const T &right)
{ return left <= right; }

template< typename T > static bool genericGreaterEqual (const T &left, const T &right)
{ return left >= right; }

static bool genericFalse (const void *left, const void *right) {
	Q_UNUSED(left)
	Q_UNUSED(right)
	return false;
}

inline static void initHelperComplex (TypeMap &map, int t1, int t2, 
				      Nuria::Variant::Comparison equal,
				      Nuria::Variant::Comparison notEqual,
				      Nuria::Variant::Comparison less,
				      Nuria::Variant::Comparison greater,
				      Nuria::Variant::Comparison lessEqual,
				      Nuria::Variant::Comparison greaterEqual) {
	
	TypeInfo::Comp comp = { equal, notEqual, less, greater, lessEqual, greaterEqual };
	
	// 
	TypeInfo info;
	info.map.insert (t1, comp);
	map.insert (t2, info);
}

template< typename T >
inline static void initHelper (TypeMap &map, int type) {
	
	initHelperComplex (map, type, type,
			   (Nuria::Variant::Comparison)&genericEqual< T >,
			   (Nuria::Variant::Comparison)&genericNotEqual< T >,
			   (Nuria::Variant::Comparison)&genericLess< T >,
			   (Nuria::Variant::Comparison)&genericGreater< T >,
			   (Nuria::Variant::Comparison)&genericLessEqual< T >,
			   (Nuria::Variant::Comparison)&genericGreaterEqual< T >);
	
}

/**
 * Default comparison function. Returns \c true if the QString LHS matches
 * exactly to the QRegExp on the RHS.
 */
static bool compareStringToRegExp (const QString &lhs, const QRegExp &rhs) {
	return rhs.exactMatch (lhs);
}

// Hack so we can easily initialize data.
static VariantPrivate *g_initDPtr = 0;

static void init (VariantPrivate *d) {
	TypeMap &map = d->comparisons;
	g_initDPtr = d;
	
	// Fill with default comparisons
	initHelper< int > (map, QMetaType::Int);
	initHelper< unsigned int > (map, QMetaType::UInt);
	initHelper< qlonglong > (map, QMetaType::LongLong);
	initHelper< qulonglong > (map, QMetaType::ULongLong);
	initHelper< double > (map, QMetaType::Double);
	initHelper< QChar > (map, QMetaType::QChar);
	initHelper< QString > (map, QMetaType::QString);
	initHelper< QRegExp > (map, QMetaType::QRegExp);
	initHelper< QByteArray > (map, QMetaType::QByteArray);
	initHelper< QDate > (map, QMetaType::QDate);
	initHelper< QTime > (map, QMetaType::QTime);
	initHelper< QDateTime > (map, QMetaType::QDateTime);
	initHelper< void * > (map, QMetaType::VoidStar);
	initHelper< long > (map, QMetaType::Long);
	initHelper< short > (map, QMetaType::Short);
	initHelper< char > (map, QMetaType::Char);
	initHelper< unsigned long > (map, QMetaType::ULong);
	initHelper< unsigned short > (map, QMetaType::UShort);
	initHelper< unsigned char > (map, QMetaType::UChar);
	initHelper< float > (map, QMetaType::Float);
	initHelper< QObject * > (map, QMetaType::QObjectStar);
	
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
	initHelper< QObject * > (map, QMetaType::QWidgetStar);
#endif
	
	// Complex default comparisons
	// QString -> QRegExp.
	initHelperComplex (map, QMetaType::QString, QMetaType::QRegExp,
			   (Nuria::Variant::Comparison)&compareStringToRegExp,
			   0, &genericFalse, 0, 0, 0);
	
	// Default iterators
	Nuria::Variant::registerIterators< QVariantList > ();
	Nuria::Variant::registerIterators< QStringList > ();
	Nuria::Variant::registerIterators< QVariantMap > ();
	Nuria::Variant::registerIterators< QVariantHash > ();
	
	// Done.
	g_initDPtr = 0;
}

static VariantPrivate *d_ptr () {
	static VariantPrivate *p = 0;
	
	if (!p) {
		p = new VariantPrivate;
		init (p);
	}
	
	return p;
}

void Nuria::Variant::registerType (int type, int rightType, Nuria::Variant::Comparison equal,
				   Nuria::Variant::Comparison lessThan) {
	
	registerType (type, rightType, equal, 0, lessThan, 0, 0, 0);
	
}

void Nuria::Variant::registerType (int type, int rightType, Nuria::Variant::Comparison equal,
				   Nuria::Variant::Comparison notEqual, Nuria::Variant::Comparison less,
				   Nuria::Variant::Comparison greater, Nuria::Variant::Comparison lessEqual,
				   Nuria::Variant::Comparison greaterEqual) {
	
	//
	VariantPrivate *d = d_ptr ();
	
	// Acquire write lock
	QWriteLocker lock (&d->compLock);
	
	// Do we know this type already?
	TypeInfo info;
	TypeMap::ConstIterator it = d->comparisons.constFind (type);
	if (it != d->comparisons.constEnd ()) {
		info.map = it->map;
		
	}
	
	// Create structure
	TypeInfo::Comp comp;
	comp.equal = equal;
	comp.notEqual = notEqual;
	comp.less = less;
	comp.greater = greater;
	comp.lessEqual = lessEqual;
	comp.greaterEqual = greaterEqual;
	
//	qDebug("Register (%s -> %s, %p, %p, %p, %p, %p, %p)",
//	       QMetaType::typeName (type), QMetaType::typeName (rightType),
//	       equal, notEqual, less, greater, lessEqual, greaterEqual);
	
	info.map.insert (rightType, comp);
	
	// Store
	d->comparisons.insert (type, info);
	
}

// Helper class which deletes the guarded pointer if 'owned' is true
class QVariantDeleter {
public:
	QVariant *variant;
	bool owned;
	QVariantDeleter (QVariant *q) : variant (q), owned (false) {}
	~QVariantDeleter () {
		if (owned) delete variant;
	}
	
};

static bool compareVariants (CompareOperators op, const QVariant &left, const QVariant &right) {
	VariantPrivate *d = d_ptr ();
	
	// Check if op is (Not-)Equal and left and right are Qt types.
	// If so, we can skip the following stuff.
	if ((op == Equal || op == NotEqual) &&
	    left.userType () < QMetaType::User && 
	    right.userType () < QMetaType::User) {
		
		if (op == Equal) {
			return left.operator== (right);
		}
		
		return left.operator!= (right);
		
	}
	
	// Find custom type
	QReadLocker lock (&d->compLock);
	TypeMap::ConstIterator it = d->comparisons.constFind (left.userType ());
	if (it == d->comparisons.constEnd ()) {
		return false;
	}
	
	// Find match for right
	const CompMap &map = it->map;
	CompMap::ConstIterator itRight = map.constFind (right.userType ());
	QVariant *rightPtr = const_cast< QVariant * > (&right);
	QVariantDeleter guard (rightPtr);
	
	// Does left know about this type?
	if (itRight == map.constEnd ()) {
		bool useVariant = false;
		
		// Okay, doesn't work. Let's try to convert right to the type
		// of left!
		QVariant var;
		itRight = map.constFind (left.userType ());
		
		if (itRight != map.constEnd ()) {
			var = Nuria::Variant::convert (right, left.userType ());
		}
		
		if (!var.isValid ()) {
			// Damn. Well, we won't give up yet. We have probably
			// a few other tries to make before we give up. So, try
			// to convert right to something left can be compared to.
			CompMap::ConstIterator cur = map.constBegin ();
			CompMap::ConstIterator end = map.constEnd ();
			for (; cur != end && !var.isValid (); ++cur) {
				var = Nuria::Variant::convert (right, cur.key ());
			}
			
			// Succeeded?
			if (!var.isValid ()) {
				
				// One last try: Does left support QVariant as RHS?
				itRight = map.constFind (QMetaType::QVariant);
				if (itRight == map.constEnd ()) {
					// Okay, lets give up. We already wasted enough
					// time on this.
					return false;
					
				}
				
				// This actually works.
				useVariant = true;
				
			}
			
		}
		
		// Copy 'var' if we don't compare by a QVariant
		if (!useVariant) {
			rightPtr = new QVariant (var);
			guard.variant = rightPtr;
			guard.owned = true;
		}
		
	}
	
	// Store meta-data about the comparison
	const TypeInfo::Comp &compInfo = *itRight;
	
	// 
	const void *inst = left.constData ();
	const void *rhs = rightPtr;
	
	// Pass the constData() to the compare function if it doesn't
	// expect a QVariant as RHS.
	if (itRight.key () != QMetaType::QVariant) {
		rhs = rightPtr->constData ();
	}
	
	// Compare
	switch (op) {
	case Equal:
		return compInfo.equal (inst, rhs);
	case NotEqual:
		
		if (compInfo.notEqual) {
			return compInfo.notEqual (inst, rhs);
		}
		
		return !compInfo.equal (inst, rhs);
		
	case Less:
		return compInfo.less (inst, rhs);
	case Greater:
		if (compInfo.greater) {
			return compInfo.greater (inst, rhs);
		}
		
		return !compInfo.less (inst, rhs) && !compInfo.equal (inst, rhs);
		
	case LessEqual:
		if (compInfo.lessEqual) {
			return compInfo.lessEqual (inst, rhs);
		}
		
		return compInfo.less (inst, rhs) || compInfo.equal (inst, rhs);
		
	case GreaterEqual:
		if (compInfo.greaterEqual) {
			return compInfo.greaterEqual (inst, rhs);
		}
		
		return !compInfo.less (inst, rhs);
	}
	
	// Shouldn't happen
	return false;
	
}

bool Nuria::Variant::equal (const QVariant &left, const QVariant &right) {
	return compareVariants (Equal, left, right);
}

bool Nuria::Variant::notEqual (const QVariant &left, const QVariant &right) {
	return compareVariants (NotEqual, left, right);
}

bool Nuria::Variant::lessThan (const QVariant &left, const QVariant &right) {
	return compareVariants (Less, left, right);
}

bool Nuria::Variant::greaterThan (const QVariant &left, const QVariant &right) {
	return compareVariants (Greater, left, right);
}

bool Nuria::Variant::lessEqualThan (const QVariant &left, const QVariant &right) {
	return compareVariants (LessEqual, left, right);
}

bool Nuria::Variant::greaterEqualThan (const QVariant &left, const QVariant &right) {
	return compareVariants (GreaterEqual, left, right);
}

bool Nuria::Variant::canCompare (int leftType, int rightType) {
	
	// Check if op is (Not-)Equal and left and right are Qt types.
	// If so, we can skip the following stuff.
	if (leftType < QMetaType::User &&  rightType < QMetaType::User) {
		return true;
		
	}
	
	// Find custom type
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->compLock);
	TypeMap::ConstIterator it = d->comparisons.constFind (leftType);
	if (it == d->comparisons.constEnd ()) {
		return false;
	}
	
	// Find match for right
	const CompMap &map = it->map;
	CompMap::ConstIterator itRight = map.constFind (rightType);
	
	// Does left know about this type?
	if (itRight == map.constEnd ()) {
		
		// Could we somehow convert rightType?
		CompMap::ConstIterator cur = map.constBegin ();
		CompMap::ConstIterator end = map.constEnd ();
		for (; cur != end; ++cur) {
			
			// Can we convert rightType to 'cur'?
			// Also check if left accepts QVariants.
			if (cur.key () == QMetaType::QVariant || canConvert (rightType, cur.key ())) {
				return true;
			}
			
		}
		
		// Not possible
		return false;
		
	}
	
	// Possible comparasion
	return true;
	
}

// Helper function for convert() which tries to convert 'variant' to 'type'
// using the Qt/QVariant conversion mechanism (Implemented since Qt5.2)
inline static QVariant convertQt (QVariant variant, int type) {
	if (!variant.convert (type)) {
		return QVariant ();
	}
	
	return variant;
}

// Helper function for canConvert().
inline static bool canConvertQt (int from, int to) {
	
	// Only possible for Qt types
	if (from >= QMetaType::User && to >= QMetaType::User) {
		return false;
	}
	
	// Construct a QVariant with from type and ask canConvert()
	QVariant variant (from, (void *)0);
	return variant.canConvert (static_cast< QVariant::Type > (to));
	
}

QVariant Nuria::Variant::convert (const QVariant &variant, int type) {
	
	// If variant and type are the same do nothing
	if (variant.userType () == type || type == QMetaType::QVariant) {
		return variant;
	}
	
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->convLock);
	
	// Find conversion structure
	ConversionMap::ConstIterator it = d->conversions.constFind (variant.userType ());
	
	if (it == d->conversions.constEnd ()) {
		// Not found.
		return convertQt (variant, type);
	}
	
	// Find conversion function
	Nuria::Variant::Conversion func = it->map.value (type, 0);
	
	// Not found?
	if (!func) {
		return convertQt (variant, type);
	}
	
	// Invoke
	void *instance = func (variant.constData ());
	
	// Return QVariant containing the instance
	QVariant result (type, instance);
	
	// Destroy instance
	QMetaType::destroy (type, instance);
	
	// Done.
	return result;
	
}

bool Nuria::Variant::canConvert (int fromType, int toType) {
	
	// Same type?
	if (fromType == toType) {
		return true;
	}
	
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->convLock);
	
	// Find conversion structure
	ConversionMap::ConstIterator it = d->conversions.constFind (fromType);
	
	if (it == d->conversions.constEnd ()) {
		// Not found.
		return canConvertQt (fromType, toType);
	}
	
	// 
	return it->map.contains (toType);
	
}

void Nuria::Variant::registerConversion (int from, int to, Nuria::Variant::Conversion func) {
	
	VariantPrivate *d = d_ptr ();
	QWriteLocker lock (&d->convLock);
	
	// Do we know 'from'?
	ConversionInfo info;
	ConversionMap::ConstIterator it = d->conversions.constFind (from);
	
	if (it != d->conversions.constEnd ()) {
		info.map = it->map;
	}
	
	// 
	info.map.insert (to, func);
	
	// Store
	d->conversions.insert (from, info);
	
}

void Nuria::Variant::registerIterator (int type, int valueType, int keyType,
				       IteratorDtorFunc dtor,
				       IteratorCopyFunc copy,
				       IteratorStartEndFunc start,
				       IteratorStartEndFunc end,
				       IteratorIncFunc increment,
				       IteratorIncFunc decrement,
				       IteratorMoveFunc move,
				       IteratorDistanceFunc distance,
				       IteratorValueFunc value,
				       IteratorValueFunc key,
				       IteratorComparisonFunc equal,
				       IteratorCountFunc count,
				       IteratorFindFunc find) {
	
	// Lock
	VariantPrivate *d = (!g_initDPtr) ? d_ptr () : g_initDPtr;
	QWriteLocker lock (&d->iterLock);
	
	// Insert
	IterInfo *info = new IterInfo;
	info->keyType = keyType;
	info->valueType = valueType;
	info->destroy = dtor;
	info->copy = copy;
	info->begin = start;
	info->end = end;
	info->increment = increment;
	info->decrement = decrement;
	info->move = move;
	info->distance = distance;
	info->value = value;
	info->key = key;
	info->equal = equal;
	info->count = count;
	info->find = find;
	
	d->iterators.insert (type, info);
	
	// Done.
	
}


Nuria::Variant::Iterator::Iterator () : d_ptr (0) {
	// 
}

Nuria::Variant::Iterator::Iterator (const Nuria::Variant::Iterator &other) {
	this->d_ptr = new Nuria::VariantIteratorPrivate (*other.d_ptr);
	
	// Copy iterator instance
	if (this->d_ptr->info && this->d_ptr->iterator) {
		this->d_ptr->iterator = this->d_ptr->info->copy (this->d_ptr->iterator);
	}
	
}

Nuria::Variant::Iterator::Iterator (const QVariant &variant, void *iter, void *info, int pos) {
	this->d_ptr = new Nuria::VariantIteratorPrivate ();
	this->d_ptr->genericPos = pos;
	
	this->d_ptr->info = static_cast< IterInfo * > (info);
	this->d_ptr->iterator = iter;
	this->d_ptr->variant = variant;
	this->d_ptr->container = const_cast< void * > (variant.constData ());
	
}

Nuria::Variant::Iterator::~Iterator () {
	if (this->d_ptr) {
		if (this->d_ptr->iterator) {
			this->d_ptr->info->destroy (this->d_ptr->iterator);
		}
		
		delete this->d_ptr;
	}
	
}

bool Nuria::Variant::Iterator::isValid () const {
	return (this->d_ptr != 0);
}

const Nuria::Variant::Iterator &Nuria::Variant::Iterator::operator= (const Nuria::Variant::Iterator &other) {
	
	// Sanity check
	if (other.d_ptr == this->d_ptr)
		return *this;
	
	// Remove old data
	if (this->d_ptr) {
		this->d_ptr->info->destroy (this->d_ptr->iterator);
		delete this->d_ptr;
	}
	
	// Copy new data
	this->d_ptr = new Nuria::VariantIteratorPrivate (*other.d_ptr);
	
	// Copy iterator instance
	if (this->d_ptr->info && this->d_ptr->iterator) {
		this->d_ptr->iterator = this->d_ptr->info->copy (this->d_ptr->iterator);
	}
	
	// 
	return *this;
}

bool Nuria::Variant::Iterator::operator== (const Nuria::Variant::Iterator &other) const {
	if (!this->d_ptr && !other.d_ptr)
		return true;
	
	// Comparison between invalid iterators
	if ((!this->d_ptr && other.d_ptr) || (this->d_ptr && !other.d_ptr))
		return false;
	
	// Compare the iterated variants
	if (this->d_ptr->container != other.d_ptr->container)
		return false;
	
	// 
	if (this->d_ptr->info != other.d_ptr->info)
		return false;
	
	// Generic comparison
	if (!this->d_ptr->iterator) {
		return (other.d_ptr->iterator == 0 && this->d_ptr->genericPos == other.d_ptr->genericPos);
	}
	
	// Thou shall no follow the NULL pointer
	if (!other.d_ptr->iterator) {
		return false;
	}
	
	// 
//	qDebug("Comparing QVariants %p and %p", this->d_ptr->variant.constData (), other.d_ptr->variant.constData ());
	return this->d_ptr->info->equal (this->d_ptr->iterator, other.d_ptr->iterator);
	
}

Nuria::Variant::Iterator &Nuria::Variant::Iterator::operator++ () {
	if (!this->d_ptr)
		return *this;
	
	// 
	if (this->d_ptr->iterator) {
		this->d_ptr->info->increment (this->d_ptr->iterator);
	} else {
		this->d_ptr->genericPos++;
	}
	
	return *this;
}

Nuria::Variant::Iterator &Nuria::Variant::Iterator::operator-- () {
	if (!this->d_ptr)
		return *this;
	
	// 
	if (this->d_ptr->iterator) {
		this->d_ptr->info->decrement (this->d_ptr->iterator);
	} else if (this->d_ptr->genericPos > 0) {
		this->d_ptr->genericPos--;
	}
	
	return *this;
}

const Nuria::Variant::Iterator &Nuria::Variant::Iterator::operator+= (int n) {
	if (!this->d_ptr)
		return *this;
	
	if (this->d_ptr->iterator) {
		this->d_ptr->info->move (this->d_ptr->iterator, n);
		
	} else {
		
		if (n < 0) {
			return operator-= (n);
		}
		
		this->d_ptr->genericPos += n;
	}
	
	return *this;
}

const Nuria::Variant::Iterator &Nuria::Variant::Iterator::operator-= (int n) {
	if (!this->d_ptr || !this->d_ptr->iterator || !this->d_ptr->info)
		return *this;
	
	if (this->d_ptr->iterator) {
		this->d_ptr->info->move (this->d_ptr->iterator, -n);
		
	} else {
		
		this->d_ptr->genericPos = (this->d_ptr->genericPos - n < 0)
					  ? 0 : this->d_ptr->genericPos - n;
		
	}
	
	return *this;
}

Nuria::Variant::Iterator Nuria::Variant::Iterator::operator+ (int n) const {
	if (!this->d_ptr || !this->d_ptr->iterator || !this->d_ptr->info)
		return Nuria::Variant::Iterator ();
	
	if (this->d_ptr->iterator) {
		void *iter = this->d_ptr->info->copy (this->d_ptr->iterator);
		this->d_ptr->info->move (iter, n);
		
		return Nuria::Variant::Iterator (this->d_ptr->variant, iter,
						 this->d_ptr->info, 0);
	}
	
	// Generic
	if (n < 0) {
		return operator- (n);
	}
	
	return Nuria::Variant::Iterator (this->d_ptr->variant, 0, 0, this->d_ptr->genericPos + n);
	
}

Nuria::Variant::Iterator Nuria::Variant::Iterator::operator- (int n) const {
	if (!this->d_ptr || !this->d_ptr->iterator || !this->d_ptr->info)
		return Nuria::Variant::Iterator ();
	
	if (this->d_ptr->iterator) {
		void *iter = this->d_ptr->info->copy (this->d_ptr->iterator);
		this->d_ptr->info->move (iter, -n);
		
		return Nuria::Variant::Iterator (this->d_ptr->variant, iter, this->d_ptr->info, 0);
	}
	
	// Generic
	if (this->d_ptr->genericPos - n < 0) {
		n = -this->d_ptr->genericPos;
	}
	
	// 
	return Nuria::Variant::Iterator (this->d_ptr->variant, 0, 0, this->d_ptr->genericPos - n);
	
}

int Nuria::Variant::Iterator::operator- (const Nuria::Variant::Iterator &other) const {
	if (!this->d_ptr || !other.d_ptr)
		return 0;
	
	// Same variants?
	if (!this->d_ptr->variant.operator== (other.d_ptr->variant))
		return 0;
	
	// Both types must be either generic or non-generic.
	if ((this->d_ptr->iterator && !other.d_ptr->iterator) ||
	    (!this->d_ptr->iterator && other.d_ptr->iterator)) {
		return 0;
	}
	
	// Iterators?
	if (this->d_ptr->iterator) {
		return this->d_ptr->info->distance (this->d_ptr->iterator,
						    const_cast< void * > (other.d_ptr->iterator));
		
	}
	
	// Generic
	return this->d_ptr->genericPos - other.d_ptr->genericPos;
	
}

QVariant Nuria::Variant::Iterator::key () const {
	if (!this->d_ptr || !this->d_ptr->iterator || !this->d_ptr->info || !this->d_ptr->info->key)
		return QVariant ();
	
	return this->d_ptr->info->key (this->d_ptr->iterator, this->d_ptr->info->keyType);
}

QVariant Nuria::Variant::Iterator::value () const {
	if (!this->d_ptr)
		return QVariant ();
	
	if (this->d_ptr->iterator) {
		return this->d_ptr->info->value (this->d_ptr->iterator, this->d_ptr->info->valueType);
	}
	
	// Generic
	if (this->d_ptr->genericPos == 0) {
		return this->d_ptr->variant;
	}
	
	return QVariant ();
	
}

static inline void *getIterator (const void *data, int typeId, bool end, IterInfo *&info) {
	
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->iterLock);
	
	// Find info
	info = d->iterators.value (typeId);
	void *iter = 0;
	
	// If this type is known construct iterator
	if (info) {
		iter = (end) ? info->end (const_cast< void * > (data)) :
			       info->begin (const_cast< void * > (data));
	}
	
	// Done.
	return iter;
}

Nuria::Variant::Iterator Nuria::Variant::begin (QVariant &variant) {
	IterInfo *info = 0;
	void *iter = getIterator (variant.constData (), variant.userType (), false, info);
	
	return Nuria::Variant::Iterator (variant, iter, info, variant.isValid () ? 0 : 1);
}

Nuria::Variant::Iterator Nuria::Variant::end (QVariant &variant) {
	IterInfo *info = 0;
	void *iter = getIterator (variant.constData (), variant.userType (), true, info);
	
	if (iter) {
		return Nuria::Variant::Iterator (variant, iter, info, 0);
	}
	
	// Return generic end iterator
	return Nuria::Variant::Iterator (variant, nullptr, nullptr, 1);
}

Nuria::Variant::Iterator Nuria::Variant::find (QVariant &variant, QVariant ident) {
	
	if (!variant.isValid ())
		return Iterator ();
	
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->iterLock);
	
	IterInfo *info = d->iterators.value (variant.userType ());
	
	// Generic type?
	if (!info) {
		if (equal (variant, ident)) {
			return Iterator (variant, nullptr, nullptr, 0);
		}
		
		return Iterator ();
	}
	
	// List or map
	// Convert 'ident' if needed.
	if (info->key && info->keyType != ident.userType ()) {
		ident = convert (ident, info->keyType);
		
	} else if (!info->key && info->valueType != ident.userType () &&
		   info->valueType != QMetaType::QVariant) {
		ident = convert (ident, info->valueType);
		
	}
	
	// Success?
	if (!ident.isValid ()) {
		return Iterator ();
	}
	
	// Find
	void *iter = info->find (const_cast< void * > (variant.constData ()), ident);
	
	// Found?
	if (iter) {
		return Iterator (variant, iter, info, -1);
	}
	
	// Not found.
	return Iterator ();
}

int Nuria::Variant::itemCount (const QVariant &variant) {
	if (!variant.isValid ())
		return 0;
	
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->iterLock);
	
	IterInfo *info = d->iterators.value (variant.userType ());
	
	// Generic type?
	if (!info) {
		return 1;
	}
	
	// List or map
	return info->count (const_cast< void * > (variant.constData ()));
}

bool Nuria::Variant::isList (const QVariant &variant) {
	return isList (variant.userType ());
}

bool Nuria::Variant::isList (int typeId) {
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->iterLock);
	
	IterInfo *info = d->iterators.value (typeId);
	return (info && info->key == 0);
}

bool Nuria::Variant::isMap (const QVariant &variant) {
	return isMap (variant.userType ());
}

bool Nuria::Variant::isMap (int typeId) {
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->iterLock);
	
	IterInfo *info = d->iterators.value (typeId);
	return (info && info->key != 0);
}

bool Nuria::Variant::isGeneric (const QVariant &variant) {
	return isGeneric (variant.userType ());
}

bool Nuria::Variant::isGeneric (int typeId) {
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->iterLock);
	
	return !d->iterators.contains (typeId);
}

QVariantList Nuria::Variant::toList (QVariant variant) {
	
	// Valid?
	if (!variant.isValid ()) {
		return QVariantList ();
	}
	
	// Iterators
	Iterator it = begin (variant);
	Iterator e = end (variant);
	
	QVariantList list;
	list.reserve (itemCount (variant));
	
	// Copy values
	for (; it != e; ++it) {
		list.append (*it);
	}
	
	// Done.
	return list;
}

QVariantMap Nuria::Variant::toMap (QVariant variant) {
	
	// Lock
	VariantPrivate *d = d_ptr ();
	QReadLocker lock (&d->iterLock);
	
	IterInfo *info = d->iterators.value (variant.userType ());
	
	// Not a map-type?
	if (!info || info->key == 0) {
		return QVariantMap ();
	}
	
	// Don't deadlock
	lock.unlock ();
	
	// Make sure we can convert the key-type to a QString
	if (!canConvert (info->key, QMetaType::QString)) {
		return QVariantMap ();
	}
	
	// Iterators
	Iterator it = begin (variant);
	Iterator e = end (variant);
	
	QVariantMap map;
	
	// Copy items. Make sure that we insertMulti, as the user
	// *may* use a multi map.
	for (; it != e; ++it) {
		map.insertMulti (toValue< QString > (it.key ()), it.value ());
	}
	
	// Done.
	return map;
	
}

void *Nuria::Variant::stealPointer (QVariant &variant) {
	QVariant::Private &data = variant.data_ptr ();
	void *result = data.data.ptr;
	
	if (data.is_shared) {
		return nullptr;
	}
	
	// Clear variant.
	data.is_null = true;
	data.type = QVariant::Invalid;
	data.data.ptr = nullptr;
	
	return result;
}
