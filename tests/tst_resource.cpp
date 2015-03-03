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
#include <nuria/resource.hpp>

#include <nuria/callback.hpp>

using namespace Nuria;

class TestResource : public Resource {
	Q_OBJECT
public:
	QString interfaceName () const;
	Invocation properties (InvokeCallback callback, int timeout = -1);
	
	Invocation invokeImpl (const QString &slot, const QVariantMap &arguments, InvokeCallback callback, int timeout);
	Invocation listenImpl (const QString &signal, InvokeCallback callback);
	
public slots:
	
	void waitTest (Nuria::Resource::InvokeCallback callback);
	
};

class ResourceTest : public QObject {
	Q_OBJECT
private slots:
	
	void verifyDefaultImplementations ();
	
	void verifyInvokeSimpleCall ();
	void defaultInvokeInvokesPropertiesMethod ();
	void defaultInvokeHandlesUnknownProperty ();
	void invokeProcessesResourceHandlerException ();
	
	void listenProcessesResourceHandlerException ();
	void listenHandlesUnknownProperty ();
	
	void invocationResultSimple ();
	void invocationResultSimpleWithQVariant ();
	void invocationResultSimpleWithVoid ();
	
	void invocationResultWaitSameThread_data ();
	void invocationResultWaitSameThread ();
	
	void invocationResultWaitThreaded_data ();
	void invocationResultWaitThreaded ();
	
	void propertyEqual_data ();
	void propertyEqual ();
};

QString TestResource::interfaceName () const {
	return "Test";
}

Invocation TestResource::properties (InvokeCallback callback, int timeout) {
	callback (Success, timeout);
	return Invocation ();
}

Invocation TestResource::invokeImpl (const QString &slot, const QVariantMap &arguments,
                                     InvokeCallback callback, int timeout) {
	if (slot == "simple") {
		callback (Success, arguments);
		return Invocation (this);
	} else if (slot == "exception") {
		throw ResourceHandlerException (TimeoutError, timeout);
	} else if (slot == "wait") {
		Callback cb (this, SLOT(waitTest(Nuria::Resource::InvokeCallback)));
		cb (callback);
		
		return Invocation (this);
	}
	
	return Resource::invokeImpl (slot, arguments, callback, timeout);
}

Invocation TestResource::listenImpl (const QString &signal, InvokeCallback callback) {
	if (signal == "exception") {
		throw ResourceHandlerException (TimeoutError, signal);
	}
	
	return Resource::listenImpl (signal, callback);
}

void TestResource::waitTest (InvokeCallback callback) {
	QTimer *timer = new QTimer (this);
	connect (timer, &QTimer::timeout, [callback]() { callback (UserError, "waited"); });
	timer->start (100);	
}

void ResourceTest::verifyDefaultImplementations () {
	// Verify according to documentation
	
	TestResource r;
	QCOMPARE(r.isSerializable (), false);
	QCOMPARE(r.serialize (), QByteArray ());
	QCOMPARE(r.deserialize (""), false);
	
	QCOMPARE(Resource::resultStateName (Resource::InvokeResultState (-2)), QString ("<Unknown:-2>"));
	QCOMPARE(Resource::resultStateName (Resource::Success), QString ("Success"));
}

void ResourceTest::verifyInvokeSimpleCall () {
	TestResource r;
	
	Resource::InvokeResultState state;
	QVariant result;
	
	// 
	QVariantMap args { { "foo", 123 } };
	r.invoke ("simple", args, [&](Resource::InvokeResultState s, const QVariant &r) {
		state = s;
		result = r;
	});
	
	// 
	QCOMPARE(state, Resource::Success);
	QCOMPARE(result.toMap (), args);
	
}

void ResourceTest::defaultInvokeInvokesPropertiesMethod () {
	TestResource r;
	
	// 
	Resource::InvokeResultState state;
	QVariant result;
	r.invoke ("", { }, [&](Resource::InvokeResultState s, const QVariant &r) {
		state = s;
		result = r;
	}, 1234);
	
	// 
	QCOMPARE(state, Resource::Success);
	QCOMPARE(result.toInt (), 1234);
}

void ResourceTest::defaultInvokeHandlesUnknownProperty () {
	TestResource r;
	
	Resource::InvokeResultState state;
	QVariant result;
	r.invoke ("does-not-exist", { }, [&](Resource::InvokeResultState s, const QVariant &r) {
		state = s;
		result = r;
	});
	
	// 
	QCOMPARE(state, Resource::UnknownError);
	QCOMPARE(result.toString (), QString ("does-not-exist"));
}

void ResourceTest::invokeProcessesResourceHandlerException () {
	TestResource r;
	
	Resource::InvokeResultState state;
	QVariant result;
	r.invoke ("exception", { }, [&](Resource::InvokeResultState s, const QVariant &r) {
		state = s;
		result = r;
	}, 1234);
	
	// 
	QCOMPARE(state, Resource::TimeoutError);
	QCOMPARE(result.toInt (), 1234);
	
}

void ResourceTest::listenProcessesResourceHandlerException () {
	TestResource r;
	
	Resource::InvokeResultState state;
	QVariant result;
	r.listen ("exception", [&](Resource::InvokeResultState s, const QVariant &r) {
		state = s;
		result = r;
	});
	
	// 
	QCOMPARE(state, Resource::TimeoutError);
	QCOMPARE(result.toString (), QString ("exception"));
}

