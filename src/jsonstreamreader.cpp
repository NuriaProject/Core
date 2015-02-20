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

#include "nuria/jsonstreamreader.hpp"

#include "private/streamingjsonhelper.hpp"

namespace Nuria {
class JsonStreamReaderPrivate {
public:
	
	Internal::StreamingJsonHelper streamer;
	bool error = false;
	
};
}

Nuria::JsonStreamReader::JsonStreamReader (QObject *parent)
        : QIODevice (parent), d_ptr (new JsonStreamReaderPrivate)
{
	
	setOpenMode (WriteOnly);
	
}

Nuria::JsonStreamReader::~JsonStreamReader () {
	delete this->d_ptr;
}

void Nuria::JsonStreamReader::discard () {
	this->d_ptr->error = false;
	this->d_ptr->streamer = Internal::StreamingJsonHelper ();
}

void Nuria::JsonStreamReader::clearStreamBuffer () {
	this->d_ptr->streamer.resetBuffer ();
	this->d_ptr->error = false;
}

bool Nuria::JsonStreamReader::hasError () {
	return this->d_ptr->error;
}

bool Nuria::JsonStreamReader::hasPendingElement () const {
	return this->d_ptr->streamer.hasWaitingElement ();
}

QJsonDocument Nuria::JsonStreamReader::nextPendingElement (QJsonParseError *parseError) {
	if (!this->d_ptr->streamer.hasWaitingElement ()) {
		return QJsonDocument ();
	}
	
	// 
	QByteArray jsonData = this->d_ptr->streamer.nextWaitingElement ();
	return QJsonDocument::fromJson (jsonData, parseError);
}

qint64 Nuria::JsonStreamReader::readData (char *data, qint64 maxlen) {
	return -1;
}

qint64 Nuria::JsonStreamReader::writeData (const char *data, qint64 len) {
	using namespace Nuria::Internal;
	
	int currentWaiting = this->d_ptr->streamer.waitingElementCount ();
	StreamingJsonHelper::Status status = this->d_ptr->streamer.appendData (data, len);
	
	// Report pending elements
	int count = this->d_ptr->streamer.waitingElementCount () - currentWaiting;
	for (int i = 0; i < count; i++) {
		emit newPendingElement ();
	}
	
	// Error reporting
	if (status == StreamingJsonHelper::JsonError) {
		this->d_ptr->error = true;
		emit error ();
	}
	
	return len;
}
