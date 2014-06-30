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

#include "referencedevice.hpp"
#include "debug.hpp"

namespace Nuria {
class ReferenceDevicePrivate {
public:
	
	QIODevice *device;
	qint64 pos = 0;
	
	// Range we're operating on
	qint64 begin = 0;
	qint64 end = 0;
	qint64 realEnd = 0;
	
};
}

Nuria::ReferenceDevice::ReferenceDevice (QIODevice *referencedDevice, QObject *parent)
	: QIODevice (parent), d_ptr (new ReferenceDevicePrivate)
{
	
	this->d_ptr->device = referencedDevice;
	setOpenMode (referencedDevice->openMode ());
	
	if (this->d_ptr->device->isSequential ()) {
		nWarn() << "Device" << referencedDevice << "is not random-access!";
	}
	
	// Signal connections
	connect (referencedDevice, &QIODevice::aboutToClose, this, &ReferenceDevice::close);
	connect (referencedDevice, &QIODevice::readyRead, this, &ReferenceDevice::autoExtendRange);
	connect (referencedDevice, &QObject::destroyed, this, &ReferenceDevice::referencedDeviceDestroyed);
	
}

Nuria::ReferenceDevice::~ReferenceDevice () {
	delete this->d_ptr;
}

QIODevice *Nuria::ReferenceDevice::referencedDevice () const {
	return this->d_ptr->device;
}

qint64 Nuria::ReferenceDevice::rangeBegin () const {
	return this->d_ptr->begin;
}

qint64 Nuria::ReferenceDevice::rangeEnd () const {
	return this->d_ptr->end;
}

void Nuria::ReferenceDevice::setRange (qint64 begin, qint64 end) {
	qint64 curSize = this->d_ptr->device->size ();
	end = (end >= 0) ? end : curSize;
	
	this->d_ptr->begin = begin;
	this->d_ptr->end = end;
	this->d_ptr->realEnd = std::min (curSize, end);
	
	// 
	QIODevice::seek (0);
	this->d_ptr->pos = 0;
	
}

void Nuria::ReferenceDevice::extendRange (qint64 bytes) {
	this->d_ptr->end += bytes;
	autoExtendRange ();
}

bool Nuria::ReferenceDevice::isSequential () const {
	return false;
}

bool Nuria::ReferenceDevice::open (QIODevice::OpenMode mode) {
	if ((this->d_ptr->device->openMode () & mode) != mode) {
		return false;
	}
	
	setOpenMode (mode);
	return true;
}

void Nuria::ReferenceDevice::close () {
	setOpenMode (NotOpen);
	emit aboutToClose ();
}

qint64 Nuria::ReferenceDevice::pos () const {
	return this->d_ptr->pos;
}

qint64 Nuria::ReferenceDevice::size () const {
	return std::max (0LL, this->d_ptr->realEnd - this->d_ptr->begin);
}

bool Nuria::ReferenceDevice::seek (qint64 pos) {
	if (pos >= size ()) {
		return false;
	}
	
	// 
	QIODevice::seek (pos);
	this->d_ptr->pos = pos;
	return true;
}

bool Nuria::ReferenceDevice::atEnd () const {
	nDebug() << (this->d_ptr->pos >= size ());
	return (this->d_ptr->pos >= size ());
}

bool Nuria::ReferenceDevice::reset () {
	this->d_ptr->pos = 0;
	return QIODevice::reset ();
}

qint64 Nuria::ReferenceDevice::bytesAvailable () const {
	qint64 size = this->d_ptr->realEnd - this->d_ptr->begin;
	return std::max (0LL, size - this->d_ptr->pos);
}

qint64 Nuria::ReferenceDevice::readData (char *data, qint64 maxlen) {
	qint64 pos = this->d_ptr->pos + this->d_ptr->begin;
	qint64 bytes = std::min (this->d_ptr->realEnd - pos, maxlen);
	if (bytes < 1) {
		return 0;
	}
	
	// 
	qint64 origPos = this->d_ptr->device->pos ();
	this->d_ptr->device->seek (pos);
	
	// 
	qint64 bytesRead = this->d_ptr->device->read (data, bytes);
	this->d_ptr->device->seek (origPos);
	
	// 
	if (bytesRead != -1) {
		this->d_ptr->pos += bytesRead;
	}
	
	// 
	return bytesRead;
}

qint64 Nuria::ReferenceDevice::writeData (const char *data, qint64 len) {
	qint64 pos = this->d_ptr->pos + this->d_ptr->begin;
	qint64 bytes = std::min (this->d_ptr->realEnd - pos, len);
	if (bytes < 1) {
		return 0;
	}
	
	// 
	qint64 origPos = this->d_ptr->device->pos ();
	this->d_ptr->device->seek (pos);
	
	// 
	qint64 written = this->d_ptr->device->write (data, bytes);
	this->d_ptr->device->seek (origPos);
	
	// 
	if (written != -1) {
		this->d_ptr->pos += written;
	}
	
	// 
	return written;
}

void Nuria::ReferenceDevice::referencedDeviceDestroyed () {
	this->d_ptr->device = nullptr;
	close ();
}

void Nuria::ReferenceDevice::autoExtendRange () {
	if (this->d_ptr->end == this->d_ptr->realEnd) {
		return;
	}
	
	// Update current end
	qint64 curSize = this->d_ptr->device->size ();
	qint64 curRealEnd = this->d_ptr->realEnd;
	this->d_ptr->realEnd = std::min (curSize, this->d_ptr->end);
	
	// readyRead() only if the ending changed at all
	if (curRealEnd != this->d_ptr->realEnd) {
		emit readyRead ();
	}
	
}
