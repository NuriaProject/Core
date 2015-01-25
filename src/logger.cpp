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

#include "nuria/logger.hpp"

#include <QCoreApplication>
#include <QThreadStorage>
#include <QMetaMethod>
#include "nuria/variant.hpp"
#include <unistd.h>
#include <time.h>
#include <QFile>

// Static variables
Nuria::Logger::Type Nuria::Logger::m_lowestLevel = Nuria::Logger::DefaultLowestMsgLevel;
QMap< uint32_t, Nuria::Logger::Type > Nuria::Logger::m_disabledModules;

static QIODevice *g_device = nullptr;
static bool g_isFile = false;
static bool g_deviceDisabled = false;
static Nuria::Logger::Handler g_handler;
static QThreadStorage< QByteArray > g_transaction;

#define FORMAT_STRING "[%TIME%] %TRANSACTION% %TYPE%/%MODULE%: %FILE%:%LINE% - %CLASS%::%METHOD%: %BODY%"
static int g_formatDefaultOffset[] = {0, 1, 6, 2, 13, 1, 6, 1, 8, 2, 6, 1, 6, 3, 7, 2, 8, 2, 6, 0, -1};
static const char *g_format = nullptr;
static int *g_formatOffset = g_formatDefaultOffset;

Nuria::Logger::Logger (Type type, const char *module, const char *fileName,
		     int line, const char *className, const char *methodName)
	: QDebug (&m_buffer), m_type (type), m_line (line), m_module (module),
	  m_file (0), m_class (className), m_method (methodName)
{
	
	// Skip path from fileName. Look for a backslash on windows.
#ifndef Q_OS_WIN
	const char *baseName = strrchr (fileName, '/');
#else
	const char *baseName = strrchr (fileName, '\\');
#endif
	
	if (baseName) {
		this->m_file = QLatin1String (baseName + 1);
	} else {
		this->m_file = QLatin1String (fileName);
	}
	
	// Init the output device if not already done.
	if (!g_device) {
		setOutputDevice (stdout);
	}
	
}

static void writeOutput (const char *type, const char *module, const char *file, int line,
			 const char *className, const char *method, const char *message) {
	
	// Get current time
	tm local;
	time_t timestamp = time (0);
	localtime_r (&timestamp, &local);
	
	// Time string
	char timeString[9]; // 00:00:00
	snprintf (timeString, sizeof(timeString), "%02i:%02i:%02i",
		  local.tm_hour, local.tm_min, local.tm_sec);
	
	// Construct output data
	QByteArray output;
	const char *format = (!g_format) ? FORMAT_STRING : g_format;
	const int *offsets = g_formatOffset;
	
	// Write format
	int pos = 0;
	int i = 0;
	for (; offsets[i + 1] != -1; i++) {
		
		// Is this a identifier?
		int curLen = offsets[i + 1];
		
		if (format[pos] != '%') {
			// No. Copy from 'pos' to 'cur' into output.
			output.append (format + pos, curLen);
		} else if (curLen >= 3) {
			
			// Compare identifiers.
			char a = format[pos + 1];
			char b = format[pos + 2];
			
			if (a == 'D' && b == 'A') { // DATE
				char dateString[11]; // 00/00/0000
				snprintf (dateString, sizeof(dateString), "%02i/%02i/%04i",
					  local.tm_mon + 1, local.tm_mday, local.tm_year + 1900);
				output.append (dateString);
				
			} else if (a == 'T' && b == 'I') { // TIME
				output.append (timeString);
			} else if (a == 'T' && b == 'Y') { // TYPE
				output.append (type);
			} else if (a == 'T' && b == 'R') { // TRANSACTION
				output.append (g_transaction.localData ());
			} else if (a == 'M' && b == 'O') { // MODULE
				output.append (module);
			} else if (a == 'F' && b == 'I') { // FILE
				output.append (file);
			} else if (a == 'L' && b == 'I') { // LINE
				output.append (QByteArray::number (line));
			} else if (a == 'C' && b == 'L') { // CLASS
				output.append (className);
			} else if (a == 'M' && b == 'E') { // METHOD
				output.append (method);
			} else if (a == 'B' && b == 'O') { // BODY
				output.append (message);
			}
			
		}
		
		// New position
		pos += curLen;
		
	}
	
	// Append "\n" and write output to device
	if (!output.isEmpty ()) {
		
		// 
		output.append ('\n');
		
		// Write to device. If we're dealing with a QFile, we use the
		// low-level methods to avoid problems in multi-threaded
		// environments.
		if (g_isFile) {
			QFile *file = static_cast< QFile * > (g_device);
			
			// Use write(3) to output the message and make sure that
			// the data has been written by calling fsync(3).
			int fileno = file->handle ();
			::write (fileno, output.constData (), output.length ());
#ifndef Q_OS_WIN
			::fsync (fileno);
#endif
		
		} else {
			g_device->write (output);
		}
		
	}
	
}

