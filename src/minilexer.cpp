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

#include "minilexer.hpp"

#include <QMetaType>

namespace Nuria {
/**
 * Helper struct for rules
 */
struct Rule {
	QVariant data; /// Can be a QRegExp or a QString (or a QRegularExpression)
	int token; /// Associated token id
};

class MiniLexerPrivate {
public:
	Qt::CaseSensitivity sensitivity; /// For matching strings
	QMap< QString, Rule > rules; /// Rules 
	QMap< QString, QList< QVariantList > > defs; /// Definitions
	
	QList< QString > values; /// Values
	
	/**
	 * Tokens of the values. Items in this list have the
	 * same index as their counterparts in \a m_values.
	 */
	QList< int > valueTokens;
	
	QString expected; /// Currently unused. Stores what was expected instead.
	int errorPos; /// Position where an error occured.
};

}

/** Tokens for MiniLexer::createInstanceFromDefinition */
enum DefinitionTokens {
	TokenName = 1,
	TokenId = 2,
	TokenRule = 3,
	TokenDefinition = 5,
	TokenString = 6,
	TokenRegExp = 7,
	TokenDefEnd = 8
};

Nuria::MiniLexer::MiniLexer (QObject *parent)
	: QObject (parent), d_ptr (new MiniLexerPrivate) 
{
	
	this->d_ptr->sensitivity = Qt::CaseSensitive;
	this->d_ptr->errorPos = -1;
	
}

Nuria::MiniLexer::~MiniLexer () {
	delete this->d_ptr;
}

Qt::CaseSensitivity Nuria::MiniLexer::matchSensitivity () const {
	return this->d_ptr->sensitivity;
}

void Nuria::MiniLexer::setMatchSensitivity (Qt::CaseSensitivity value) {
	this->d_ptr->sensitivity = value;
}

int Nuria::MiniLexer::length () const {
	return this->d_ptr->values.length ();
}

Nuria::MiniLexer::TokenValueList Nuria::MiniLexer::tokenValueList () const {
	TokenValueList list;
	list.reserve (this->d_ptr->values.length ());
	
	for (int i = 0; i < this->d_ptr->values.length (); i++) {
		list.append (qMakePair (this->d_ptr->valueTokens.at (i), this->d_ptr->values.at (i)));
	}
	
	return list;
}

Nuria::MiniLexer::TokenValue Nuria::MiniLexer::tokenValue (int at) const {
	return qMakePair (this->d_ptr->valueTokens.at (at), this->d_ptr->values.at (at));
}

const QString &Nuria::MiniLexer::value (int at) const {
	return this->d_ptr->values.at (at);
}

int Nuria::MiniLexer::token (int at) const {
	return this->d_ptr->valueTokens.at (at);
}

void Nuria::MiniLexer::addRule (const QString &name, const QRegExp &regExp, int token) {
	Rule rule;
	rule.data = regExp;
	rule.token = token;
	
	this->d_ptr->rules.insert (name, rule);
	
}

