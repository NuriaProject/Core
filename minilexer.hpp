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

#ifndef NURIA_MINILEXER_HPP
#define NURIA_MINILEXER_HPP

#include "essentials.hpp"
#include <QVariant>
#include <QObject>
#include <QRegExp>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QRegularExpression>
#endif

namespace Nuria {
class MiniLexerPrivate;

/**
 * Helper class for MiniLexer::addDefinition.
 * \sa MiniLexer::addDefinition
 */
class NURIA_CORE_EXPORT LexerRule {
public:
	inline LexerRule (const QString &name = QString ()) : m_name (name) { }
	inline LexerRule (const char *name) : m_name (QLatin1String (name)) { }
	inline const QString &name () const { return this->m_name; }
	inline void setName (const QString &name) { this->m_name = name; }
private:
	QString m_name;
};

/**
 * Helper class for MiniLexer::addDefinition.
 * \sa MiniLexer::addDefinition
 */
class NURIA_CORE_EXPORT LexerDefinition {
public:
	inline LexerDefinition (const QString &name = QString ()) : m_name (name) { }
	inline LexerDefinition (const char *name) : m_name (QLatin1String (name)) { }
	inline const QString &name () const { return this->m_name; }
	inline void setName (const QString &name) { this->m_name = name; }
private:
	QString m_name;
};

/**
 * \brief A simple lexer.
 * You can use this class if you want to parse some data based on regular
 * expressions. It is not really smart, as it is meant to be used for simple
 * tasks. NQL uses it for example to parse queries.
 * 
 * \warning If you know Flex then be warned that the meanings of 'rules' and
 * 'definitions' are switched in MiniLexer.
 * 
 * \note If compiled on Qt5, QRegularExpression is also supported.
 * 
 * \par Example
 * 
 * \note This example assumes that you have
 * \codeline using namespace Nuria;
 * somewhere in your code.
 * 
 * Lets see a little example. Lets say we want to write a INI-Parser. First, we
 * need a MiniLexer instance. We also create a enumeration of possible tokens.
 * \code
 * enum IniTokens { TokenGroup, TokenKey, TokenValue };
 * MiniLexer lexer;
 * \endcode
 * Next we need some rules. A line in an ini file is either a group declaration
 * or a Key=Value pair.\n
 * A group looks like this: <i>[Groupname]</i>\n
 * A Key=Value pair looks like: <i>Key = Value</i>\n
 * The whitespace in front and back of the equal sign is optional.
 * So, our first rule will take care of groups:
 * \code
 * // Rule for INI groups. Accepts anything in the brackets excluding [].
 * lexer.addRule ("Group", QRegExp ("^\\[[^\\[\\]]+\\]$"), TokenGroup);
 * \endcode
 * We head directly to the next two rules:
 * \code
 * // Matches on anything before the first equal sign
 * lexer.addRule ("Key", QRegExp ("[^=]+"), TokenKey);
 * 
 * // Matches on everything to the newline character
 * lexer.addRule ("Value", QRegExp ("[^\n]*", TokenValue);
 * \endcode
 * Okay, now we need to tell MiniLexer how a line must look like. For this, we
 * define two definitions. One for groups, and the other one for Key=Value pairs:
 * \code
 * lexer.addDefinition (QString(), QVariantList() << QVariant::fromValue (LexerRule ("Group")));
 * lexer.addDefinition (QString(), QVariantList() << QVariant::fromValue (LexerRule ("Key"))
 *     << "=" << QVariant::fromValue (LexerRule ("Value")));
 * \endcode
 * Next part, the lexing itself. We open up a file called "test.ini" using QFile
 * and lex line after line, printing whatever we get. We break up if something goes wrong.
 * \code
 * QFile file ("test.ini");
 * if (!file.open (QIODevice::ReadOnly | QIODevice::Text)) {
 *     qFatal ("Failed to open test.ini");
 * }
 * 
 * while (!file.atEnd ()) {
 *     if (!lexer.lex (file.readLine ())) {
 *         qFatal ("Syntax error.");
 *     }
 *     
 *     if (lexer.token (0) == TokenGroup) {
 *         qDebug() << "Group:" << lexer.value (0);
 *     } else {
 *         qDebug() << lexer.value (0) << "->" << lexer.value (1);
 *     }
 * }
 * \endcode
 * 
 * And thats it. You could also use QSettings when you want to read ini files, but
 * wheres the fun in it if you can also use a lexer? :P
 */
class NURIA_CORE_EXPORT MiniLexer : public QObject {
	Q_OBJECT
public:
	typedef QPair< int, QString > TokenValue;
	typedef QList< TokenValue >  TokenValueList;
	
	explicit MiniLexer (QObject *parent = 0);
	~MiniLexer ();
	
	/**
	 * Returns if strings are matched case-sensitive or case-insensitive.
	 * \sa setMatchSensitivity
	 */
	Qt::CaseSensitivity matchSensitivity () const;
	
	/**
	 * Sets if strings should be matched case-sensitive or case-insensitive.
	 * This only applies to strings and has no effect on regular expressions.
	 * Default is case-sensitive matching.
	 */
	void setMatchSensitivity (Qt::CaseSensitivity value);
	
	/**
	 * Returns the count of parsed tokens.
	 */
	int length () const;
	
	/**
	 * Returns a list of TokenValue's containing the current list of token and values.
	 */
	TokenValueList tokenValueList () const;
	
	/**
	 * Returns a single TokenValue pair. TokenValue is a typedef for
	 * \c QPair<int,QString>. \a first is the token id, \a second
	 * the value.
	 */
	TokenValue tokenValue (int at) const;
	
