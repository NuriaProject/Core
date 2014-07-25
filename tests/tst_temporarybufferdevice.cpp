/* This file is part of the NuriaProject Framework. Copyright 2012 NuriaProject
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * The NuriaProject Framework is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with the library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <nuria/temporarybufferdevice.hpp>

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QObject>
#include <QDate>

using namespace Nuria;

class TemporaryBufferDeviceTest : public QObject {
	Q_OBJECT
private slots:
	
	void emptyBufferNoDevice ();
	void smallBufferUsesMemoryDevice ();
	void bigBufferUsesFileDevice ();
	void whenExceedingLimitDataIsCopied ();
	void discardRemovesDevice ();
	void stealInternalDevice ();
	
};

void TemporaryBufferDeviceTest::emptyBufferNoDevice () {
	TemporaryBufferDevice buffer;
	QVERIFY(buffer.atEnd ());
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::NoDevice);
}

void TemporaryBufferDeviceTest::smallBufferUsesMemoryDevice () {
	TemporaryBufferDevice buffer (32);
	QSignalSpy spy (&buffer, SIGNAL(storageModeChanged(Nuria::TemporaryBufferDevice::StorageMode)));
	
	buffer.write ("Less than 32 Bytes.");
	
	QVariant arg = spy.value (0).value (0);
	QCOMPARE(spy.length (), 1);
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::InMemory);
	QVERIFY(buffer.internalDevice ()->inherits ("QBuffer"));
	QCOMPARE(arg.value< TemporaryBufferDevice::StorageMode > (), buffer.storageMode ());
}

void TemporaryBufferDeviceTest::bigBufferUsesFileDevice () {
	TemporaryBufferDevice buffer (16);
	QSignalSpy spy (&buffer, SIGNAL(storageModeChanged(Nuria::TemporaryBufferDevice::StorageMode)));
	
	buffer.write ("More than 16 Bytes.");
	
	QVariant arg = spy.value (0).value (0);
	QCOMPARE(spy.length (), 1);
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::TemporaryFile);
	QVERIFY(buffer.internalDevice ()->inherits ("QTemporaryFile"));
	QCOMPARE(arg.value< TemporaryBufferDevice::StorageMode > (), buffer.storageMode ());
}

void TemporaryBufferDeviceTest::whenExceedingLimitDataIsCopied () {
	QByteArray thirtyBytes = "012345678901234567890123456789";
	
	TemporaryBufferDevice buffer (30);
	QSignalSpy spy (&buffer, SIGNAL(storageModeChanged(Nuria::TemporaryBufferDevice::StorageMode)));
	
	// 
	buffer.write (thirtyBytes);
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::InMemory);
	
	buffer.write (thirtyBytes);
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::TemporaryFile);
	
	// 
	QVERIFY(buffer.reset ());
	QCOMPARE(buffer.pos (), 0);
	QCOMPARE(buffer.size (), thirtyBytes.length () * 2);
	QCOMPARE(buffer.readAll (), thirtyBytes + thirtyBytes);
	
	// 
	QVariant first = spy.value (0).value (0);
	QVariant second = spy.value (1).value (0);
	
	QCOMPARE(spy.length (), 2);
	QCOMPARE(first.value< TemporaryBufferDevice::StorageMode > (), TemporaryBufferDevice::InMemory);
	QCOMPARE(second.value< TemporaryBufferDevice::StorageMode > (), TemporaryBufferDevice::TemporaryFile);
}

void TemporaryBufferDeviceTest::discardRemovesDevice () {
	QByteArray testData = "Some data";
	
	TemporaryBufferDevice buffer (30);
	QSignalSpy spy (&buffer, SIGNAL(storageModeChanged(Nuria::TemporaryBufferDevice::StorageMode)));
	
	// 
	buffer.write (testData);
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::InMemory);
	
	buffer.discard ();
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::NoDevice);
	QVERIFY(!buffer.internalDevice ());
	
	// 
	QVariant first = spy.value (0).value (0);
	QVariant second = spy.value (1).value (0);
	
	QCOMPARE(spy.length (), 2);
	QCOMPARE(first.value< TemporaryBufferDevice::StorageMode > (), TemporaryBufferDevice::InMemory);
	QCOMPARE(second.value< TemporaryBufferDevice::StorageMode > (), TemporaryBufferDevice::NoDevice);
}

void TemporaryBufferDeviceTest::stealInternalDevice () {
	QByteArray testData = "Some data";
	
	TemporaryBufferDevice buffer (30);
	QSignalSpy spy (&buffer, SIGNAL(storageModeChanged(Nuria::TemporaryBufferDevice::StorageMode)));
	
	// 
	buffer.write (testData);
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::InMemory);
	
	QIODevice *bufferDevice = buffer.stealInternalDevice ();
	QCOMPARE(buffer.storageMode (), TemporaryBufferDevice::NoDevice);
	QVERIFY(!buffer.internalDevice ());
	QVERIFY(bufferDevice);
	
	// 
	QVariant first = spy.value (0).value (0);
	QVariant second = spy.value (1).value (0);
	
	QCOMPARE(spy.length (), 2);
	QCOMPARE(first.value< TemporaryBufferDevice::StorageMode > (), TemporaryBufferDevice::InMemory);
	QCOMPARE(second.value< TemporaryBufferDevice::StorageMode > (), TemporaryBufferDevice::NoDevice);
	
	delete bufferDevice;
}

QTEST_MAIN(TemporaryBufferDeviceTest)
#include "tst_temporarybufferdevice.moc"
