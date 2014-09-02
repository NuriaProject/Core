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

#ifndef NURIA_CALLBACK_HPP
#define NURIA_CALLBACK_HPP

#include <type_traits>
#include <functional>

#include <QSharedData>
#include <QMetaType>
#include <QVariant>
#include <QList>

#include "essentials.hpp"
#include "variant.hpp"

namespace Nuria {
class CallbackPrivate;

/** \internal */
namespace CallbackHelper {
	template< int ... Index >
	struct IndexTuple {
		template< int N >
		using append = IndexTuple< Index ..., N >;
	};
	
	template< int Size >
	struct CreateIndexTuple {
		typedef typename CreateIndexTuple< Size - 1 >::type::template append< Size - 1 > type;
	};
	
	template< >
	struct CreateIndexTuple< 1 > {
		typedef IndexTuple< > type;
	};
	
	template< typename... Types >
	using buildIndexTuple = typename CreateIndexTuple< sizeof...(Types) + 1 >::type;
	
	// qMetaTypeId wrapper, which returns 0 for T = void.
	template< typename T >
	constexpr int typeId () { return qMetaTypeId< T > (); }
	
	template< >
	constexpr int typeId< void > () { return 0; }
	
	// Creates a raw list of type ids and values from 'args'
	template< typename T >
        void getArguments (void **list, int *types, int idx, T *cur) {
	        list[idx] = cur;
	        types[idx] = qMetaTypeId< typename std::remove_const< T >::type > ();
        }
	
	template< typename T, typename T2, typename ... Args >
	void getArguments (void **list, int *types, int idx,
			   T *cur, T2 *next, Args * ... args) {
		getArguments (list, types, idx, cur);
		getArguments (list, types, idx + 1, next, args ...);
	}
	
}

/**
 * \brief A modern style callback mechanism which can be bound to various method
 * types including slots.
 * 
 * The Nuria::Callback class can be used when developers don't want to use the
 * default signal/slots mechanism Qt provides for various reasons. Callback
 * is designed to mimic parts of the std::function methods which makes it easy
 * to use.
 * 
 * \note This class is \b explicitly \b shared. This means that changes to the
 * instance will \b not copy the data, but instead alter the instance directly.
 * 
 * \par Methods
 * Callback can be created out of all available method types - Essentially
 * everything that has a <i>operator()</i>. This includes:
 * 
 * - Static methods, including static member methods
 * - Class member methods
 * - QObject slots
 * - std::function
 * - Lambdas
 * 
 * For all of the above types Callback provides a convenient constructors with
 * the exception of lambdas (See fromLambda).
 * 
 * \par Argument and return types
 * Please note that \b all types which are passed to or from the callback must
 * be registered to the Qt Meta System using the Q_DECLARE_METATYPE macro.
 * 
 * Another point is that Callback will always try its best to invoke a method.
 * Passed arguments are checked if they match the type of the target method.
 * If a argument doesn't, Callback will try to use QVariant::convert to
 * convert the passed argument to the expected argument type. If this still
 * fails, it will \b construct a temporary instance of the expected type and
 * passes it instead, \b silently discarding the original argument.
 * 
 * It is also possible to use variadic callbacks. This means that on invocation,
 * all passed arguments are packed into a single QVariantList. This list is now
 * treated as it would've been passed as only argument, meaning that bound
 * variables are applied after this process. The placeholder \a _1 points to the
 * QVariantList.
 * 
 * \par Binding
 * Just like std::function Callback supports a mechanism to bind arguments to
 * a callback. Placeholders are also supported (Using _1, ..., _10).
 * 
 * \sa bind boundLambda
 * 
 * \par Behaviour with QObject slots
 * When a callback uses a slot as target method, it can be used for thread to
 * thread communication. If the slot returns \a void (nothing), then the slot
 * will be invoked using a Qt::QueuedConnection, that means, the caller won't
 * be blocked until the callee (The slot) has finished its operation. If the
 * callee returns something different than a \a void, then the caller will be
 * \b blocked until the callee finished and returned the result to the calling
 * thread.
 *  
 * \par Multi-threading
 * Callback is \b re-entrant. Do not change a instance from multiple threads
 * concurrently. Target methods will be invoked in the \a calling thread, when
 * not using a QObject slot as target.
 */
class NURIA_CORE_EXPORT Callback {
	
