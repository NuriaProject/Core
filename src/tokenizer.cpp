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

#include "nuria/tokenizer.hpp"
#include <QVector>
#include <QDebug>
#include <regex>

namespace Nuria {
struct Location {
	int position = 0;
	int column = 0;
	int row = 0;
	
};

class TokenizerPrivate {
public:
	
	QMap< QString, TokenizerRules > rules;
	const TokenizerRulesPrivate *currentSet = nullptr;
	QString currentRuleName;
	
	QByteArray data;
	
	Token token;
	Location last;
	Location current;
	Location error;
	
};

class TokenizerRulesPrivate : public QSharedData {
public:
	TokenizerRules::WhitespaceMode mode;
	
	QVector< QPair< QByteArray, int > > stringTokens;
	QVector< QPair< std::regex, int > > rxTokens;
	QMap< int, TokenizerRules::TokenAction > actions;
	
};

}

Nuria::Tokenizer::Tokenizer (QObject *parent)
        : QObject (parent), d_ptr (new TokenizerPrivate)
{
	
	this->d_ptr->rules.insert (QString (), TokenizerRules ());
	setCurrentTokenizerRules (QString ());
	
}

Nuria::Tokenizer::~Tokenizer () {
	delete this->d_ptr;
}

Nuria::TokenizerRules &Nuria::Tokenizer::defaultTokenizerRules () {
	return this->d_ptr->rules[QString ()];
}

void Nuria::Tokenizer::setDefaultTokenizerRules (const TokenizerRules &ruleSet) {
	addTokenizerRules (QString (), ruleSet);
}

void Nuria::Tokenizer::addTokenizerRules (const QString &name, const TokenizerRules &ruleSet) {
	this->d_ptr->rules.insert (name, ruleSet);
	setCurrentTokenizerRules (this->d_ptr->currentRuleName);
}

Nuria::TokenizerRules Nuria::Tokenizer::tokenizerRules (const QString &name) const {
	return this->d_ptr->rules.value (name);
}

void Nuria::Tokenizer::removeTokenizerRules (const QString &name) {
	if (name.isEmpty ()) {
		return;
	}
	
	// 
	this->d_ptr->rules.remove (name);
	
	if (this->d_ptr->currentRuleName == name) {
		setCurrentTokenizerRules (QString ());
	}
	
}

void Nuria::Tokenizer::setCurrentTokenizerRules (const QString &name) {
	if (!this->d_ptr->rules.contains (name)) {
		return (void) setCurrentTokenizerRules (QString ());
	}
	
	// 
	this->d_ptr->currentRuleName = name;
	this->d_ptr->currentSet = this->d_ptr->rules.value (name).d.constData ();
}

const Nuria::TokenizerRules &Nuria::Tokenizer::currentTokenizerRules () const {
	auto it = this->d_ptr->rules.constFind (this->d_ptr->currentRuleName);
	return *it;
}

void Nuria::Tokenizer::tokenize (const QByteArray &data) {
	this->d_ptr->data = data;
	
	this->d_ptr->current = Location ();
	this->d_ptr->last = Location ();
	
	this->d_ptr->error.column = -1;
	this->d_ptr->error.row = -1;
	this->d_ptr->error.position = -1;
	
}

QByteArray Nuria::Tokenizer::tokenizeData () const {
	return this->d_ptr->data;
}

Nuria::Token Nuria::Tokenizer::nextToken () {
	
	// Whitespace handling
	if (this->d_ptr->currentSet->mode == TokenizerRules::AutoHandleWhitespace) {
		skipWhitespace ();
	}
	
	// End check
	if (atEnd ()) {
		return Token ();
	}
	
	// 
	if (readAndHandleTokens ()) {
		return this->d_ptr->token;
	}
	
	// Error
	this->d_ptr->error = this->d_ptr->last;
	return Token ();
	
}

bool Nuria::Tokenizer::atEnd () const {
	return (this->d_ptr->current.position >= this->d_ptr->data.length ());
}

bool Nuria::Tokenizer::hasError () const {
	return (this->d_ptr->error.position >= 0);
}

int Nuria::Tokenizer::errorColumn () const {
	return this->d_ptr->error.column;
}

int Nuria::Tokenizer::errorRow () const {
	return this->d_ptr->error.row;
}

int Nuria::Tokenizer::errorPosition () const {
	return this->d_ptr->error.position;
}

int Nuria::Tokenizer::currentColumn () const {
	return this->d_ptr->current.column;
}

int Nuria::Tokenizer::currentRow () const {
	return this->d_ptr->current.row;
}

int Nuria::Tokenizer::currentPosition () const {
	return this->d_ptr->current.position;
}

void Nuria::Tokenizer::setPosition (int position, int column, int row) {
	this->d_ptr->current.position = position;
	this->d_ptr->current.column = column;
	this->d_ptr->current.row = row;
}

void Nuria::Tokenizer::advanceLocation (char c) {
	this->d_ptr->current.column++;
	if (c == '\n') {
		this->d_ptr->current.row++;
		this->d_ptr->current.column = 0;
	}
	
}

void Nuria::Tokenizer::skipWhitespace () {
	int &pos = this->d_ptr->current.position;
	int len = this->d_ptr->data.length ();
	
	while (pos < len && isspace (this->d_ptr->data.at (pos))) {
		advanceLocation (this->d_ptr->data.at (pos));
		pos++;
	}
	
}

