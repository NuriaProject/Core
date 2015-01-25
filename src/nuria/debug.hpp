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

#ifndef NURIA_LOGGER_HPP
#define NURIA_LOGGER_HPP

#include "essentials.hpp"
#include "callback.hpp"
#include <QDebug>

namespace Nuria {

/**
 * \brief Logging class of the Nuria Framework.
 * 
 * This class provides an easy way for outputting useful logging messages.
 * You can use this as a drop-in replacement for QDebug.
 * 
 * Defining NURIA_LOGGER_NO_{DEBUG,LOG,WARN,ERROR,CRITICAL} will disable
 * the level at compile-time, causing the corresponding logging macro to
 * create code the compiler can optimize out.
 * 
 * \par Redirecting output
 * By default all output is sent to stdout. 
 * 
 * You can redirect it using setOutputDevice(). Use setOutputHandler()
 * to redirect logging data to a method of yours instead.
 * 
 * \par QDebug and Nuria::Logger
 * Nuria::Logger is meant as drop-in replacement for QDebug. You can still use
 * qDebug(), qWarning(), ... directly but still let Nuria::Logger do the work.
 * \sa qtMessageHandler
 * 
 * \par Modules
 * Nuria::Logger knows the concept of "modules". A message can be in a single
 * module, which is printed next to the message itself. Additionally, you can
 * use enableModule and disableModule to change the logging behaviour at
 * run-time.
 * 
 * The module name of a translation unit can be set by #defining NURIA_MODULE.
 * The value of it is expected to be a literal. The line containing the #define
 * must appear before the line where logger.hpp is #include'd.
 *
 * \code
 * #define NURIA_MODULE "ModuleName"
 * #include <nuria/logger.hpp>
 * \endcode
 * 
 * \note The default module name is "" (Empty string) which is used if you
 * don't provide a value on your own. No warning is issued.
 * 
 * \par Example
 * \code
 * // Before including logger.hpp
 * #define NURIA_MODULE "Example"
 * 
 * // 
 * nDebug() << "Hello, this is a debug message";
 * nLog() << "Something worth logging happend!";
 * nWarn() << "I'm a warning";
 * nError() << "Some error occured :(";
 * nCritical() << "Some unrecoverable error occured.";
 * \endcode
 * 
 * \par Transaction-oriented logging
 * Nuria::Logger supports a thread-wide transaction to be set. The usage is to
 * set the transaction name at the beginning of a transaction (E.g. a unique
 * identifier of the action, a user id, or similar), and then reset it to the
 * default empty one when you're done. You can use LoggerTransaction as helper
 * class to manage transactions.
 * 
 * \sa Logger::setTransaction LoggerTransaction
 * 
 * \par Outputting custom types
 * If you want to output custom types you simply overload operator<< for QDebug:
 * \code
 * QDebug operator<< (QDebug out, const MyType &instance) {
 * 	out.nospace () << "(" << instance.name () << ")";
 * }
 * \endcode
 * \note For a more complete example see
 * http://doc.qt.io/qt-5/custom-types.html#making-the-type-printable
 * 
 */
class NURIA_CORE_EXPORT Logger : public QDebug {
public:
	
	/** Type enumeration. */
	enum Type {
		DebugMsg = 0,
		LogMsg = 1,
		WarnMsg = 2,
		ErrorMsg = 3,
		CriticalMsg = 4,
		
		AllLevels = CriticalMsg + 1,
		DefaultLowestMsgLevel = DebugMsg
		
	};
	
	/** Output handler */
	typedef std::function< void(Nuria::Logger::Type /* type */, const QByteArray &/* typeName */,
	                            const QByteArray &/* transaction */, const QByteArray &/* moduleName */,
	                            const QByteArray &/* file */, int /* line */, const QByteArray &/* className */,
	                            const QByteArray &/* methodName */, const QString &/* message */) > Handler;
	
	/**
	 * Constructor. You usually don't use this directly.
	 * Use nDebug, nWarn, nError, nCritical or nLog instead.
	 */
	Logger (Type type, const char *module, const char *fileName, int line,
	       const char *className, const char *methodName);
	
	/** Destructor. Writes the output data. */
	~Logger ();
	
	/**
	 * Sets the logging level of a certain module. \a leastLevel is
	 * non-inclusive, which means that passing \c LogMsg will output for
	 * everything below that in \a module. To completely disable logging,
	 * pass \c AllLevels.
	 * 
	 * Passing \c nullptr for \a module acts as a wildcard, affecting all
	 * modules.
	 * 
	 * \sa enableCategory
	 */
	static void setModuleLevel (const char *module, Type leastLevel);
	
	/**
	 * Returns \c true if \a module at \a level is disabled.
	 */
	static bool isModuleDisabled (const char *module, Type level);
	
	/** \internal Fast access for logging macros. */
	static inline bool isModuleDisabled (uint32_t module, Type level) {
		return (level < m_lowestLevel ||
			level < m_disabledModules.value (module, DefaultLowestMsgLevel));
	}
	
