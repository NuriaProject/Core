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

#include "nuria/referencedevice.hpp"
#include "nuria/logger.hpp"

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
