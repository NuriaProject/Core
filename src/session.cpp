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

#include "nuria/session.hpp"

#include <QVariantMap>

#include "nuria/abstractsessionmanager.hpp"

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

void Nuria::Session::insert (const QString &key, const QVariant &value) {
	this->d->dirty = true;
	this->d->data.insert (key, value);
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