	/**
	 * Use this function in combination with qInstallMsgHandler to tunnel
	 * all QDebug data through Logger. This way legacy code which uses
	 * qDebug() etc. will still use the log paths you defined.
	 * 
	 * With Qt5 this message handler also supports output of file name,
	 * line numbers and method names in debug mode. These will not be
	 * available in release mode.
	 * 
	 * \sa installMessageHandler
	 */
	static void qtMessageHandler (QtMsgType type, const QMessageLogContext &context, const QString &message);

	/** Install the QDebug message handler. */
	static void installMessageHandler ();
	
	/**
	 * Use this method to disable or enable the default output method.
	 * By default the default output method is enabled.
	 * \note This only affects the usage of the \c default output
	 * which you can manipulate through setOutputDevice. It does not
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
	 * 
	 * \note Logger takes ownership of \a handle.
	 */
	static void setOutputDevice (FILE *handle);
	
	/**
	 * \overload Uses \a device as output device.
	 * 
	 * \warning \a device must be thread-safe for write-access when the
	 * application uses multi-threading.
	 */
	static void setOutputDevice (QIODevice *device);
	
	/**
	 * Installs \a handler as output handler, which is called every time a
	 * logging message should be written or otherwise processed.
	 * 
	 * There can only be one output handler at any given time. When there's
	 * already one installed, the old one will be overridden.
	 * 
	 * \warning \a handler will be called from the thread logging data. This
	 * means it must be thread-safe in multi-threaded applications.
	 * 
	 * \sa setOutputDisabled
	 */
	static void setOutputHandler (const Handler &handler);
	
	/**
	 * Sets the format which is used to write a message into the output
	 * stream. If \a format is \c 0 the default format will be used.
	 * 
	 * \note The default output format is:
	 * "[%TIME%] %TRANSACTION% %TYPE%/%MODULE%: %FILE%:%LINE% - %CLASS%::%METHOD%: %BODY%"
	 * 
	 * \par Identifiers
	 * - %DATE% The current date (MM/DD/YYYY)
	 * - %TIME% The current time (HH:MM:SS)
	 * - %TYPE% The message type ("Debug", "Warning", ...)
	 * - %TRANSACTION% Transaction name
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
	
	/**
	 * Returns the current transaction. The default transactions name is
	 * empty.
	 */
	static QByteArray transaction ();
	
	/** Sets \a transaction for the current thread only. */
	static void setTransaction (const QByteArray &transaction);
	
private:
	
	void setBuffer (const QString &buffer);
	
	// Exposing these for faster access
	static Type m_lowestLevel;
	static QMap< uint32_t, Type > m_disabledModules;
	
	QString m_buffer;
	Type m_type;
	int m_line;
	QLatin1String m_module;
	QLatin1String m_file;
	QLatin1String m_class;
	QLatin1String m_method;
	
};

/**
 * \brief Helper class for temporarily setting the logger transaction
 * 
 * This is a simplistic class which when constructed sets the threads' current
 * transaction to the passed name and resets it to the old one in its
 * destructor.
 */
class NURIA_CORE_EXPORT LoggerTransaction {
public:
	
	/**
	 * Constructor. Will set \a transaction as this threads' current logging
	 * transaction.
	 */
	LoggerTransaction (const QByteArray &transaction = QByteArray ());
	
	/** Destructor. Resets the transaction. */
	~LoggerTransaction ();
	
private:
	QByteArray m_oldTransaction;
	
};

class NURIA_CORE_EXPORT LoggerIgnore {
public:
	template< typename T >
	inline LoggerIgnore &operator<< (const T &)
	{ return *this; }
};

}

Q_DECLARE_METATYPE(Nuria::Logger::Type)

// Macro magic
#ifndef NURIA_MODULE
# define NURIA_MODULE ""
#endif

#define NURIA_LOGGER(type) \
	if (Nuria::Logger::isModuleDisabled \
	(Nuria::jenkinsHash (NURIA_MODULE, sizeof(NURIA_MODULE) - 1), type)) {} else \
	Nuria::Logger(type, NURIA_MODULE, __FILE__, __LINE__, Q_FUNC_INFO, 0)

#ifndef NURIA_LOGGER_NO_DEBUG
#define nDebug() NURIA_LOGGER(Nuria::Logger::DebugMsg)
#else
#define nDebug() Nuria::LoggerIgnore()
#endif

#ifndef NURIA_LOGGER_NO_LOG
#define nLog() NURIA_LOGGER(Nuria::Logger::LogMsg)
#else
#define nLog() Nuria::LoggerIgnore()
#endif

#ifndef NURIA_LOGGER_NO_WARN
#define nWarn() NURIA_LOGGER(Nuria::Logger::WarnMsg)
#else
#define nWarn() Nuria::LoggerIgnore()
#endif

#ifndef NURIA_LOGGER_NO_ERROR
#define nError() NURIA_LOGGER(Nuria::Logger::ErrorMsg)
#else
#define nError() Nuria::LoggerIgnore()
#endif

#ifndef NURIA_LOGGER_NO_CRITICAL
#define nCritical() NURIA_LOGGER(Nuria::Logger::CriticalMsg)
#else
#define nCritical() Nuria::LoggerIgnore()
#endif

#endif // NURIA_LOGGER_HPP
