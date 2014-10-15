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

#include "nuria/dependencymanager.hpp"
#include "nuria/debug.hpp"

#include <QCoreApplication>
#include <QThreadStorage>
#include <QMetaType>
#include <QMutex>
#include <QMap>

// 
struct Dependency {
	int type;
	void *object;
};

typedef std::function< QObject *() > Creator;
typedef QMap< QString, Dependency > DependencyMap;
typedef QMap< QString, Creator > CreatorMap;

// 
#define GUARD_BEGIN \
	policy = (policy == DefaultPolicy) ? this->d_ptr->policy : policy; \
	DependencyMap &map = (policy == ThreadLocal) ? this->d_ptr->local.localData () : this->d_ptr->objects; \
	if (policy == ApplicationGlobal) { \
		this->d_ptr->mutex.lock (); \
	}

#define GUARD_END \
	if (policy == ApplicationGlobal) { \
		this->d_ptr->mutex.unlock (); \
	}

namespace Nuria {
class DependencyManagerPrivate {
public:
	
	DependencyManagerPrivate () : mutex (QMutex::Recursive) { }
	
	DependencyManager::ThreadingPolicy policy;
	
	QMutex mutex; // ApplicationGlobal
	DependencyMap objects; // ApplicationGlobal and SingleThread
	CreatorMap creators;
	QThreadStorage< DependencyMap > local; // ThreadLocal
	
};
}

Nuria::DependencyManager *Nuria::DependencyManager::instance () {
	static DependencyManager *instance = nullptr;
	
	if (!instance) {
		instance = new DependencyManager (qApp);
	}
	
	return instance;
}

Nuria::DependencyManager::ThreadingPolicy Nuria::DependencyManager::defaultThreadingPolicy () const {
	return this->d_ptr->policy;
}

void Nuria::DependencyManager::setDefaultThreadingPolicy (Nuria::DependencyManager::ThreadingPolicy policy) {
	this->d_ptr->policy = policy;
}

Nuria::DependencyManager::DependencyManager (QObject *parent)
	: QObject (parent), d_ptr (new DependencyManagerPrivate)
{
	
	this->d_ptr->policy = ThreadLocal;
	connect (qApp, SIGNAL(aboutToQuit()), SLOT(freeAllObjects()));
	
}

Nuria::DependencyManager::~DependencyManager () {
	
}

static void *createObjectByType (int type) {
	if (type == -1) {
		return nullptr;
	}
	
	// QObject type?
	const QMetaObject *meta = QMetaType::metaObjectForType (type);
	if (!meta) {
		nWarn() << "Type" << type << QMetaType::typeName (type) << "is not a QObject!";
		return nullptr;
	}
	
	// 
	return meta->newInstance ();
	
}

static void *getObjectFromMap (DependencyMap &map, const CreatorMap &creators, const QString &name, int type) {
	auto it = map.constFind (name);
	if (it != map.constEnd ()) {
		if (type != -1 && it->type != type)
			return nullptr;
		return it->object;
		
	}
	
	// Construct object
	void *instance = nullptr;
	auto creator = creators.find (name);
	if (creator != creators.end ()) {
		 instance = (*creator) ();
	} else { // Try to invoke constructor
		instance = createObjectByType (type);
	}
	
	// Store and return
	if (instance)
		map.insert (name, { type, instance });
	return instance;
	
}

void *Nuria::DependencyManager::objectByName (const QString &name, int type, ThreadingPolicy policy) {
	GUARD_BEGIN;
	void *ptr = getObjectFromMap (map, this->d_ptr->creators, name, type);
	GUARD_END;
	
	return ptr;
}

int Nuria::DependencyManager::objectType (const QString &name, ThreadingPolicy policy) const {
	int type = -1;
	
	GUARD_BEGIN;
	auto it = map.constFind (name);
	if (it != map.constEnd ()) {
		type = it->type;
	}
	GUARD_END;
	
	return type;
}

void Nuria::DependencyManager::storeObject (const QString &name, void *object,
					    int type, ThreadingPolicy policy) {
	GUARD_BEGIN;
	map.insert (name, { type, object });
	GUARD_END;
}

void Nuria::DependencyManager::setCreator (const QString &name, const std::function< QObject *() > &creator) {
	this->d_ptr->creators.insert (name, creator);
}

void Nuria::DependencyManager::freeAllObjects () {
	
	for (const Dependency &cur : this->d_ptr->objects) {
		delete static_cast< QObject * > (cur.object);
	}
	
}