	// Forward declare helper structures
	struct TrampolineBase;
	template< typename Ret, typename ... Args > struct MethodHelper;
	template< typename Class, typename Ret, typename ... Args > struct MemberMethodHelper;
	template< typename FullType, typename Ret, typename ... Args > struct LambdaHelper;
	
public:
	
	/**
	 * Possible types of callbacks.
	 */
	enum Type {
		Invalid = 0, /// Invalid instance
		StaticMethod = 1, /// Invokes a static method
		MemberMethod = 2, /// Invokes a member method
		Slot = 3, /// Invokes a slot
		Lambda = 4 /// Invokes a lambda
	};
	
	/**
	 * Placeholders for bind().
	 */
	enum Placeholder {
		_1 = 0,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,
		_10
	};
	
	/** Constructs an invalid instance. */
	Callback ();
	
	/** Constructs a callback out of a slot. */
	explicit Callback (QObject *receiver, const char *slot, bool variadic = false);
	
	/** Copy constructor. */
	Callback (const Callback &other);
	
	/** Constructs a callback from a static method. */
	template< typename Ret, typename ... Args >
	Callback (Ret (*func)(Args ...), bool variadic = false)
		: d (0) { setCallback (func); setVariadic (variadic); }
	
	/** Constructs a callback from a member method. */
	template< typename Class, typename Ret, typename ... Args >
	Callback (Class *instance, Ret (Class::*func)(Args ...), bool variadic = false)
		: d (0) { setCallback (instance, func); setVariadic (variadic); }
	
	template< typename Ret, typename ... Args >
	Callback (const std::function< Ret(Args ...) > &func, bool variadic = false)
		: d (0) { setCallback (func); setVariadic (variadic); }
	
	/** Constructs a callback out of a lambda. */
	template< typename Lambda >
	static Callback fromLambda (Lambda l, bool variadic = false) {
		Callback cb = fromLambdaImpl (l, &Lambda::operator());
		cb.setVariadic (variadic);
		return cb;
	}
	
	/** Destructor. */
	~Callback ();
	
	/** Assignment operator. */
	Callback &operator= (const Callback &other);
	
	/**
	 * Comparison operator.
	 * 
	 * Returns \c true if both callbacks point to the very same method (and
	 * instance).
	 * 
	 * \warning Bound arguments aren't taken into account for this
	 * operation.
	 */
	bool operator== (const Callback &other) const;
	
	/** Returns \c true when this callback is valid i.e. callable. */
	bool isValid () const;
	
	/** Returns the type of this callback. */
	Type type () const;
	
	/** Returns if this callback is variadic. */
	bool isVariadic () const;
	
	/** Sets if this callback is variadic or not. */
	void setVariadic (bool variadic);
	
	/** Returns the return-type of the callback. */
	int returnType () const;
	
	/** Returns a list of arguments expected by this callback. */
	QList< int > argumentTypes () const;
	
	/** \addtogroup Callback setters
	 * @{
	 */
	
	// 
	template< typename Ret, typename ... Args >
	bool setCallback (Ret (*func)(Args ...)) {
		QList< int > args;
		buildArgList< Args ... > (args);
		return initBase (new MethodHelper< Ret, Args ... > (func),
				 CallbackHelper::typeId< Ret > (), args);
	}
	
	// Member methods
	template< typename Class, typename Ret, typename ... Args >
	bool setCallback (Class *instance, Ret (Class::*func)(Args ...)) {
		QList< int > args;
		buildArgList< Args ... > (args);
		return initBase (new MemberMethodHelper< Class, Ret, Args ... > (instance, func),
				 CallbackHelper::typeId< Ret > (), args);
	}
	
	// std::function
	template< typename Ret, typename ... Args >
	bool setCallback (std::function< Ret(Args ...) > func) {
		QList< int > args;
		buildArgList< Args ... > (args);
		return initBase (new LambdaHelper< std::function< Ret(Args ...) >, Ret, Args ... > (func),
				 CallbackHelper::typeId< Ret > (), args);
	}
	
	// QObject slot
	bool setCallback (QObject *receiver, const char *slot);
	
	// Convenience assignment operators
	template< typename Ret, typename ... Args >
	Callback &operator= (Ret (*func)(Args ...)) { setCallback (func); return *this; }
	
	/** @} */
	
