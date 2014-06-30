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

#include "session.hpp"

#include <QVariantMap>

#include "abstractsessionmanager.hpp"

class Nuria::SessionPrivate : public QSharedData {
public:
	QByteArray id;
	QVariantMap data;
	bool dirty = false;
	AbstractSessionManager *manager = nullptr;
};

Nuria::Session::Session ()
	: d (new SessionPrivate)
{
}

Nuria::Session::Session (const QByteArray &id, AbstractSessionManager *manager)
	: d (new SessionPrivate)
{
	d->id = id;
	d->dirty = false; 
	d->manager = manager;
}

Nuria::Session::Session (const Session &other)
	: d (other.d)
{
}

Nuria::Session::~Session () {
	// 
}

bool Nuria::Session::isValid () const {
	if (this->d->id.isEmpty () || this->d->manager == nullptr) {
		return false;
	}
	
	return true;
}

QByteArray Nuria::Session::id () const {
	return this->d->id;
}

Nuria::AbstractSessionManager *Nuria::Session::manager () const {
	return this->d->manager;
}

bool Nuria::Session::isDirty () const {
	return this->d->dirty;
}

void Nuria::Session::markDirty () {
	this->d->dirty = true;
}

void Nuria::Session::markClean () {
	this->d->dirty = false;
}

void Nuria::Session::remove () {
	this->d->manager->removeSession (d->id);
}

int Nuria::Session::refCount () const {
	return this->d->ref.load();
}

QVariant Nuria::Session::value (const QString &key) const {
	return this->d->data.value (key);
}

bool Nuria::Session::contains (const QString &key) const {
	return this->d->data.contains (key);
}

QVariant &Nuria::Session::operator[] (const QString &key) {
	this->d->dirty = true;
	return this->d->data[key];
}

QVariant Nuria::Session::operator[] (const QString &key) const {
	return this->value (key);
}

Nuria::Session &Nuria::Session::operator= (const Session &other) {
	this->d = other.d; 
	return *this;
}

bool Nuria::operator== (const Session &a, const Session &b) {
	return (a.d == b.d);
}

bool Nuria::operator!= (const Session &a, const Session &b) {
	return (a.d != b.d);
}
