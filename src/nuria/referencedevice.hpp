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

#ifndef NURIA_REFERENCEDEVICE_HPP
#define NURIA_REFERENCEDEVICE_HPP

#include <QIODevice>
#include "essentials.hpp"

namespace Nuria {

class ReferenceDevicePrivate;

/**
 * \brief Operates on a part of another QIODevice.
 * 
 * ReferenceDevice references a given part of a random-access QIODevice.
 * This device does not store any data itself. This means, that the
 * referenced QIODevice must be kept alive as long a ReferenceDevice is
 * operating on it.
 * 
 * \note If the referenced device is destroyed, all reference devices will
 * be closed.
 * 
 * \par Open mode
 * The open mode is copied form the referenced device in the constructor.
 * The implementation of open() will only allow modes compatible with the
 * current open mode of the device. This means, that if the referenced device
 * is currently in QIODevice::ReadWrite mode, then QIODevice::NotOpen, 
 * QIODevice::ReadOnly, QIODevice::WriteOnly and QIODevice::ReadWrite are
 * allowed. If the device is in QIODevice::ReadOnly mode, only
 * QIODevice::NotOpen and QIODevice::ReadOnly are allowed - and so on.
 * 
 */
class NURIA_CORE_EXPORT ReferenceDevice : public QIODevice {
	Q_OBJECT
public:
	
	/**
	 * Constructs a ReferenceDevice referencing \a referencedDevice.
	 * \sa setRange
	 */
	explicit ReferenceDevice (QIODevice *referencedDevice, QObject *parent = 0);
	
	/** Destructor. */
	~ReferenceDevice () override;
	
	/** Returns the device the ReferenceDevice is operating on. */
	QIODevice *referencedDevice () const;
	
	/** Returns the begin of the referenced range. */
	qint64 rangeBegin () const;
	
	/** Returns the end of the referenced range. */
	qint64 rangeEnd () const;
	
	/**
	 * Sets the operated range to be from \a begin till \a end (exclusive).
	 * If \a end is longer than the current size() of the
	 * referencedDevice(), then the range will be extended automatically
	 * when data has been written to the referenced device, until it matches
	 * the given one.
	 * 
	 * If \a begin is not currently outside the referenced device and the
	 * range is greater than \c 0, then readyRead() will be emitted.
	 * 
	 * If \a end is \c -1, then the range will span from \a begin till the
	 * current size() of the referenced device.
	 * 
	 * \note After this operation, the position of this device will be \c 0.
	 */
	void setRange (qint64 begin, qint64 end = -1);
	
	/**
	 * Extends the current range by \a bytes. The same range extension
	 * mechanism like in setRange() takes place. Signals are emitted
	 * accordingly.
	 */
	void extendRange (qint64 bytes);
	
	// 
	bool isSequential () const override;
	bool open (OpenMode mode) override;
	void close () override;
	qint64 pos () const override;
	qint64 size () const override;
	bool seek (qint64 pos) override;
	bool atEnd () const override;
	bool reset () override;
	qint64 bytesAvailable () const override;
	
protected:
	qint64 readData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len);
	
private:
	void referencedDeviceDestroyed ();
	void autoExtendRange ();
	
	ReferenceDevicePrivate *d_ptr;
	
};

}

#endif // NURIA_REFERENCEDEVICE_HPP