	/**
	 * Binds arguments to the callback. This essentially works like
	 * std::bind(). If no placeholders are used additional arguments
	 * are appended to the final argument list when invoked. If
	 * placeholders are used only the arguments whose positions are set
	 * with a placeholder are passed, the others are silently discarded.
	 */
	void bindList (const QVariantList &arguments = QVariantList());
	
	/**
	 * Takes a lambda and returns a Callback with \a args already bound.
	 */
	template< typename Lambda, typename ... Args >
	static Callback boundLambda (Lambda func, const Args &... args) {
		Callback cb = fromLambdaImpl (func, &Lambda::operator());
		cb.bind (args ...);
		return cb;
	}
	
	/**
	 * \overload
	 * This method offers a more std::bind-like functionality.
	 * Use bindList if you have a QVariantList of these arguments.
	 */
	template< typename ... Args >
	inline Callback &bind (const Args &... args) {
		bindList (Variant::buildList (args ... ));
		return *this;
	}
	
	/**
	 * Invokes the callback using \a arguments. Use this method if you have
	 * the arguments themself as list.
	 * \sa operator()
	 */
	QVariant invoke (const QVariantList &arguments) const;
	
	/**
	 * Invokes the callback method, passing \a args.
	 */
	template< typename ... Args >
	QVariant operator() (const Args &... args) const {
		void *list[sizeof... (Args)];
		int types[sizeof... (Args)];
		CallbackHelper::getArguments (list, types, 0, const_cast< Args * > (&args) ...);
		return invoke (sizeof... (Args), list, types);
	}
	
	/** Invokes the callback without arguments. */
	inline QVariant operator() () {
		return invoke (0, 0, 0);
	}
	
private:
	
	friend class CallbackPrivate;
	
	QVariant invoke (int count, void **args, int *types) const;
	QVariant invokePrepared (const QVariantList &arguments) const;
	
	/** \internal */
	QVariant invokeInternal (int count, void **args, int *types) const;
	
	/** \internal Helper structure based on std::remove_reference<> */
	// If you have a error on line struct removeRef< T & >:
	// T& is not allowed for values. Use const T & or just T instead.
	template< typename T > struct removeRef { typedef T type; };
        template< typename T > struct removeRef< T & >; // Not allowed!
	template< typename T > struct removeRef< const T & > { typedef T type; };
	
	/**
	 * \internal
	 */
	template< typename Lambda, typename Ret, typename ... Args >
	inline static Callback fromLambdaImpl (Lambda l, Ret (Lambda::*)(Args ...) const) {
		return Callback (std::function< Ret(Args ...) > (l));
	}
	
	/**
	 * \internal
	 *  Initializes a native function.
	 */
	bool initBase (TrampolineBase *base, int retType, const QList< int > &args);
	
	template< typename T1, typename ... T2 >
	inline void buildArgListDo (QList< int > &list, T1 cur, T2 ... next) {
		list.append (cur);
		buildArgListDo (list, next ...);
	}
	
	template< typename T >
	inline void buildArgListDo (QList< int > &list, T cur)
	{ list.append (cur); }
	
	/** \overload No-op. */
	inline void buildArgListDo (QList< int > &) { }
	
	/**
	 * \internal
	 * This variadic template function populates \a list with the type ids
	 * of the typenames passed in as template.
	 */
	template< typename ... Args >
	inline void buildArgList (QList< int > &list) {
		buildArgListDo (list, qMetaTypeId< typename removeRef< Args >::type > () ...);
	}
	
	/**
	 * \internal
	 * Helper structure for creating trampoline functions needed to invoke
	 * functions.
	 */
	struct TrampolineBase {
		Type type;
		TrampolineBase (Type t) : type (t) {}
		
		virtual ~TrampolineBase () {}
		virtual void trampoline (void **) = 0;
	};
	
	struct MemberTrampolineBase : TrampolineBase {
		MemberTrampolineBase (void *inst) : TrampolineBase (MemberMethod), instance (inst) {}
		void *instance;
	};
	
	// Static methods
	template< typename Ret, typename ... Args >
	struct MethodHelper : public TrampolineBase {
		typedef Ret (*Prototype)(Args ...);
		Prototype func;
		
		MethodHelper (Prototype f) : TrampolineBase (StaticMethod), func (f) {}
		
