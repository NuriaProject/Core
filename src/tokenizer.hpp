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

#ifndef NURIA_TOKENIZER_HPP
#define NURIA_TOKENIZER_HPP

#include "essentials.hpp"
#include <QSharedData>
#include <functional>
#include <QVariant>
#include <QObject>
#include <regex>

namespace Nuria {

class TokenizerRulesPrivate;
class TokenizerPrivate;
class Tokenizer;

/**
 * \brief Token as returned by Nuria::Tokenizer
 * 
 * This plain structure is used by Nuria::Tokenizer to store token data,
 * such as location, the token id and the value.
 * 
 * The structure is flat, meaning you directly access the fields. This was
 * chosen due to the runtime overhead of shared structures to avoid performance
 * issues while tokenizing and subsequent parsing applications.
 * 
 * Both the row and column begin counting from zero.
 * 
 * \note You can also output this structure on QDebug and Nuria::Debug streams.
 */
struct NURIA_CORE_EXPORT Token {
	Token (int tokenId = -1, int row = 0, int column = 0,
	       const QVariant &value = QVariant ())
	        : row (row), column (column), tokenId (tokenId), value (value)
	{ }
	
	/** The token id */
	int tokenId;
	
	/** Row */
	int row;
	
	/** Column */
	int column;
	
	/** The value of this token */
	QVariant value;
	
	/** Returns \c true if this instance comes before \a right. */
	bool operator< (const Token &right) const;
	
};

/**
 * \brief Storage of rules used by Nuria::Tokenizer
 * 
 * A instance of this class define a rule-set for Nuria::Tokenizer, defining
 * how to read the data stream.
 * 
 * Rules are not directly stored in Nuria::Tokenizer to allow for complex
 * grammatics, demanding for multiple rule-sets. Please see Nuria::Tokenizer
 * for usage details.
 * 
 * \note This structure is \b implicitly shared.
 * 
 * \par Precedence
 * 
 * During matching, string tokens are tested first, meaning that string tokens
 * take precedence over regular-expression ones. Matches are tried in the order
 * they were added to the rule-set, meaning the first added rule will also first
 * tried.
 * 
 * \par Whitespace handling
 * 
 * TokenizerRules can automatically handle whitespace, meaning that it's
 * automatically skipped. This is the default setting.
 * 
 */
class NURIA_CORE_EXPORT TokenizerRules {
public:
	
	/** Whitespace handling modes. */
	enum WhitespaceMode {
		
		/** Automatically ignore whitespace. Default setting. */
		AutoHandleWhitespace = 0,
		
		/** Do not ignore whitespace. */
		ManualWhitespaceHandling
		
	};
	
	/** Typedef of a token action. See setTokenAction() for details. */
	typedef std::function< bool(Token &, Tokenizer *) > TokenAction;
	
	/** Constructs a empty rule-set. */
	TokenizerRules (WhitespaceMode mode = AutoHandleWhitespace);
	
	/** Copy constructor. */
	TokenizerRules (const TokenizerRules &other);
	
	/** Assignment operator. */
	TokenizerRules &operator= (const TokenizerRules &other);
	
	/** Destructor. */
	~TokenizerRules ();
	
	/**
	 * Adds a token matching exactly \a terminal.
	 */
	void addStringToken (int tokenId, const QByteArray &terminal);
	
	/**
	 * Adds a token matching \a regularExpression. This method is provided
	 * for convenience. It's equivalent to:
	 * \code
	 * addRegexToken (tokenId, QRegularExpression (regularExpression));
	 * \endcode
	 */
	void addRegexToken (int tokenId, const QByteArray &regularExpression);
	
	/**
	 * Adds a token matching the \a regularExpression.
	 */
	void addRegexToken (int tokenId, const std::regex &regularExpression);	
	
	/**
	 * Sets \a action as handler for all tokens of type \a tokenId.
	 * The handler will be called everytime a token of that type is
	 * encountered. The prototype of \a action is as follows:
	 * 
	 * \code
	 * bool handler (Token &token, Tokenizer *tokenizer);
	 * \endcode
	 * 
	 * The handler will be passed the token itself as mutable reference and
	 * the Tokenizer instance.
	 * On success, \c true must be returned by the handler. If the handler
	 * returns \c false, tokenizing will fail.
	 * 
	 * The handler may change the contents of the passed token. If the
	 * handler changes the tokenId of the token, the token action handler
	 * of that new tokenId will \b not be invoked.
	 */
	void setTokenAction (int tokenId, TokenAction action);
	
	/** Returns the current whitespace handling mode. */
	WhitespaceMode whitespaceMode () const;
	
