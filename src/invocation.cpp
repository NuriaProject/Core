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

#include "nuria/invocation.hpp"

#include <QPointer>

#include "nuria/resource.hpp"

namespace Nuria {
class InvocationPrivate : public QSharedData {
public:
	
	InvocationPrivate (Nuria::Resource *res, const QSharedPointer< Invocation::Interface > &inter)
	        : resource (res), interface (inter)
	{}
	
	QPointer< Resource > resource;
	QSharedPointer< Invocation::Interface > interface;
};
}

Nuria::Invocation::Invocation () {
	// 
}

Nuria::Invocation::Invocation (Nuria::Resource *processingResource, const QSharedPointer< Interface > &interface)
        : d (new InvocationPrivate (processingResource, interface))
{
	
}

Nuria::Invocation::Invocation (const Nuria::Invocation &other)
        : d (other.d)
{
	
}

Nuria::Invocation &Nuria::Invocation::operator= (const Nuria::Invocation &other) {
	this->d = other.d;
	return *this;
}

Nuria::Invocation::~Invocation () {
	// 
}

Nuria::Resource *Nuria::Invocation::resource () const {
	return (this->d) ? this->d->resource.data () : nullptr;
	
}

void Nuria::Invocation::cancel () {
	if (this->d && this->d->resource && this->d->interface) this->d->interface->cancel ();
}
