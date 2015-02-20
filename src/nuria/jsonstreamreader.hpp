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

#ifndef NURIA_JSONSTREAMREADER_HPP
#define NURIA_JSONSTREAMREADER_HPP

#include <QJsonDocument>
#include <QIODevice>

namespace Nuria {

class JsonStreamReaderPrivate;

/**
 * \brief QIODevice for reading JSON streams
 * 
 * This QIODevice makes it simple to read JSON streams, as used in network
 * protocols like JSON-RPC. It works element-wise, meaning that a whole valid
 * JSON document must be present for it to parse it.
 * 
 * This device is write-only in terms of QIODevice. To append streamed data,
 * use write().
 * 
 * \par Usage
 * 
 * After instantiating JsonStreamReader, you use QIODevice::write() to append
 * data to the stream buffer. The stream buffer is the first internal buffer
 * where data ends up. If the code notes that the data contains a complete
 * JSON value (That's an object, array, string, number, boolean or null), it
 * gets forwarded to the element buffer. Entries in the element buffer are
 * deemed 'complete', but don't necessarily need to be valid JSON code.
 * 
 * For each found element newPendingElement() is emitted. You can use then
 * nextPendingElement() to get the parsed QJsonDocument of it, which may
 * error if the element contained invalid JSON data.
 * 
 * \par Error handling
 * 
 * It's important to note that the're two kinds of errors both indicating
 * invalid JSON data:
 * 
 * 1. JSON error as indicated by hasError() and error().
 * 2. JSON error as found in nextPendingElement().
 * 
 * The first one requires intervention by the user of this class. It occurs when
 * data is written into the device containing JSON with non-matching brackets
 * for objects and arrays. This will result in a error() emission and hasError()
 * will return \c true after the write() call. When this happens, you have to
 * call clearStreamBuffer(), which clears the stream buffer putting the reader
 * back into a known state. This is not done automatically as it results in
 * potentional data loss.
 * 
 * The second one requires \b no intervention by the user. It's solely local to
 * the returned JSON document.
 * 
 */
class JsonStreamReader : public QIODevice {
	Q_OBJECT
public:
	
	/** Constructor. */
	explicit JsonStreamReader (QObject *parent = nullptr);
	
	/** Destructor. */
	~JsonStreamReader ();
	
	/**
	 * Discards internal buffers, resetting the instance to its initial
	 * state.
	 */
	void discard ();
	
	/**
	 * Clears the current streaming buffer for error recovery. JSON elements
	 * which have been detected as 'complete' are not cleared.
	 * 
	 * \sa hasError
	 */
	void clearStreamBuffer ();
	
	/**
	 * Returns \c true if the stream detected a JSON error. If this happens,
	 * you can call clear() to remove the current streaming buffer and reset
	 * the reader into a known state for error recovery.
	 * 
	 * \note Writing more data will never recover from an error.
	 * \warning This only indicates errors in the streaming buffer. JSON
	 * objects which are otherwise deemed 'complete' may still be invalid
	 * JSON.
	 * 
	 * \sa error clearStreamBuffer
	 */
	bool hasError ();
	
	/**
	 * Returns \c true if there are complete JSON elements.
	 */
	bool hasPendingElement () const;
	
	/**
	 * Returns the next pending parsed JSON element. Passing \a parseError
	 * is recommended. See QJsonDocument::fromJson() for further information
	 * on error handling.
	 * 
	 * If there's no pending element, a empty QJsonDocument is returned.
	 * 
	 * \sa hasPendingElement newPendingElement
	 */
	QJsonDocument nextPendingElement (QJsonParseError *parseError = nullptr);
	
signals:
	
	/** Emitted for each newly found element. \sa nextPendingElement */
	void newPendingElement ();
	
	/**
	 * Emitted when appended data resulted in a reader error. This signal
	 * is only raised in this event and is the pendant to hasError(). It
	 * is \b not triggered when nextPendingElement() finds a JSON error.
	 * 
	 * \sa hasError clearStreamBuffer
	 */
	void error ();
	
protected:
	qint64 readData (char *data, qint64 maxlen) override;
	qint64 writeData (const char *data, qint64 len) override;
	
private:
	
	JsonStreamReaderPrivate *d_ptr;
	
};

} // namespace Nuria

#endif // NURIA_JSONSTREAMREADER_HPP
