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

#include "nuria/directoryresource.hpp"

Nuria::DirectoryResource::DirectoryResource (QObject *parent)
        : Resource (parent)
{
	
}

Nuria::DirectoryResource::~DirectoryResource () {
	
}

QString Nuria::DirectoryResource::interfaceName () const {
	return QStringLiteral("Directory");
}

Nuria::Invocation Nuria::DirectoryResource::invokeImpl (const QString &slot, const QVariantMap &arguments,
                                                        InvokeCallback callback, int timeout) {
	if (slot == "list") {
		return list (callback, timeout);
	} else if (slot == "get") {
		QVariant nameVariant = arguments.value (QStringLiteral("name"));
		
		if (nameVariant.userType () != QMetaType::QString) {
			throw ResourceHandlerException (BadArgumentError);
		}
		
		return get (nameVariant.toString (), callback, timeout);
	}
	
	return Resource::invokeImpl (slot, arguments, callback, timeout);
}

Nuria::Invocation Nuria::DirectoryResource::properties (InvokeCallback callback, int timeout) {
	Q_UNUSED(timeout);
	
	static PropertyList list {
		{ Property::Slot, QStringLiteral("list"), { }, qMetaTypeId< DirectoryEntries > () },
		{ Property::Slot, QStringLiteral("get"), {
				{ QStringLiteral("name"), QMetaType::QString }
			}, qMetaTypeId< ResourcePointer > () },
	};
	
	// 
	callback (Success, QVariant::fromValue (list));
	return Invocation (this);
}
