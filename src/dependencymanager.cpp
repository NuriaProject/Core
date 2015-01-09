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

#include "nuria/dependencymanager.hpp"
#include "nuria/debug.hpp"

#include <QCoreApplication>
#include <QThreadStorage>
#include <QMetaType>
#include <QMutex>
#include <QMap>

// 
struct Instance {
	int type;
	void *object;
};

typedef std::function< QObject *() > Creator;
typedef QMap< QByteArray, Instance > DependencyMap;
typedef QMap< QByteArray, Creator > CreatorMap;

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
	// 
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

static void *getObjectFromMap (DependencyMap &map, const CreatorMap &creators, const QByteArray &name, int type) {
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

void *Nuria::DependencyManager::objectByName (const QByteArray &name, int type, ThreadingPolicy policy) {
	GUARD_BEGIN;
	void *ptr = getObjectFromMap (map, this->d_ptr->creators, name, type);
	GUARD_END;
	
	return ptr;
}

int Nuria::DependencyManager::objectType (const QByteArray &name, ThreadingPolicy policy) const {
	int type = -1;
	
	GUARD_BEGIN;
	auto it = map.constFind (name);
	if (it != map.constEnd ()) {
		type = it->type;
	}
	GUARD_END;
	
	return type;
}

void Nuria::DependencyManager::storeObject (const QByteArray &name, void *object,
					    int type, ThreadingPolicy policy) {
	GUARD_BEGIN;
	map.insert (name, { type, object });
	GUARD_END;
}

void Nuria::DependencyManager::setCreator (const QByteArray &name, const std::function< QObject *() > &creator) {
	this->d_ptr->creators.insert (name, creator);
}

void Nuria::DependencyManager::freeAllObjects () {
	
	for (const Instance &cur : this->d_ptr->objects) {
		delete static_cast< QObject * > (cur.object);
	}
	
}