	/** Sets the whitespace handling mode. */
	void setWhitespaceMode (WhitespaceMode mode);
	
private:
	friend class Tokenizer;
	QSharedDataPointer< TokenizerRulesPrivate > d;
};

/**
 * \brief General-purpose run-time tokenizer
 * 
 * This is a general-purpose tokenizer which can be constructed and configured
 * at run-time, allowing for complex token-schemes.
 * 
 * \par Usage
 * 
 * Basic usage consists of creating an instance of Nuria::Tokenizer, filling
 * the default rule-set with data (See defaultTokenizerRules() ) and then
 * using tokenize() and nextToken() to iterate over a data stream.
 * 
 * Please note that Nuria::Tokenizer uses std::regex for regular expressions
 * as QRegularExpression only works on QStrings. Although the syntax is pretty
 * similar, please have a look at the documentation of ECMAScript regular
 * expressions, e.g. here: http://www.cplusplus.com/reference/regex/ECMAScript/
 * 
 * \par Using multiple rule-sets
 * 
 * For some types of data it may be desirable to use multiple rule-sets.
 * Nuria::Tokenizer allows this use-case too.
 * 
 * To do this, you'll first need to create all needed rule-sets by creating
 * instances of TokenizerRules and filling them. 
 * 
 * Second, you'll need to use TokenizerRules::setTokenAction() to register
 * a token handler for all tokens which should switch the currently used
 * rule-set. You can do this by calling setCurrentTokenizerRules() on the
 * Nuria::Tokenizer instance passed to the handler.
 * 
 * After this, it's just a matter of adding all rule-sets using
 * addTokenizerRules().
 * 
 * \note The default rule-set has "" (empty string) as name.
 * 
 * \par Ignoring tokens
 * 
 * All tokens with negative token ids will be ignored and silently discarded.
 * If nextToken() encounters such a token, it'll read on until it found a
 * token which is not ignored or until the end of the data-stream.
 * 
 * To decide later if a token should be discarded, you can register a token
 * action handler on a specific token. If you want to ignore that token, you
 * can simply set the tokenId of the 'token' argument to a negative value.
 * 
 * \par Location and error handling
 * 
 * Nuria::Tokenizer automatically takes care of location and error tracking.
 * You can access the cursor position using currentRow(), currentColumn or
 * currentPosition() to get the current row, column, or position in the
 * data-stream respectively.
 * 
 * The same goes for the error position, which can be accessed using errorRow(),
 * errorColumn() and errorPosition().
 */
class NURIA_CORE_EXPORT Tokenizer : public QObject {
	Q_OBJECT
public:
	
	/** Constructor. */
	Tokenizer (QObject *parent = nullptr);
	
	/** Destructor. */
	~Tokenizer () override;
	
	/** Returns the default tokenizer rule-set. */
	TokenizerRules &defaultTokenizerRules ();
	
	/** Sets the default tokenizer \a ruleSet. */
	void setDefaultTokenizerRules (const TokenizerRules &ruleSet);
	
	/** Adds the named \a ruleSet as \a name. */
	void addTokenizerRules (const QString &name, const TokenizerRules &ruleSet);
	
	/** Returns the rule-set called \a name. */
	TokenizerRules tokenizerRules (const QString &name) const;
	
	/**
	 * Removes the rule-set called \a name.
	 * If \a name is empty the call will have no effect.
	 * If \a name is the currently used rule-set, the default rule-set
	 * will be the currently used one after the call.
	 */
	void removeTokenizerRules (const QString &name);
	
	/**
	 * Tells the tokenizer to use the rule-set known as \a name from now on.
	 * If \a name is not a known rule-set, the default rule-set is used.
	 */
	void setCurrentTokenizerRules (const QString &name);
	
	/** Returns the currently used tokenizer rule-set. */
	const TokenizerRules &currentTokenizerRules () const;
	
	/**
	 * Sets \a data to be tokenized. Use nextToken() to acquire the next
	 * token.
	 */
	void tokenize (const QByteArray &data);
	
	/**
	 * Returns the data as passed to the last call to tokenize(). 
	 */
	QByteArray tokenizeData () const;
	
	/**
	 * Moves the tokenizer onwards by one token, returning the most-recently
	 * read token. \sa atEnd hasError
	 */
	Token nextToken ();
	
	/**
	 * Returns \c true if the tokenizer reached the end of the data stream.
	 */
	bool atEnd () const;
	
	/** Returns \c true if the last call to nextToken() raised an error. */
	bool hasError () const;
	
	/** Returns the column where the error occured. */
	int errorColumn () const;
	
	/** Returns the row where the error occured. */
	int errorRow () const;
	
	/** Returns the position in the data-stream where the error occured. */
	int errorPosition () const;
	
	/** Returns the current column in the data-stream. */
	int currentColumn () const;
	
	/** Returns the current row in the data-stream. */
	int currentRow () const;
	
	/** Returns the current position in the data-stream. */
	int currentPosition () const;
	
private:
	
	void advanceLocation (char c);
	void skipWhitespace ();
	bool readTokens ();
	bool readAndHandleTokens ();
	bool checkStringToken ();
	bool checkStringToken (const QByteArray &token, int tok);
	bool checkRegexToken ();
	bool checkRegexToken (const std::regex &regex, int tok);
	
	TokenizerPrivate *d_ptr;
	
};

}

NURIA_CORE_EXPORT QDebug operator<< (QDebug debug, const Nuria::Token &token);

#endif // NURIA_TOKENIZER_HPP