void Nuria::MiniLexer::addRule (const QString &name, const QString &string, int token) {
	Rule rule;
	rule.data = string;
	rule.token = token;
	
	this->d_ptr->rules.insert (name, rule);
	
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
void Nuria::MiniLexer::addRule (const QString &name, const QRegularExpression &regExp, int token) {
	Rule rule;
	rule.data = regExp;
	rule.token = token;
	
	this->d_ptr->rules.insert (name, rule);
}
#endif

void Nuria::MiniLexer::addDefinition (const QString &name, const QVariantList &def) {
	QList< QVariantList > list = this->d_ptr->defs.value (name);
	list.prepend (def);
	this->d_ptr->defs.insert (name, list);
}

bool Nuria::MiniLexer::lex (const QString &data) {
	
	// Reset internal variables
	this->d_ptr->values.clear ();
	this->d_ptr->valueTokens.clear ();
	this->d_ptr->expected.clear ();
	this->d_ptr->errorPos = -1;
	
	QList< QVariantList > list = this->d_ptr->defs.value (QString ());
	
	// Iterate over start definitions. Try to match one of them.
	for (int i = 0; i < list.length (); i++) {
		
		int pos = 0;
		if (lexDefinition (list.at (i), data, pos)) {
			// Return false if not matched completely.
			return (pos >= data.length ());
		}
		
	}
	
	// No definition matched.
	return false;
}

QString Nuria::MiniLexer::lastError () const {
	
	if (this->d_ptr->expected.isEmpty ()) {
		return tr("Unrecognized character at position %1")
				.arg (this->d_ptr->errorPos + 1);
	}
	
	return tr("Unrecognized character at position %1, expected %2")
			.arg (this->d_ptr->errorPos + 1).arg (this->d_ptr->expected);
	
}

int Nuria::MiniLexer::errorPosition () const {
	return this->d_ptr->errorPos;
}

bool Nuria::MiniLexer::hasStartDefinition () const {
	return this->d_ptr->defs.contains (QString ());
}

bool Nuria::MiniLexer::hasRule (const QString &name) const {
	return this->d_ptr->rules.contains (name);
}

bool Nuria::MiniLexer::hasDefinition (const QString &name) const {
	return this->d_ptr->defs.contains (name);
}

static QVariant stringToVariant (const QString &value) {
	
	if (value.startsWith (QLatin1Char ('"'))) { // A string
		return value.mid (1, value.length () - 2);
	} else if (value.startsWith (QLatin1Char ('/'))) { // A regexp
		QRegExp rx (value.mid (1, value.lastIndexOf (QLatin1Char ('/')) - 1));
		
		// Apply case insensitivity if there is a 'i' at the end.
		if (value.endsWith (QLatin1Char ('i'), Qt::CaseInsensitive)) {
			rx.setCaseSensitivity (Qt::CaseInsensitive);
		}
		
		return rx;
	} else if (value.startsWith (QLatin1Char ('$'))) { // A rule
		return QVariant::fromValue (Nuria::LexerRule (value.mid (1)));
	} 
	
	// A definition
	// If value == "START", then it referes to the start definition.
	if (value == QLatin1String ("START"))
		return QVariant::fromValue (Nuria::LexerDefinition (QString ()));
	
	return QVariant::fromValue (Nuria::LexerDefinition (value));
	
}

Nuria::MiniLexer *Nuria::MiniLexer::createInstanceFromDefinition (const QString &definition, QString &error) {
	
	// We use a lexer to tokenize the definition string to create a lexer. Yes.
	MiniLexer lexer;
	
	QRegExp ws ("[ \t\r\n]+");
	QRegExp wsOpt ("[ \t\r\n]*");
	
	lexer.addRule ("Name", QRegExp ("\\$?[A-Za-z]+"), TokenName);
	lexer.addRule ("Id", QRegExp ("-?[0-9]+"), TokenId);
	lexer.addRule ("Rule", ":", TokenRule);
	lexer.addRule ("Definition", "=", TokenDefinition);
	lexer.addRule ("String", QRegExp ("\"(?:\\\\.|[^\\\\\"])*\""), TokenString);
	lexer.addRule ("RegExp", QRegExp ("/(?:\\\\.|[^\\\\/])*/i?"), TokenRegExp);
	lexer.addRule ("DefEnd", ";", TokenDefEnd);
	
	// Empty line
	lexer.addDefinition ("Line", QVariantList() << QRegExp ("[ \t]*"));
	
	// Comment
	lexer.addDefinition ("Line", QVariantList() << QRegExp ("#[^\r\n]*"));
	
	// RHS for rules
	lexer.addDefinition ("RuleDef", QVariantList() << QVariant::fromValue (LexerRule("RegExp")));
	lexer.addDefinition ("RuleDef", QVariantList() << QVariant::fromValue (LexerRule("String")));
	
	// Rule
	lexer.addDefinition ("Line", QVariantList() << QVariant::fromValue (LexerRule("Name"))
			     << wsOpt << QString("(") << QVariant::fromValue (LexerRule("Id")) << QString(")")
			     << wsOpt << QVariant::fromValue (LexerRule("Rule"))
			     << wsOpt << QVariant::fromValue (LexerDefinition("RuleDef")));
	
	// Definition
	lexer.addDefinition ("Item", QVariantList() << QVariant::fromValue (LexerRule ("Name")));
	lexer.addDefinition ("Item", QVariantList() << QVariant::fromValue (LexerRule ("String")));
	lexer.addDefinition ("Item", QVariantList() << QVariant::fromValue (LexerRule ("RegExp")));
	
	lexer.addDefinition ("Part", QVariantList() << QVariant::fromValue (LexerDefinition ("Item")));
	lexer.addDefinition ("Part", QVariantList() << QVariant::fromValue (LexerDefinition ("Item"))
			     << ws << QVariant::fromValue (LexerDefinition ("Part")));
	
	lexer.addDefinition ("Line", QVariantList() << QVariant::fromValue (LexerRule("Name"))
			     << wsOpt << QVariant::fromValue (LexerRule("Definition"))
			     << wsOpt << QVariant::fromValue (LexerDefinition ("Part"))
			     << wsOpt << QVariant::fromValue (LexerRule("DefEnd")));
	
	// 
	lexer.addDefinition (QString(), QVariantList() << QVariant::fromValue (LexerDefinition ("Line")));
	lexer.addDefinition (QString(), QVariantList() << QVariant::fromValue (LexerDefinition ("Line"))
			     << QRegExp("(?:\r\n|\r|\n)") // Matches a newline
			     << QVariant::fromValue (LexerDefinition (QString())));
	
	// Tokenize definition
	if (!lexer.lex (definition)) {
		// Failed. Return a invalid MiniLexer instance.
		error = tr("Failed to read definition");
		return new MiniLexer;
	}
	
	// Okay, see what we got here...
	MiniLexer *result = new MiniLexer;
	
	int i;
	QString name; // Name of the rule or definition
	
	// Note: We don't do a lot of checks here, as the definitions from above
	// ensure that we only get valid formatted data. It'd be a waste of resources
	// to do it once again.
	for (i = 0; i < lexer.length (); i++) {
		
		// Name of a rule or definition
		if (lexer.token (i) == TokenName) {
			name = lexer.value (i);
			continue;
		}
		
		// Name must not be empty
		if (name.isEmpty ()) {
			error = tr("Name of definitions and rules must not be empty");
			break;
		}
		
		if (lexer.token (i) == TokenId) {
			// It's a rule.
			int tokenId = lexer.value (i).toInt ();
			
			// Rule names may start with a dollar sign ('$')
			// If it has one, remove it.
			if (name.startsWith (QLatin1Char ('$')))
				name = name.mid (1);
			
			i += 2; // Skip to the body
			
			// We expect a string or a regexp here.
			if (lexer.token (i) != TokenString && lexer.token (i) != TokenRegExp) {
				error = tr("Expected a regular expression or a string as rule body");
				break;
			}
			
			// Add rule
			QVariant ruleContent = stringToVariant (lexer.value (i));
			if (ruleContent.canConvert< QString > ())
				result->addRule (name, ruleContent.toString (), tokenId);
			else
				result->addRule (name, ruleContent.toRegExp (), tokenId);
			
			name.clear ();
			continue;
		} else if (lexer.token (i) == TokenDefinition) {
			// It's a definition.
			i++;
			
			// The name must not start with a $
			if (name.startsWith (QLatin1Char ('$'))) {
				error = tr("Definition name must not start with a dollar-sign ('$')");
				break;
			}
			
			// If name == "START", it refers to the start definition
			if (name == QLatin1String ("START"))
				name.clear ();
			
			// Create list
			QVariantList def;
			for (; i < lexer.length () && lexer.token (i) != TokenDefEnd; i++) {
				def.append (stringToVariant (lexer.value (i)));
			}
			
			// Add definition
			result->addDefinition (name, def);
			name.clear ();
			continue;
		}
		
		// Unexpected token
		error = tr("Unexpected token");
		break;
		
	}
	
	// In case of error return a invalid instance.
	if (i != lexer.length ()) {
		delete result;
		return new MiniLexer;
	}
	
	return result;
}

void Nuria::MiniLexer::chopValueList (int lastValidLength) {
	
	if (this->d_ptr->values.length () <= lastValidLength)
		return;
	
	// Remove all entries after lastValidLength in m_values and m_valueTokens
	QList< QString >::Iterator fromValues = this->d_ptr->values.begin () + lastValidLength;
	QList< int >::Iterator fromTokens = this->d_ptr->valueTokens.begin () + lastValidLength;
	
	this->d_ptr->values.erase (fromValues, this->d_ptr->values.end ());
	this->d_ptr->valueTokens.erase (fromTokens, this->d_ptr->valueTokens.end ());
	
}

bool Nuria::MiniLexer::lexDefinition (const QVariantList &def, const QString &data, int &pos) {
	int valuePos = this->d_ptr->values.length (); // For chopValueList.
	
	for (int i = 0; i < def.length (); i++) {
		const QVariant &cur = def.at (i);
		
		if (cur.canConvert< QString > ()) {
			// String. Use a QStringRef so we don't waste time on copying
			// some bytes from data.
			
			QString value = cur.toString ();
			QStringRef ref = data.midRef (pos, value.length ());
			
			// Fail if no match
			if (ref.compare (value, this->d_ptr->sensitivity)) {
				chopValueList (valuePos);
				this->d_ptr->errorPos = pos;
				return false;
			}
			
			pos += value.length ();
			
		} else if (cur.canConvert< QRegExp > ()) {
			
			QRegExp rx = cur.toRegExp ();
			
			// Fail if no match. Only matches if the regex matches at pos.
			if (rx.indexIn (data, pos) != pos) {
				chopValueList (valuePos);
				this->d_ptr->errorPos = pos;
				return false;
			}
			
			pos += rx.matchedLength ();
			
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
			
		} else if (cur.canConvert< QRegularExpression > ()) {
			QRegularExpression rx = cur.toRegularExpression ();
			
			// Match ...
			QRegularExpressionMatch match = rx.match (data, pos, QRegularExpression::NormalMatch,
								  QRegularExpression::AnchoredMatchOption);
			
			// Only matches if the regex matches at pos.
			if (match.hasMatch ()) {
				chopValueList (valuePos);
				this->d_ptr->errorPos = pos;
				return false;
			}
			
			pos += match.capturedLength ();
			
#endif
			
		} else if (cur.userType () == qMetaTypeId< Nuria::LexerRule > ()) {
			
			const QString &name = cur.value< Nuria::LexerRule > ().name ();
			Rule rule = this->d_ptr->rules.value (name);
			
			// Is the rule a regular expression or just a string?
			if (rule.data.userType () == QMetaType::QRegExp) {
				QRegExp rx = rule.data.toRegExp ();
				
				// Rule not found?
				if (!rx.isValid ()) {
					chopValueList (valuePos);
					return false;
				}
				
				// Match. Return on failure.
				int mPos = rx.indexIn (data, pos);
				if (mPos != pos) {
					chopValueList (valuePos);
					this->d_ptr->errorPos = pos;
					return false;
				}
				
				// Copy matched length
				if (rule.token >= 0) {
					// Use cap(1) if it is available.
					// Else, use the matched text.
					if (rx.captureCount () > 0) {
						this->d_ptr->values.append (rx.cap (1));
					} else {
						this->d_ptr->values.append (rx.cap (0));
					}
					
					this->d_ptr->valueTokens.append (rule.token);
				}
				
				pos += rx.matchedLength ();
				
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
			} else if (rule.data.userType () == QMetaType::QRegularExpression) {
				QRegularExpression rx = cur.toRegularExpression ();
				
				// Sanity check.
				if (!rx.isValid ()) {
					chopValueList (valuePos);
					return false;
				}
				
				// Match ...
				QRegularExpressionMatch match = rx.match (data, pos, QRegularExpression::NormalMatch,
									  QRegularExpression::AnchoredMatchOption);
				
				// Only matches if the regex matches at pos.
				if (match.hasMatch ()) {
					chopValueList (valuePos);
					this->d_ptr->errorPos = pos;
					return false;
				}
				
				// Copy matching part
				if (rule.token >= 0) {
					// Use captured(1) if it is available.
					// Else, use the matched text.
					if (match.capturedStart (1) != -1) {
						this->d_ptr->values.append (match.captured (1));
					} else {
						this->d_ptr->values.append (match.captured (0));
					}
					
					this->d_ptr->valueTokens.append (rule.token);
				}
				
				// 
				pos += match.capturedLength ();
				
#endif
				
			} else {
				QString string = rule.data.toString ();
				
				// See if it matches
				QStringRef ref = data.midRef (pos, string.length ());
				
				// Fail if no match
				if (ref.compare (string, this->d_ptr->sensitivity)) {
					chopValueList (valuePos);
					this->d_ptr->errorPos = pos;
					return false;
				}
				
				// Copy matched part
				if (rule.token >= 0) {
					// We use the reference here as we want to retain the
					// exact upper and lower case of the string characters.
					this->d_ptr->values.append (ref.toString ());
					this->d_ptr->valueTokens.append (rule.token);
				}
				
				pos += string.length ();
				
			}
				
		} else if (cur.userType () == qMetaTypeId< Nuria::LexerDefinition > ()) {
			
			// Child definition.
			const QString &name = cur.value< Nuria::LexerDefinition > ().name ();
			QList< QVariantList > list = this->d_ptr->defs.value (name);
			
			int j;
			for (j = 0; j < list.length (); j++) {
				
				int oldPos = pos;
				if (lexDefinition (list.at (j), data, pos))
					break;
				
				pos = oldPos;
				
			}
			
			if (j >= list.length ()) {
				// Nothing matched. Return.
				chopValueList (valuePos);
				this->d_ptr->errorPos = pos;
				return false;
			}
			
		}
		
	}
	
	// Everything matched correctly. Return.
	return true;
	
}