		template< int ... Index >
		inline void trampolineImpl (void **args, CallbackHelper::IndexTuple< Index... >) {
			(*reinterpret_cast< typename removeRef< Ret >::type * > (args[0])) =
				func (*reinterpret_cast< typename removeRef< Args >::type * >(args[Index]) ...);
		}
		
		void trampoline (void **args)
		{ trampolineImpl (args, CallbackHelper::buildIndexTuple< Args ... > ()); }
		
	};
	
	template< typename ... Args >
	struct MethodHelper< void, Args ... > : public TrampolineBase {
		typedef void (*Prototype)(Args ...);
		Prototype func;
		
		MethodHelper (Prototype f) : TrampolineBase (StaticMethod), func (f) {}
		
		template< int ... Index >
		inline void trampolineImpl (void **args, CallbackHelper::IndexTuple< Index... >) {
			Q_UNUSED(args);
			func (*reinterpret_cast< typename removeRef< Args >::type * >(args[Index]) ...);
		}
		
		void trampoline (void **args)
		{ trampolineImpl (args, CallbackHelper::buildIndexTuple< Args ... > ()); }
		
	};
	
	// Member methods
	template< typename Class, typename Ret, typename ... Args >
	struct MemberMethodHelper : public MemberTrampolineBase {
		typedef Ret (Class::*Prototype)(Args ...);
		Prototype func;
		
		MemberMethodHelper (Class *inst, Prototype f) : MemberTrampolineBase (inst), func (f) {}
		
		template< int ... Index >
		inline void trampolineImpl (void **args, CallbackHelper::IndexTuple< Index... >) {
			(*reinterpret_cast< typename removeRef< Ret >::type * > (args[0])) =
					(reinterpret_cast< Class * > (instance)->*func)
					(*reinterpret_cast< typename removeRef< Args >::type * >(args[Index]) ...);
		}
		
		void trampoline (void **args)
		{ trampolineImpl (args, CallbackHelper::buildIndexTuple< Args ... > ()); }
		
	};
	
	template< typename Class, typename ... Args >
	struct MemberMethodHelper< Class, void, Args ... > : public MemberTrampolineBase {
		typedef void (Class::*Prototype)(Args ...);
		Prototype func;
		
		MemberMethodHelper (Class *inst, Prototype f) : MemberTrampolineBase (inst), func (f) {}
		
		template< int ... Index >
		inline void trampolineImpl (void **args, CallbackHelper::IndexTuple< Index... >) {
			Q_UNUSED(args);
			(reinterpret_cast< Class * > (instance)->*func)
				(*reinterpret_cast< typename removeRef< Args >::type * >(args[Index]) ...);
		}
		
		void trampoline (void **args)
		{ trampolineImpl (args, CallbackHelper::buildIndexTuple< Args ... > ()); }
		
	};
	
	// std::function< ... >
	template< typename FullType, typename Ret, typename ... Args >
	struct LambdaHelper : public TrampolineBase {
		FullType func;
		
		LambdaHelper (const FullType &f) : TrampolineBase (Lambda), func (f) {}
		
		template< int ... Index >
		inline void trampolineImpl (void **args, CallbackHelper::IndexTuple< Index... >) {
			(*reinterpret_cast< typename removeRef< Ret >::type * > (args[0])) =
				func (*reinterpret_cast< typename removeRef< Args >::type * >(args[Index]) ...);
		}
		
		void trampoline (void **args)
		{ trampolineImpl (args, CallbackHelper::buildIndexTuple< Args ... > ()); }
		
	};
	
	template< typename FullType, typename ... Args >
	struct LambdaHelper< FullType, void, Args ... > : public TrampolineBase {
		FullType func;
		
		LambdaHelper (const FullType &f) : TrampolineBase (Lambda), func (f) {}
		
		template< int ... Index >
		inline void trampolineImpl (void **args, CallbackHelper::IndexTuple< Index... >) {
			Q_UNUSED(args);
			func (*reinterpret_cast< typename removeRef< Args >::type * >(args[Index]) ...);
		}
		
		void trampoline (void **args)
		{ trampolineImpl (args, CallbackHelper::buildIndexTuple< Args ... > ()); }
		
	};
	
	// d-ptr
	CallbackPrivate *d;
	
};
}

// 
Q_DECLARE_METATYPE(Nuria::Callback)
Q_DECLARE_METATYPE(Nuria::Callback::Placeholder)

#endif // NURIA_CALLBACK_HPP