	/**
	 * Returns the value of a token/value pair at position \a at.
	 */
	const QString &value (int at) const;
	
	/**
	 * Returns the token of a token/value pair at position \a at.
	 */
	int token (int at) const;
	
	/** 
	 * Adds a rule to the lexer. \a name must be unique (if there is already
	 * a rule with the same name, it will be overwritten). \a regExp is the
	 * regular expressions which is connected to this rule. \a token is a
	 * user-defined token id which is used to distinguish between different
	 * rules after the lexer has been run.
	 * \note When you pass \c -1 as \a token, matches for this rule won't be
	 * stored for later use.
	 * \note If you only care about a certain part of the match, then capture
	 * it using (brackets). MiniLexer will take the first captured text as value
	 * if it finds any. Else, it takes the whole match as value.
	 */
	void addRule (const QString &name, const QRegExp &regExp, int token);
	
	/** \overload */
	void addRule (const QString &name, const QString &string, int token);
	
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	/** \overload */
	void addRule (const QString &name, const QRegularExpression &regExp, int token);
#endif
	
	/**
	 * Adds a definition to the lexer. A definition declares how the input
	 * should look like (Think of it being like BNF, but using C++ stuff to
	 * describe things instead of some text input).
	 * You can do that by using strings, regular expressions, rules and other
	 * definitions in the \a def list. You can add multiple definitions to a
	 * \a name. You can't remove a definition later. MiniLexer isn't
	 * really smart when it comes to matching. That being said, make sure that
	 * when a specific match order of definitions with the same \a name is
	 * important, MiniLexer tries from the top which is least-important to
	 * most-important at the bottom.
	 * \note MiniLexer uses "" as start definition. To add a start definition
	 * simply pass \c QString() as \a name.
	 * \note To put a reference to a rule into the list, use:
	 * QVariant::fromValue(LexerRule("Name of the rule"))
	 * \note Same goes for references to definitions: 
	 * QVariant::fromValue(LexerDefinition("Name of the definition"))
	 */
	void addDefinition (const QString &name, const QVariantList &def);
	
	/**
	 * Lexes \a data. Returns \a true if \a data could be completely parsed,
	 * returns \a false otherwise.
	 * \sa tokenValueList tokenValue token value lastError
	 */
	bool lex (const QString &data);
	
	/**
	 * If lex fails, this function will return a human-readable string
	 * describing the problem.
	 */
	QString lastError () const;
	
	/**
	 * If lex fails, this function will return the position where the
	 * error occured.
	 */
	int errorPosition () const;
	
	/**
	 * Returns \a true if this instance has a start definition (e.g. a
	 * definition with the name "").
	 */
	bool hasStartDefinition () const;
	
	/**
	 * Returns \a true if there is a rule with the name \a name.
	 */
	bool hasRule (const QString &name) const;
	
	/**
	 * Returns \a true if there is a definition with the name \a name.
	 */
	bool hasDefinition (const QString &name) const;
	
	/**
	 * Creates a MiniLexer instance based on a definition string.
	 * If parsing failed, the hasStartDefinition() method of the returned
	 * MiniLexer instance will return \a false. \a error will contain a
	 * human-readable error message.\n
	 * \a definition must be a string with the following format:
	 * <ul>
	 * <li>\c # Starts a comment. It ends at the end of the line.
	 * It can only appear at the beginning of a line</li>
	 * <li>\c Rule(\c Token ): \c Body
	 * <ul>
	 * <li>\c Rule = The name of the rule. May be prefixed with a $</li>
	 * <li>\c Token = The token id as integer</li>
	 * <li>\c Body = The rule body. May be a "string" or a /regex/</li>
	 * </ul></li>
	 * 
	 * <li>\c Definiton = \c Body ;
	 * <ul>
	 * <li>\c Definition = The name of the definition. Must not be prefixed with a $</li>
	 * <li>\c Body = The body of the definition. The body contains one or more items
	 * separated with whitespace. The body ends with a semicolon ';'. Possible items
	 * are:
	 * <ul>
	 * <li>A C-style string literal enclosed in ""</li>
	 * <li>A regular expression enclosed in // (Slashes). A single 'i' after the closing
	 * slash marks the regular expression to match case-insensitive.</li>
	 * <li>A name of a definition. Just write the name itself.</li>
	 * <li>A name of a rule. Prefix the name with a dollar-sign '$'</li>
	 * </ul>
	 * </ul>
	 * </li>
	 * </ul>
	 * \note You can format the definition string using different types of whitespace.
	 * The method is pretty free in that regard.
	 * \note Also read the docs of addRule and addDefinition. They also apply on this function.
	 */
	static MiniLexer *createInstanceFromDefinition (const QString &definition, QString &error);
	
private:
	
	/**
	 * Helper function used by lexDefinition. It chops off anything in
	 * \a m_values and \a m_valueTokens after \a lastValidLength.
	 * This is needed when lexDefinition (or lex) tries to match
	 * different definitions.
	 */
	void chopValueList (int lastValidLength);
	
	/**
	 * Internal function used by lex(). This function does the real work.
	 */
	bool lexDefinition (const QVariantList &def, const QString &data, int &pos);
	
	// 
	MiniLexerPrivate *d_ptr;
	
};

}

Q_DECLARE_METATYPE(Nuria::LexerRule)
Q_DECLARE_METATYPE(Nuria::LexerDefinition)

#endif // NURIA_MINILEXER_HPP
