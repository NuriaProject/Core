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

#include "nuria/temporarybufferdevice.hpp"

#include <QTemporaryFile>
#include "nuria/debug.hpp"
#include <QBuffer>

namespace Nuria {
class TemporaryBufferDevicePrivate {
public:
	
	TemporaryBufferDevice::StorageMode mode;
	QIODevice *device = nullptr;
	
	int maxSize;
};
}

Nuria::TemporaryBufferDevice::TemporaryBufferDevice (int maximumMemorySize, QObject *parent)
	: QIODevice (parent), d_ptr (new TemporaryBufferDevicePrivate)
{
	
	qRegisterMetaType< StorageMode > ();
	
	setOpenMode (ReadWrite);
	this->d_ptr->mode = NoDevice;
	this->d_ptr->maxSize = maximumMemorySize;
	
}

Nuria::TemporaryBufferDevice::TemporaryBufferDevice (QObject *parent)
	: TemporaryBufferDevice (DefaultMemorySize, parent)
{
	
}

Nuria::TemporaryBufferDevice::~TemporaryBufferDevice () {
	discard ();
	delete this->d_ptr;
}

int Nuria::TemporaryBufferDevice::maximumMemorySize () const {
	return this->d_ptr->maxSize;
}

void Nuria::TemporaryBufferDevice::setMaximumMemorySize (int maximumSize) {
	this->d_ptr->maxSize = maximumSize;
	decideStrategy (size ());
}

Nuria::TemporaryBufferDevice::StorageMode Nuria::TemporaryBufferDevice::storageMode () const {
	return this->d_ptr->mode;
}

QIODevice *Nuria::TemporaryBufferDevice::internalDevice () const {
	return this->d_ptr->device;
}

QIODevice *Nuria::TemporaryBufferDevice::stealInternalDevice () {
	QIODevice *device = this->d_ptr->device;
	this->d_ptr->device = nullptr;
	this->d_ptr->mode = NoDevice;
	
	device->setParent (nullptr);
	emit storageModeChanged (NoDevice);
	return device;
}

void Nuria::TemporaryBufferDevice::discard () {
	this->d_ptr->mode = NoDevice;
	delete this->d_ptr->device;
	this->d_ptr->device = nullptr;
	
	emit storageModeChanged (NoDevice);
}

bool Nuria::TemporaryBufferDevice::isSequential () const {
	return false;
}

bool Nuria::TemporaryBufferDevice::open (OpenMode mode) {
	setOpenMode (mode);
	return true;
}

void Nuria::TemporaryBufferDevice::close () {
	setOpenMode (NotOpen);
	delete this->d_ptr->device;
	this->d_ptr->device = nullptr;
}

qint64 Nuria::TemporaryBufferDevice::pos () const {
	if (!this->d_ptr->device) {
		return 0;
	}
	
	return this->d_ptr->device->pos ();
}

qint64 Nuria::TemporaryBufferDevice::size () const {
	if (!this->d_ptr->device) {
		return 0;
	}
	
	return this->d_ptr->device->size ();
}

bool Nuria::TemporaryBufferDevice::seek (qint64 pos) {
	if (!this->d_ptr->device) {
		return false;
	}
	
	QIODevice::seek (pos);
	return this->d_ptr->device->seek (pos);
}

bool Nuria::TemporaryBufferDevice::atEnd () const {
	if (!this->d_ptr->device) {
		return true;
	}
	
	return this->d_ptr->device->atEnd ();
}

bool Nuria::TemporaryBufferDevice::reset () {
	if (!this->d_ptr->device) {
		return true;
	}
	
	QIODevice::reset ();
	return this->d_ptr->device->reset ();
}

qint64 Nuria::TemporaryBufferDevice::bytesAvailable () const {
	if (!this->d_ptr->device) {
		return 0;
	}
	
	return this->d_ptr->device->bytesAvailable ();
}

qint64 Nuria::TemporaryBufferDevice::bytesToWrite () const {
	if (!this->d_ptr->device) {
		return 0;
	}
	
	return this->d_ptr->device->bytesToWrite ();
}

bool Nuria::TemporaryBufferDevice::canReadLine () const {
	if (!this->d_ptr->device) {
		return false;
	}
	
	return this->d_ptr->device->canReadLine ();
}

bool Nuria::TemporaryBufferDevice::waitForReadyRead (int msecs) {
	if (!this->d_ptr->device) {
		return false;
	}
	
	return this->d_ptr->device->waitForReadyRead (msecs);
}

bool Nuria::TemporaryBufferDevice::waitForBytesWritten (int msecs) {
	if (!this->d_ptr->device) {
		return true;
	}
	
	return this->d_ptr->device->waitForBytesWritten (msecs);
}

qint64 Nuria::TemporaryBufferDevice::readData (char *data, qint64 maxlen) {
	if (!this->d_ptr->device) {
		return 0;
	}
	
	qint64 bytes = this->d_ptr->device->read (data, maxlen);
	decideStrategy (size ());
	return bytes;
}

qint64 Nuria::TemporaryBufferDevice::readLineData (char *data, qint64 maxlen) {
	if (!this->d_ptr->device) {
		return 0;
	}
	
	qint64 bytes = this->d_ptr->device->readLine (data, maxlen);
	decideStrategy (size ());
	return bytes;
}

qint64 Nuria::TemporaryBufferDevice::writeData (const char *data, qint64 len) {
	qint64 cur = pos ();
	if (cur + len > size ()) {
		decideStrategy (cur + len);
	}
	
	if (this->d_ptr->device) {
		return this->d_ptr->device->write (data, len);
	}
	
	return -1;
}

void Nuria::TemporaryBufferDevice::decideStrategy (int newSize) {
	StorageMode mode = this->d_ptr->mode;
	QIODevice *oldDevice = this->d_ptr->device;
	QIODevice *newDevice = nullptr;
	
	if (newSize == 0) {
		discard ();
		
	} else if (newSize > this->d_ptr->maxSize && mode != TemporaryFile) {
		mode = TemporaryFile;
		newDevice = openFileBuffer ();
		
	} else if (mode == NoDevice) {
		mode = InMemory;
		newDevice = openMemoryBuffer ();
	}
	
	// Copy data from old to the new device.
	if (newDevice) {
		moveMemoryBufferToFileDevice (oldDevice, newDevice);
		delete oldDevice;
		
		this->d_ptr->device = newDevice;
		this->d_ptr->mode = mode;
		emit storageModeChanged (mode);
	}
	
}

void Nuria::TemporaryBufferDevice::moveMemoryBufferToFileDevice (QIODevice *memory, QIODevice *file) {
	QBuffer *buffer = qobject_cast< QBuffer * > (memory);
	QTemporaryFile *target = qobject_cast< QTemporaryFile * > (file);
	
	if (!buffer || !target) {
		return;
	}
	
	// 
	target->write (buffer->data ());
}

QIODevice *Nuria::TemporaryBufferDevice::openMemoryBuffer () {
	QBuffer *buffer = new QBuffer (this);
	buffer->open (QIODevice::ReadWrite);
	
	return buffer;
}

QIODevice *Nuria::TemporaryBufferDevice::openFileBuffer () {
	QTemporaryFile *buffer = new QTemporaryFile (this);
	
	if (!buffer->open ()) {
		nCritical() << "Failed to open temporary file - DISCARDING BUFFER!!";
		delete buffer;
		buffer = nullptr;
		discard ();
	}
	
	return buffer;
}