Nuria::Logger::~Logger () {
	
	// Remove trailing space
	if (this->m_buffer.endsWith (QLatin1Char (' ')))
		this->m_buffer.chop (1);
	
	char *temporary = nullptr;
	
	// If m_method is 0, the complete definition is in m_class.
	if (!this->m_method.latin1 ()) {
		
		temporary = qstrdup (this->m_class.latin1 ()); // Copy signature
		
		// G++ produces signatures like this: void Foo::bar()
		// MSVC produces signatures like this: void __cdecl Foo::bar()
		
		// Use do .. while so we can simply use break; on failure
		do {
			// Search for the first ( and set it to 0x0
			char *bracket = strchr (temporary, '(');
			
			if (!bracket) break;
			*bracket = 0x0;
			
			char *colon = bracket;
			for (; *colon != ':'&& *colon != ' ' && colon >= temporary; colon--);
			
			if (colon == temporary) break;
			
			// Read method name
			this->m_method = QLatin1String (colon + 1);
			
			// When *colon == ' ', then this is a method on global scope.
			// In this case set class to "" and break.
			if (*colon == ' ') {
				this->m_class = QLatin1String ("");
				break;
			}
			
			// Decrement colon and set it to 0x0.
			colon--;
			*colon = 0x0;
			
			// Now backward-search for the first space and we have the class name
			for (; *colon != ' ' && colon >= temporary; colon--);
			if (colon == temporary) break;
			
			// Read class name
			this->m_class = QLatin1String (colon + 1);
			
		} while (0);
		
	}
	
	// Convert the type into a string representation
	const char *typeString = "<Unknown>";
	switch (this->m_type) {
	case DebugMsg: typeString = "Debug"; break;
	case WarnMsg: typeString = "Warning"; break;
	case ErrorMsg: typeString = "Error"; break;
	case CriticalMsg: typeString = "Critical"; break;
	case LogMsg: typeString = "Log"; break;
	case AllLevels: break;
	}
	
	// Use the device output if enabled.
	if (!g_deviceDisabled) {
		
		writeOutput (typeString, this->m_module.latin1 (), this->m_file.latin1 (),
			     this->m_line, this->m_class.latin1 (), this->m_method.latin1 (),
			     qPrintable (this->m_buffer));
		
	}
	
	// Invoke additional output handlers
	if (g_handler) {
		
		// 
		QByteArray typeName (typeString);
		QByteArray moduleName (this->m_module.latin1 (), this->m_module.size ());
		QByteArray fileName (this->m_file.latin1 (), this->m_file.size ());
		QByteArray className (this->m_class.latin1 (), this->m_class.size ());
		QByteArray methodName (this->m_method.latin1 (), this->m_method.size ());
		
		g_handler (this->m_type, g_transaction.localData (), typeName, moduleName, fileName,
		           this->m_line, className, methodName, this->m_buffer);
		
	}
	
	// Clean up
	delete[] temporary;
	
}

void Nuria::Logger::setModuleLevel (const char *module, Nuria::Logger::Type leastLevel) {
	if (!module) {
		m_lowestLevel = leastLevel;
		return;
	}
	
	uint32_t hash = jenkinsHash (module, strlen(module));
	if (leastLevel == DebugMsg) {
		m_disabledModules.remove (hash);
	} else {
		m_disabledModules.insert (hash, leastLevel);
	}
	
}

