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

#ifndef NURIA_DEBUG_HPP
#define NURIA_DEBUG_HPP

#include "essentials.hpp"
#include "callback.hpp"
#include <QDebug>

namespace Nuria {

/**
 * \brief Debugging and logging class of the Nuria Framework.
 * 
 * This class provides an easy way for outputting useful logging messages.
 * You can use this as a drop-in replacement for QDebug.
 * 
 * \note Despite its name it's also usable as a logger.
 * 
 * \par Extending
 * As time of writing, Debug is not easily extendable in regards of message
 * types. It is possible through the setDestination methods to redirect the
 * output. If this isn't enough you can also install custom "output handlers"
 * using installOutputHandler.
 * 
 * \par QDebug and Nuria::Debug
 * Nuria::Debug is meant as drop-in replacement for QDebug. You can still use
 * qDebug(), qWarning(), ... directly but still let Nuria::Debug do the work.
 * Please note that Nuria::Debug can't determine your module, the source file,
 * line number, class or method name when you use qDebug().
 * \sa qtMessageHandler
 * 
 * \par Module
 * Another thing to mention is that Nuria::Debug supports module names. But not
 * in a traditional manner where you pass the name to each logger instance.
 * Instead Nuria::Debug uses a #define named NURIA_MODULE. The value must be a
 * literal enclosed in "". It is important that you put this line \b before
 * including debug.hpp like this:
 * 
 * \code
 * #define NURIA_MODULE "ModuleName"
 * #include <nuria/debug.hpp>
 * \endcode
 * 
 * \note The default value is "" (A empty string literal) which is used if you
 * don't provide a value on your own. No warning is issued.
 * 
 * \par Example
 * \code
 * // Before including debug.hpp
 * #define NURIA_MODULE "Example"
 * 
 * // 
 * nDebug() << "Hello, this is a debug message";
 * nWarn() << "I'm a warning";
 * nError() << "Some error occured :(";
 * nCritical() << "Some unrecoverable error occured.";
 * nLog() << "Something worth logging happend!";
 * \endcode
 * 
 * \par Outputting custom types
 * If you want to output custom types you simply overload operator<< for QDebug:
 * \code
 * QDebug operator<< (QDebug out, const MyType &instance) {
 * 	out.nospace () << "(" << instance.name () << ")";
 * }
 * \endcode
 * \note For a more complete example 
 * <a href="http://qt-project.org/doc/custom-types.html#making-the-type-printable">read this</a>
 * 
 */
class NURIA_CORE_EXPORT Debug : public QDebug {
public:
	
	/** Type enumeration. */
	enum Type {
		DebugMsg = 0,
		WarnMsg = 1,
		ErrorMsg = 2,
		CriticalMsg = 3,
		LogMsg = 4
	};
	
	/**
	 * Constructor. You usually don't use this directly.
	 * Use nDebug, nWarn, nError, nCritical or nLog instead.
	 */
	Debug (Type type, const char *module, const char *fileName, int line,
	       const char *className, const char *methodName);
	
	/**
	 * Destructor. Writes the debug data to the output stream.
	 */
	~Debug ();
	
	/**
	 * Use this function in combination with qInstallMsgHandler to tunnel
	 * all QDebug data through Debug. This way legacy code which uses
	 * qDebug() etc. will still use the log paths you defined.
	 * 
	 * With Qt5 this message handler also supports output of file name,
	 * line numbers and method names.
	 * \sa installMessageHandler
	 */
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	static void qtMessageHandler (QtMsgType type, const QMessageLogContext &context, const QString &message);
#else
	static void qtMessageHandler (QtMsgType type, const char *message);
#endif
	
	/** 
	 * Automatically installs the QDebug message handler.
	 * Works on Qt4 and Qt5.
	 */
	static void installMessageHandler ();
	
	/**
	 * Use this method to disable or enable the default output method.
	 * By default the default output method is enabled.
	 * \note This only affects the usage of the \c default output
	 * which you can manipulate through setDestination. It does not
	 * disable your output handlers.
	 */
	static void setOutputDisabled (bool disabled);
	