bool Nuria::Tokenizer::readTokens () {
	
	this->d_ptr->last = this->d_ptr->current;
	while (!atEnd () && (checkStringToken () || checkRegexToken ())) {
		if (this->d_ptr->token.tokenId >= 0) {
			return true;
		}
		
		// Whitespace handling
		if (this->d_ptr->currentSet->mode == TokenizerRules::AutoHandleWhitespace) {
			skipWhitespace ();
		}
		
		// 
		this->d_ptr->last = this->d_ptr->current;
	}
	
	return false;
}

bool Nuria::Tokenizer::readAndHandleTokens () {
	auto end = this->d_ptr->currentSet->actions.constEnd ();
	
	while (readTokens ()) {
		auto it = this->d_ptr->currentSet->actions.constFind (this->d_ptr->token.tokenId);
		
		// Token action handler found?
		if (it == end) {
			return true;
		}
		
		// Invoke handler
		if (!(*it) (this->d_ptr->token, this)) {
			return false;
		}
		
		// Accept token?
		if (this->d_ptr->token.tokenId >= 0) {
			return true;
		}
		
	}
	
	// Error
	return false;
}

bool Nuria::Tokenizer::checkStringToken () {
	const TokenizerRulesPrivate *p = this->d_ptr->currentSet;
	auto it = p->stringTokens.constBegin ();
	auto end = p->stringTokens.constEnd ();
	
	// Check for a match
	bool result = false;
	for (; it != end && !result; ++it) {
		result = checkStringToken (it->first, it->second);
	}
	
	// Done
	return result;
}

bool Nuria::Tokenizer::checkStringToken (const QByteArray &token, int tok) {
	const char *ptr = this->d_ptr->data.constData () + this->d_ptr->current.position;
	if (::memcmp (ptr, token.constData (), token.length ())) {
		return false;
	}
	
	// Match. Copy token
	this->d_ptr->token.column = this->d_ptr->current.column;
	this->d_ptr->token.row = this->d_ptr->current.row;
	this->d_ptr->token.tokenId = tok;
	this->d_ptr->token.value = token;
	
	// Update location
	this->d_ptr->current.position += token.length ();
	for (int i = 0; i < token.length (); i++) {
		advanceLocation (token.at (i));
	}
	
	// Done.
	return true;
	
}

bool Nuria::Tokenizer::checkRegexToken () {
	const TokenizerRulesPrivate *p = this->d_ptr->currentSet;
	auto it = p->rxTokens.constBegin ();
	auto end = p->rxTokens.constEnd ();
	
	bool result = false;
	for (; it != end && !result; ++it) {
		result = checkRegexToken (it->first, it->second);
	}
	
	// 
	return result;
}

bool Nuria::Tokenizer::checkRegexToken (const std::regex &regex, int tok) {
	const char *ptr = this->d_ptr->data.constData () + this->d_ptr->current.position;
	
	// Check
	std::cmatch matches;
	if (!std::regex_search (ptr, matches, regex, std::regex_constants::match_continuous)) {
		return false;
	}
	
	// Match found!
	const std::csub_match &m = matches[0];
	int len = m.length ();
	
	// Copy token
	this->d_ptr->token.column = this->d_ptr->current.column;
	this->d_ptr->token.row = this->d_ptr->current.row;
	this->d_ptr->token.tokenId = tok;
	this->d_ptr->token.value = QByteArray (ptr, len);
	
	// Advance cursor
	this->d_ptr->current.position += len;
	for (int i = 0; i < len; i++) {
		advanceLocation (ptr[len]);
	}
	
	// Done.
	return true;
}

Nuria::TokenizerRules::TokenizerRules (WhitespaceMode mode)
        : d (new TokenizerRulesPrivate)
{
	
	this->d->mode = mode;
	
}

Nuria::TokenizerRules::TokenizerRules (const Nuria::TokenizerRules &other)
        : d (other.d)
{
	
}

Nuria::TokenizerRules &Nuria::TokenizerRules::operator= (const Nuria::TokenizerRules &other) {
	this->d = other.d;
	return *this;
}

Nuria::TokenizerRules::~TokenizerRules () {
	// 
}

void Nuria::TokenizerRules::addStringToken (int tokenId, const QByteArray &terminal) {
	this->d->stringTokens.append (qMakePair (terminal, tokenId));
}

void Nuria::TokenizerRules::addRegexToken (int tokenId, const QByteArray &regularExpression) {
	return addRegexToken (tokenId, std::regex (regularExpression.constData ()));
}

void Nuria::TokenizerRules::addRegexToken (int tokenId, const std::regex &regularExpression) {
	this->d->rxTokens.append (qMakePair (regularExpression, tokenId));
}

void Nuria::TokenizerRules::setTokenAction (int tokenId, Nuria::TokenizerRules::TokenAction action) {
	this->d->actions.insert (tokenId, action);
}

Nuria::TokenizerRules::WhitespaceMode Nuria::TokenizerRules::whitespaceMode () const {
	return this->d->mode;
}

void Nuria::TokenizerRules::setWhitespaceMode (WhitespaceMode mode) {
	this->d->mode = mode;
}

bool Nuria::Token::operator< (const Token &right) const {
	if (this->row <= right.row && this->column < right.column) {
		return true;
	}
	
	return (this->row < right.row);
}

QDebug operator<< (QDebug debug, const Nuria::Token &token) {
	debug.nospace () << "Token(" << token.tokenId << " ";
	
	if (!token.value.isValid ()) {
		debug << "<no value> ";
	} else {
		debug << token.value.toString ();
	}
	
	debug << " [" << token.row << "|" << token.column << "])";
	return debug.space ();
}