bool Nuria::Logger::isModuleDisabled (const char *module, Nuria::Logger::Type level) {
	if (!module) {
		return (level < m_lowestLevel);
	}
	
	uint32_t hash = jenkinsHash (module, strlen(module));
	return isModuleDisabled (hash, level);
}

void Nuria::Logger::qtMessageHandler (QtMsgType type, const QMessageLogContext &context,
				     const QString &message) {
	
	Type t = DebugMsg;
	
	// Translate QtMsgType to a Debug::Type value
	switch (type) {
	case QtDebugMsg:
		t = DebugMsg;
		break;
	case QtWarningMsg:
		t = WarnMsg;
		break;
	case QtCriticalMsg:
	case QtFatalMsg:
		t = CriticalMsg;
		break;
	}
	
	// Output.
	const char *category = (context.category) ? context.category : "";
	const char *file = (context.file) ? context.file : "";
	const char *function = (context.function) ? context.function : "";
	Logger out (t, category, file, context.line, function, nullptr);
	out.setBuffer (message);
	
}

void Nuria::Logger::installMessageHandler () {
	qInstallMessageHandler (qtMessageHandler);
}

void Nuria::Logger::setOutputDisabled (bool disabled) {
	g_deviceDisabled = disabled;
}

bool Nuria::Logger::isOutputDisabled () {
	return g_deviceDisabled;
}

void Nuria::Logger::setOutputDevice (FILE *handle) {
	
	// Create a QFile from handle
	QFile *device = new QFile;
	device->open (handle, QIODevice::WriteOnly);
	
	// Set device
	setOutputDevice (device);
	
}

void Nuria::Logger::setOutputDevice (QIODevice *device) {
	
	// Delete old device
	delete g_device;
	
	// Store new device and transfer ownership
	device->setParent (qApp);
	g_device = device;
	
	// Store if device is a QFile
	g_isFile = device->inherits ("QFile");
	
}

void Nuria::Logger::setOutputHandler (const Handler &handler) {
	g_handler = handler;
}

void Nuria::Logger::setOutputFormat (const char *format) {
	
	if (g_format) {
		delete[] g_format;
		delete g_formatOffset;
	}
	
	// 
	g_format = (!format) ? nullptr : qstrdup (format);
	
	// Do we need to calculate the offset array?
	if (!g_format) {
		g_formatOffset = g_formatDefaultOffset;
		return;
	}
	
	// Yes. First count the occurences of '%'
	int size = 0;
	for (int i = 0; format[i]; i++) {
		size += (format[i] == '%') ? 1 : 0;
	}
	
	// Allocate
	g_formatOffset = new int [size + 3];
	
	// Offset array position
	g_formatOffset[0] = 0;
	int o = 1;
	
	// 
	int offset = 0;
	int i;
	bool inIdent = false;
	for (i = 0; format[i]; i++) {
		
		if (format[i] != '%') {
			offset++;
			continue;
		}
		
		// 
		inIdent = !inIdent;
		g_formatOffset[o] = offset + (inIdent ? 0 : 2);
		offset = 0;
		o++;
		
	}
	
	// Mark end
	g_formatOffset[o] = offset;
	g_formatOffset[o + 1] = -1;
	
}

QByteArray Nuria::Logger::transaction () {
	return g_transaction.localData ();
}

void Nuria::Logger::setTransaction (const QByteArray &transaction) {
	g_transaction.setLocalData (transaction);
}

void Nuria::Logger::setBuffer (const QString &buffer) {
	this->m_buffer = buffer;
}


Nuria::LoggerTransaction::LoggerTransaction (const QByteArray &transaction)
        : m_oldTransaction (g_transaction.localData ())
{
	g_transaction.setLocalData (transaction);
}

Nuria::LoggerTransaction::~LoggerTransaction () {
	g_transaction.setLocalData (this->m_oldTransaction);
}