	/**
	 * Returns \c true if the default output is disabled (as in not used).
	 */
	static bool isOutputDisabled ();
	
	/**
	 * Use this method to redirect the logging output to some other
	 * destination. By default \c stdout is used.
	 * \note Debug takes ownership of \a handle.
	 */
	static void setDestination (FILE *handle);
	
	/**
	 * \overload
	 * Uses \a device for output.
	 * \note Debug takes ownership of \a device.
	 */
	static void setDestination (QIODevice *device);
	
	/**
	 * Installs an output handler. A output handler is called each time the
	 * logging interface is used. You can use this if you need  more control
	 * over logging in your application, for example if a single output file
	 * isn't enough for you.
	 * 
	 * The prototype for the callback looks like this:
	 * \code
	 * void (Nuria::Debug::Type type, QByteArray typeName,
	 *	 QByteArray moduleName, QByteArray file, int line,
	 *	 QByteArray className, QByteArray methodName,
	 *	 QString message);
	 * \endcode
	 * 
	 * \param type The type of this message
	 * \param typeName A human-readable representation of \a type
	 * \param moduleName The module name
	 * \param file The file name (As generated by the __FILE__ macro)
	 * \param line The line number (As generated by the __LINE__ macro)
	 * \param className The name of the class that sent the message
	 * \param methodName The method name which sent the message
	 * \param message The message itself
	 * 
	 * \sa uninstallOutputHandler
	 */
	static void installOutputHandler (const Callback &callback);
	
	/**
	 * Removes an output handler.
	 */
	static void uninstallOutputHandler (const Callback &callback);
	
	/**
	 * Sets the format which is used to write a message into the output
	 * stream. If \a format is \c 0 the default format will be used.
	 * 
	 * \note The default output format is:
	 * "[%TIME%] %TYPE%/%MODULE%: %FILE%:%LINE% - %CLASS%::%METHOD%: %MESSAGE%"
	 * 
	 * \par Identifiers
	 * - %DATE% The current date (MM/DD/YY)
	 * - %TIME% The current time (HH:MM:SS)
	 * - %TYPE% The message type ("Debug", "Warning", ...)
	 * - %MODULE% The module name
	 * - %FILE% The source file name
	 * - %LINE% The line number inside %FILE%
	 * - %CLASS% Name of the class
	 * - %METHOD% Method which sent the message
	 * - %BODY% The message body
	 * 
	 * \note Identifiers are case-sensitive.
	 * 
	 * A new-line character is automatically appended.
	 */
	static void setOutputFormat (const char *format);
	
#if 0
	/**
	 * Special output operator for QVariant which uses Nuria::Variant to
	 * output more useful data.
	 */
	Debug &operator<< (Debug &dbg, const QVariant &variant);
#endif
	
private:
	
	void setBuffer (const QString &buffer);
	
	QString m_buffer;
	Type m_type;
	int m_line;
	QLatin1String m_module;
	QLatin1String m_file;
	QLatin1String m_class;
	QLatin1String m_method;
	
};

}

Q_DECLARE_METATYPE(Nuria::Debug::Type)

// Macro magic
#ifndef NURIA_MODULE
# define NURIA_MODULE ""
#endif

#define NURIA_DEBUG(type) Nuria::Debug(type, NURIA_MODULE, __FILE__, __LINE__, Q_FUNC_INFO, 0)

#define nDebug() NURIA_DEBUG(Nuria::Debug::DebugMsg)
#define nWarn() NURIA_DEBUG(Nuria::Debug::WarnMsg)
#define nError() NURIA_DEBUG(Nuria::Debug::ErrorMsg)
#define nCritical() NURIA_DEBUG(Nuria::Debug::CriticalMsg)
#define nLog() NURIA_DEBUG(Nuria::Debug::LogMsg)

#endif // NURIA_DEBUG_HPP
