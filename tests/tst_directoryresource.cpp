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

#include <QtTest>
#include <nuria/directoryresource.hpp>

#include <nuria/callback.hpp>

using namespace Nuria;

class TestResource : public DirectoryResource {
	Q_OBJECT
public:
	class Foo : public Resource {
	public:
		QString iface;
		
		QString interfaceName () const {
			return iface;
		}
		
		Invocation properties (InvokeCallback, int) {
			return Invocation ();
		}
		
	};
	
	Foo returnedResource;
	Invocation list (InvokeCallback callback, int timeout) {
		Q_UNUSED(timeout);
		callback (Success, QVariant::fromValue (DirectoryEntries { { "foo", "bar" } }));
		return Invocation (this);
	}
	
	Invocation get (const QString &name, InvokeCallback callback, int timeout) {
		Q_UNUSED(timeout);
		
		returnedResource.iface = name;
		callback (Success, QVariant::fromValue (ResourcePointer (&returnedResource)));
		
		return Invocation (this);
	}
	
};

class DirectoryResourceTest : public QObject {
	Q_OBJECT
private slots:
	
	void interfaceNameIsDirectory ();
	void propertiesAreAccordingToSpec ();
	void invokeList ();
	void invokeGetStringArgument ();
	void invokeGetBadArgument ();
	void invokeGetNoArgument ();
	
};



void DirectoryResourceTest::interfaceNameIsDirectory () {
	TestResource resource;
	QCOMPARE(resource.interfaceName (), QString("Directory"));
}

template< typename T >
std::function< void(Nuria::Resource::InvokeResultState, QVariant) > capture (T &variable) {
	return [&variable](Nuria::Resource::InvokeResultState, const QVariant &v) {
		variable = v.value< T > ();
	};
}

template< typename T >
std::function< void(Nuria::Resource::InvokeResultState, QVariant) >
capture (Nuria::Resource::InvokeResultState &state, T &variable) {
	return [&state, &variable](Nuria::Resource::InvokeResultState s, const QVariant &v) {
		state = s;
		variable = v.value< T > ();
	};
}

void DirectoryResourceTest::propertiesAreAccordingToSpec () {
	TestResource resource;
	Resource::PropertyList list;
	
	
	Resource::PropertyList expected {
		{ Resource::Property::Slot, "list", { }, qMetaTypeId< DirectoryEntries > () },
		{ Resource::Property::Slot, "get", {
				{ "name", QMetaType::QString }
			}, qMetaTypeId< ResourcePointer > () }
	};
	
	resource.invoke ("", { }, capture (list));
	QCOMPARE(list, expected);
}

void DirectoryResourceTest::invokeList () {
	TestResource resource;
	DirectoryEntries map;
	
	resource.invoke ("list", { }, capture (map));
	
	QCOMPARE(map, DirectoryEntries ({ { "foo", "bar" } }));
}

void DirectoryResourceTest::invokeGetStringArgument () {
	Resource::InvokeResultState state;
	TestResource resource;
	ResourcePointer ptr;
	
	resource.invoke ("get", { { "name", "Something" } }, capture (state, ptr));
	
	QVERIFY(!ptr.isNull ());
	QCOMPARE(state, Resource::Success);
	QCOMPARE(ptr.data (), &resource.returnedResource);
	QCOMPARE(ptr->interfaceName (), QString ("Something"));
	
}


void DirectoryResourceTest::invokeGetBadArgument () {
	Resource::InvokeResultState state;
	TestResource resource;
	ResourcePointer ptr;
	
	resource.invoke ("get", { { "name", 123 } }, capture (state, ptr));
	
	QVERIFY(ptr.isNull ());
	QCOMPARE(state, Resource::BadArgumentError);
}

void DirectoryResourceTest::invokeGetNoArgument () {
	Resource::InvokeResultState state;
	TestResource resource;
	ResourcePointer ptr;
	
	resource.invoke ("get", { }, capture (state, ptr));
	
	QVERIFY(ptr.isNull ());
	QCOMPARE(state, Resource::BadArgumentError);
}

QTEST_MAIN(DirectoryResourceTest)
#include "tst_directoryresource.moc"
