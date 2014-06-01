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

#ifndef NURIA_REFERENCEDEVICE_HPP
#define NURIA_REFERENCEDEVICE_HPP

#include <QIODevice>

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
class ReferenceDevice : public QIODevice {
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
