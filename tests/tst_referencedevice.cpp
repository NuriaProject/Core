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

#include <QCoreApplication>

#include <nuria/referencedevice.hpp>
#include <QtTest/QTest>
#include <QSignalSpy>
#include <QBuffer>
#include <QDebug>

using namespace Nuria;

class ReferenceDeviceTest : public QObject {
	Q_OBJECT
private slots:
	
	void verifyOpenModeCompliance ();
	void verifySize ();
	void readData ();
	void dontReadOverRange ();
	void writeData ();
	void dontWriteOverRange ();
	void aboutToCloseIsPassedThrough ();
	void destroyingTheDeviceClosesTheReference ();
	void readyReadIsEmittedOnExtension ();
	
private:
	
	QBuffer *createBuffer (const QByteArray &data = "0123456789",
			       QIODevice::OpenMode mode = QIODevice::ReadWrite) {
		QBuffer *buffer = new QBuffer (this);
		buffer->setData (data);
		buffer->open (mode);
		return buffer;
	}
	
};



void ReferenceDeviceTest::verifyOpenModeCompliance () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	
	// 
	QCOMPARE(buffer->openMode (), device.openMode ());
	
	// 
	QVERIFY(device.open (QIODevice::ReadOnly));
	QVERIFY(device.open (QIODevice::WriteOnly));
	QVERIFY(device.open (QIODevice::ReadWrite));
	QVERIFY(device.open (QIODevice::NotOpen));
	
	// 
	buffer->open (QIODevice::ReadOnly);
	QVERIFY(device.open (QIODevice::ReadOnly));
	QVERIFY(!device.open (QIODevice::WriteOnly));
	QVERIFY(!device.open (QIODevice::ReadWrite));
	QVERIFY(device.open (QIODevice::NotOpen));
	
	// 
	buffer->open (QIODevice::WriteOnly);
	QVERIFY(!device.open (QIODevice::ReadOnly));
	QVERIFY(device.open (QIODevice::WriteOnly));
	QVERIFY(!device.open (QIODevice::ReadWrite));
	QVERIFY(device.open (QIODevice::NotOpen));
	
	// 
	buffer->close ();
	QVERIFY(!device.open (QIODevice::ReadOnly));
	QVERIFY(!device.open (QIODevice::WriteOnly));
	QVERIFY(!device.open (QIODevice::ReadWrite));
	QVERIFY(device.open (QIODevice::NotOpen));
	
}

void ReferenceDeviceTest::verifySize () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	
	QCOMPARE(device.size (), 0); // No range set
	
	// End == buffer.size
	device.setRange (0, 10);
	QCOMPARE(device.size (), 10);
	
	// End > buffer.size
	device.setRange (0, 15);
	QCOMPARE(device.size (), 10);
	
	// 
	device.setRange (4, 10);
	QCOMPARE(device.size (), 6);
	
	// 
	device.setRange (4, 15);
	QCOMPARE(device.size (), 6);
	
	// 
	device.setRange (3);
	QCOMPARE(device.size (), 7);
	
}

void ReferenceDeviceTest::readData () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	buffer->seek (5);
	
	// 
	for (int end : { 10, 15 }) {
		device.setRange (0, end);
		QCOMPARE(device.read (end), QByteArray ("0123456789"));
		QCOMPARE(device.pos (), 10);
		QCOMPARE(buffer->pos (), 5);
	}
	
	// 
	for (int end : { 10, 15 }) {
		device.setRange (4, end);
		QCOMPARE(device.read (end), QByteArray ("456789"));
		QCOMPARE(device.pos (), 6);
		QCOMPARE(buffer->pos (), 5);
	}
	
}


void ReferenceDeviceTest::dontReadOverRange () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	buffer->seek (6);
	
	// 
	device.setRange (2, 4);
	QCOMPARE(device.read (10), QByteArray ("23"));
	QCOMPARE(device.pos (), 2);
	QCOMPARE(buffer->pos (), 6);
	
	// 
	device.setRange (8, 15);
	QCOMPARE(device.read (10), QByteArray ("89"));
	QCOMPARE(device.pos (), 2);
	QCOMPARE(buffer->pos (), 6);
	
}


void ReferenceDeviceTest::writeData () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	buffer->seek (7);
	
	// 
	device.setRange (0, 10);
	QCOMPARE(device.write ("9876543210", 10), 10);
	QCOMPARE(device.pos (), 10);
	QCOMPARE(buffer->pos (), 7);
	
	// 
	device.setRange (2, 4);
	QCOMPARE(device.write ("ABCDEFG", 7), 2);
	QCOMPARE(device.pos (), 2);
	QCOMPARE(buffer->pos (), 7);
	
	// 
	buffer->close ();
	QCOMPARE(buffer->data (), QByteArray ("98AB543210"));
}

void ReferenceDeviceTest::dontWriteOverRange () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	
	// 
	device.setRange (10, 20);
	QCOMPARE(device.write ("0123456789", 10), 0);
	QCOMPARE(device.pos (), 0);
	
	// 
	device.setRange (5, 15);
	QCOMPARE(device.write ("0123456789", 10), 5);
	QCOMPARE(device.pos (), 5);
	
}

void ReferenceDeviceTest::aboutToCloseIsPassedThrough () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	QSignalSpy spy (&device, SIGNAL(aboutToClose()));
	
	// 
	buffer->close ();
	qApp->processEvents ();
	
	// 
	QCOMPARE(spy.length (), 1);
}


void ReferenceDeviceTest::destroyingTheDeviceClosesTheReference () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	QSignalSpy spy (&device, SIGNAL(aboutToClose()));
	
	// 
	delete buffer;
	qApp->processEvents ();
	
	// 
	QCOMPARE(device.openMode (), QIODevice::NotOpen);
	QCOMPARE(spy.length (), 1);
}

void ReferenceDeviceTest::readyReadIsEmittedOnExtension () {
	QBuffer *buffer = createBuffer ();
	ReferenceDevice device (buffer);
	QSignalSpy spy (&device, SIGNAL(readyRead()));
	
	// 
	device.setRange (5, 15); // Longer than 'buffer'
	QCOMPARE(device.readAll (), QByteArray ("56789"));
	
	// 
	buffer->seek (10);
	buffer->write ("ABC");
	qApp->processEvents ();
	
	QCOMPARE(device.size (), 8);
	QCOMPARE(device.bytesAvailable (), 3);
	QCOMPARE(device.pos (), 5);
	QCOMPARE(device.readAll (), QByteArray ("ABC"));
	QCOMPARE(spy.length (), 1);
	
	// 
	buffer->write ("DEFG");
	qApp->processEvents ();
	QCOMPARE(device.size (), 10);
	QCOMPARE(device.bytesAvailable (), 2);
	QCOMPARE(device.pos (), 8);
	QCOMPARE(device.readAll (), QByteArray ("DE"));
	QCOMPARE(spy.length (), 2);
	
	// 
	device.extendRange (5);
	qApp->processEvents ();
	QCOMPARE(device.size (), 12);
	QCOMPARE(device.bytesAvailable (), 2);
	QCOMPARE(device.pos (), 10);
	QCOMPARE(device.readAll (), QByteArray ("FG"));
	QCOMPARE(spy.length (), 3);
	QCOMPARE(device.rangeBegin (), 5);
	QCOMPARE(device.rangeEnd (), 20);
	
}

QTEST_MAIN(ReferenceDeviceTest)
#include "tst_referencedevice.moc"
