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

#ifndef NURIA_TEMPORARYBUFFERDEVICE_HPP
#define NURIA_TEMPORARYBUFFERDEVICE_HPP

#include "essentials.hpp"
#include <QIODevice>

namespace Nuria {

class TemporaryBufferDevicePrivate;

/**
 * \brief In-memory buffer device with fallback to a temporary file.
 * 
 * This buffer uses internally a QBuffer to store data, but will start using
 * a temporary file as storage when the buffer gets bigger than a given
 * amount of bytes.
 * 
 * \sa DefaultMemorySize
 * 
 * \par Behaviour
 * This class acts as usual random-access (i.e. non-sequential) QIODevice for
 * easy and quick storage of temporary data. It will try to use a QBuffer to
 * store the data directly in-memory first. When the data gets bigger than
 * a certain value though, it'll copy the data into a temporary file and
 * begin using it as back-end. This process is completely transparent to the
 * user.
 * 
 * The device itself is usually opened for read and write access. You can
 * change this using open(), though this has no effect on the internal
 * device.
 */
class NURIA_CORE_EXPORT TemporaryBufferDevice : public QIODevice {
	Q_OBJECT
public:
	enum {
		/**
		 * Default maximum size of the in-memory buffer.
		 */
		DefaultMemorySize = 4096 * 1024 // 4MiB
	};
	
	/** Storage mode for the internal device. */
	enum StorageMode {
		
		/** The buffer device contains no data. */
		NoDevice = 0,
		
		/** In-memory buffer. */
		InMemory,
		
		/** Temporary file as buffer. */
		TemporaryFile
	};
	
	/** Constructor. Initializes \a maximumMemorySize directly. */
	explicit TemporaryBufferDevice (int maximumMemorySize, QObject *parent = 0);
	
	/** \overload */
	explicit TemporaryBufferDevice (QObject *parent = 0);
	
	/** Destructor. */
	~TemporaryBufferDevice () override;
	
	/**
	 * Returns the maximum size of the in-memory buffer. After the buffer
	 * grows beyond this barrier, a temporary file will be used instead.
	 */
	int maximumMemorySize () const;
	
	/**
	 * Sets the maximum size of the in-memory buffer.
	 * The new value is applied immediately, thus if the buffer is
	 * exceeding the limit, a temporary file will be used.
	 */
	void setMaximumMemorySize (int maximumSize);
	
	/** Returns the current storage mode. */
	StorageMode storageMode () const;
	
	/**
	 * Returns the currently used internal storage device.
	 * \note Usually you don't need this.
	 */
	QIODevice *internalDevice () const;
	
	/**
	 * Steals the internal storage device from the instance. Ownership
	 * of the device is transferred to the caller. The TemporaryBufferDevice
	 * instance will act as if it was just created, thus contain no data.
	 */
	QIODevice *stealInternalDevice ();
	
	/**
	 * Discards the internal buffer. Doing so will loose all buffered data.
	 */
	void discard ();
	
	// 
	bool isSequential () const;
	bool open (OpenMode mode);
	
	/**
	 * Closes the internal device, deleting all buffered data.
	 */
	void close ();
	qint64 pos () const;
	qint64 size () const;
	bool seek (qint64 pos);
	bool atEnd () const;
	bool reset ();
	qint64 bytesAvailable () const;
	qint64 bytesToWrite () const;
	bool canReadLine () const;
	bool waitForReadyRead (int msecs);
	bool waitForBytesWritten (int msecs);
	
signals:
	
	/**
	 * Triggered when the storage mode has been changed.
	 */
	void storageModeChanged (Nuria::TemporaryBufferDevice::StorageMode storageMode);
	
protected:
	qint64 readData(char *data, qint64 maxlen);
	qint64 readLineData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len);
	
private:
	void decideStrategy (int newSize);
	void moveMemoryBufferToFileDevice (QIODevice *memory, QIODevice *file);
	QIODevice *openMemoryBuffer ();
	QIODevice *openFileBuffer ();
	
	TemporaryBufferDevicePrivate *d_ptr;
	
};

}

Q_DECLARE_METATYPE(Nuria::TemporaryBufferDevice::StorageMode)

#endif // NURIA_TEMPORARYBUFFERDEVICE_HPP
