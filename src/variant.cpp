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

#include "nuria/variant.hpp"

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

void *Nuria::Variant::getPointer (QVariant &variant) {
	return const_cast< void * > (getPointer (static_cast< const QVariant & > (variant)));
}

const void *Nuria::Variant::getPointer (const QVariant &variant) {
	const QVariant::Private &data = variant.data_ptr ();
	if (data.is_null || data.type == QMetaType::UnknownType || data.type < uint(QVariant::Char)) {
		return nullptr;
	}
	
	if (data.is_shared) {
		return data.data.shared->ptr;
	}
	
	return data.data.ptr;
}