void ResourceTest::listenHandlesUnknownProperty () {
	TestResource r;
	
	Resource::InvokeResultState state;
	QVariant result;
	r.listen ("does-not-exist", [&](Resource::InvokeResultState s, const QVariant &r) {
		state = s;
		result = r;
	});
	
	// 
	QCOMPARE(state, Resource::UnknownError);
	QCOMPARE(result.toString (), QString ("does-not-exist"));
	
}

void ResourceTest::invocationResultSimple () {
	TestResource r;
	
	QVariantMap args { { "foo", 456 } };
	InvocationResult< QVariantMap > result = r.invoke< QVariantMap > ("simple", args);
	
	QVERIFY(result.hasFinished ());
	QVERIFY(!result.hasError ());
	QCOMPARE(result.result (), QVariant (args));
	QCOMPARE(result.value (), args);
	
	QVariantMap t = result; // Implicit cast
	QCOMPARE(t, args);
}

void ResourceTest::invocationResultSimpleWithQVariant () {
	TestResource r;
	
	QVariantMap args { { "foo", 456 } };
	InvocationResult< QVariant > result = r.invoke< QVariant > ("simple", args);
	
	QVERIFY(result.hasFinished ());
	QVERIFY(!result.hasError ());
	QCOMPARE(result.resultState (), Resource::Success);
	QCOMPARE(result.result (), QVariant (args));
	QCOMPARE(result.value (), QVariant (args));
	
	QVariant t = result; // Implicit cast
	QCOMPARE(t, QVariant (args));
	
}

void ResourceTest::invocationResultSimpleWithVoid () {
	TestResource r;
	
	QVariantMap args { { "foo", 456 } };
	InvocationResult< void > result = r.invoke< void > ("simple", args);
	
	QVERIFY(result.hasFinished ());
	QVERIFY(!result.hasError ());
	// result.result() does not have to be valid.
	
}

Q_DECLARE_METATYPE(Nuria::InvocationResultBase::WaitMode);
void ResourceTest::invocationResultWaitSameThread_data () {
	QTest::addColumn< InvocationResultBase::WaitMode > ("mode");
	
	QTest::newRow ("automatic") << InvocationResultBase::Automatic;
	QTest::newRow ("event-loop") << InvocationResultBase::EventLoop;
}

void ResourceTest::invocationResultWaitSameThread () {
	QFETCH(InvocationResultBase::WaitMode, mode);
	
	TestResource r;
	InvocationResult< QString > result = r.invoke< QString > ("wait", { });
	
	Resource::InvokeResultState state = result.waitForFinished (mode);
	
	QCOMPARE(state, Resource::UserError);
	QCOMPARE(result.resultState (), state);
	QCOMPARE(result.value (), QString ("waited"));
	
}

void ResourceTest::invocationResultWaitThreaded_data () {
	QTest::addColumn< InvocationResultBase::WaitMode > ("mode");
	
	QTest::newRow ("automatic") << InvocationResultBase::Automatic;
	QTest::newRow ("event-loop") << InvocationResultBase::EventLoop;
	QTest::newRow ("blocking") << InvocationResultBase::Blocking;
	
}

void ResourceTest::invocationResultWaitThreaded () {
	QFETCH(InvocationResultBase::WaitMode, mode);
	
	QThread thread;
	thread.start ();
	
	// 
	TestResource *r = new TestResource;
	r->moveToThread (&thread);
	QTest::qSleep (100); // 100ms are hopefully enough
	
	// Create watchdog so this test keeps being useful without human intervention
	QTimer *watchdog = new QTimer;
	connect (watchdog, &QTimer::timeout, []() {
		qDebug("WATCHDOG: Looks like the implementation dead-locked. Aborting!");
		abort ();
	});
	
	watchdog->start (1000); // 1sec
	watchdog->moveToThread (&thread);
	
	// Invoke!
	InvocationResult< QString > result = r->invoke< QString > ("wait", { });
	Resource::InvokeResultState state = result.waitForFinished (mode);
	
	QCOMPARE(state, Resource::UserError);
	QCOMPARE(result.resultState (), state);
	QCOMPARE(result.value (), QString ("waited"));
	
	// 
	Callback (&thread, SLOT(quit())) ();
	thread.wait ();
	
}

void ResourceTest::propertyEqual_data () {
	QTest::addColumn< bool > ("equal");
	QTest::addColumn< Resource::Property > ("a");
	QTest::addColumn< Resource::Property > ("b");
	
	Resource::Property a (Resource::Property::Slot, "foo", { { "argument", 123 } }, 456);
	Resource::Property type (Resource::Property::Signal, "foo", { { "argument", 123 } }, 456);
	Resource::Property name (Resource::Property::Slot, "bar", { { "argument", 123 } }, 456);
	Resource::Property args (Resource::Property::Slot, "foo", { { "argument", 123 }, { "asd", 789 } }, 456);
	Resource::Property resultType (Resource::Property::Slot, "foo", { { "argument", 123 } }, 789);
	
	QTest::newRow ("invalid-equal") << true << Resource::Property () << Resource::Property ();
	QTest::newRow ("valid-invalid") << false << a << Resource::Property ();
	QTest::newRow ("invalid-valid") << false << Resource::Property () << a;
	QTest::newRow ("different-type") << false << a << type;
	QTest::newRow ("different-name") << false << a << name;
	QTest::newRow ("different-args") << false << a << args;
	QTest::newRow ("different-resultType") << false << a << resultType;
	
}

void ResourceTest::propertyEqual () {
	QFETCH(bool, equal);
	QFETCH(Resource::Property, a);
	QFETCH(Resource::Property, b);
	
	QCOMPARE(a == b, equal);
}

QTEST_MAIN(ResourceTest)
#include "tst_resource.moc"
