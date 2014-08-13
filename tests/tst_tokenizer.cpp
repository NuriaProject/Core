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

#include <nuria/tokenizer.hpp>
#include <nuria/debug.hpp>
#include <QtTest/QTest>

using namespace Nuria;

// Helper macros
#define CHECK_TOKEN(Tokenizer, Type, Row, Column) \
{ \
	Token tok = Tokenizer.nextToken (); \
	if (tok.tokenId != Type || tok.row != Row || tok.column != Column) { \
	qWarning() << "Result  :" << tok; \
	qWarning() << "Expected:" << Token (Type, Row, Column); \
	QFAIL("The returned token did not match the expected one."); \
	} \
	}

#define CHECK_TOKEN_VALUE(Tokenizer, Type, Row, Column, Value) \
{ \
	Token tok = Tokenizer.nextToken (); \
	if (tok.tokenId != Type || tok.row != Row || tok.column != Column || tok.value != Value) { \
	qWarning() << "Result  :" << tok; \
	qWarning() << "Expected:" << Token (Type, Row, Column, Value); \
	QFAIL("The returned token did not match the expected one."); \
	} \
	}

// 
class TokenizerTest : public QObject {
	Q_OBJECT
private slots:
	
	void error ();
	void stringTokens ();
	void regexTokens ();
	void mixedTokens ();
	void manualWhitespaceHandling ();
	void ignoredStringToken ();
	void ignoredRegexToken ();
	void tokenHandler ();
	void tokenHandlerIgnoresToken ();
	void tokenHandlerErrors ();
	void multipleRuleSets ();
	
};

void TokenizerTest::error () {
	Tokenizer tokenizer;
	
	// Whitespace auto handling is active
	tokenizer.tokenize (" a");
	tokenizer.nextToken ();
	
	QVERIFY(tokenizer.hasError ());
	QCOMPARE(tokenizer.errorColumn (), 1);
	QCOMPARE(tokenizer.errorRow (), 0);
	QCOMPARE(tokenizer.errorPosition (), 1);
	
}

void TokenizerTest::stringTokens () {
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.addStringToken ('a', "a");
	rules.addStringToken ('b', "b");
	
	tokenizer.tokenize ("ab\n"
	                    "a b");
	
	CHECK_TOKEN_VALUE(tokenizer, 'a', 0, 0, "a");
	CHECK_TOKEN_VALUE(tokenizer, 'b', 0, 1, "b");
	CHECK_TOKEN_VALUE(tokenizer, 'a', 1, 0, "a");
	CHECK_TOKEN_VALUE(tokenizer, 'b', 1, 2, "b");
	QVERIFY(tokenizer.atEnd ());
}

void TokenizerTest::regexTokens () {
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.addRegexToken (1, "[0-9]");
	rules.addRegexToken (2, "[a-z]");
	
	tokenizer.tokenize ("12 ab");
	
	CHECK_TOKEN_VALUE(tokenizer, 1, 0, 0, "1");
	CHECK_TOKEN_VALUE(tokenizer, 1, 0, 1, "2");
	CHECK_TOKEN_VALUE(tokenizer, 2, 0, 3, "a");
	CHECK_TOKEN_VALUE(tokenizer, 2, 0, 4, "b");
	QVERIFY(tokenizer.atEnd ());
}

void TokenizerTest::mixedTokens () {
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.addStringToken (1, "b");
	rules.addRegexToken (2, "[a-z]");
	
	tokenizer.tokenize ("ab");
	
	CHECK_TOKEN_VALUE(tokenizer, 2, 0, 0, "a");
	CHECK_TOKEN_VALUE(tokenizer, 1, 0, 1, "b");
	QVERIFY(tokenizer.atEnd ());
}

void TokenizerTest::manualWhitespaceHandling () {
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.setWhitespaceMode (TokenizerRules::ManualWhitespaceHandling);
	rules.addStringToken (1, " ");
	rules.addRegexToken (2, "[a-z]");
	
	tokenizer.tokenize ("a b");
	
	CHECK_TOKEN_VALUE(tokenizer, 2, 0, 0, "a");
	CHECK_TOKEN_VALUE(tokenizer, 1, 0, 1, " ");
	CHECK_TOKEN_VALUE(tokenizer, 2, 0, 2, "b");
	QVERIFY(tokenizer.atEnd ());
}

void TokenizerTest::ignoredStringToken () {
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.addStringToken (-1, "a");
	rules.addRegexToken (1, "[a-z]");
	
	tokenizer.tokenize ("a b");
	
	CHECK_TOKEN_VALUE(tokenizer, 1, 0, 2, "b");
	QVERIFY(tokenizer.atEnd ());
}

void TokenizerTest::ignoredRegexToken () {
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.addStringToken (1, "a");
	rules.addRegexToken (-1, "[a-z]");
	
	tokenizer.tokenize ("b a");
	
	CHECK_TOKEN_VALUE(tokenizer, 1, 0, 2, "a");
	QVERIFY(tokenizer.atEnd ());
}

void TokenizerTest::tokenHandler () {
	bool invoked = false;
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.addStringToken (1, "a");
	
	tokenizer.tokenize ("\n a");
	
	rules.setTokenAction (1, [&](Token &tok, Tokenizer *inst) {
		if (tok.tokenId != 1) qCritical() << "token id was:" << tok.tokenId;
		else if (inst != &tokenizer) qCritical () << "Passed tokenizer did not match";
		else if (tok.row != 1) qCritical() << "Bad row";
		else if (tok.column != 1) qCritical() << "Bad column";
		else if (tok.value != "a") qCritical() << "Bad value";
		else invoked = true;
		
		tok.tokenId = 2;
		return true;
	});
	
	CHECK_TOKEN_VALUE(tokenizer, 2, 1, 1, "a");
	QVERIFY(invoked);
	QVERIFY(tokenizer.atEnd ());
}

void TokenizerTest::tokenHandlerIgnoresToken () {
	bool invoked = false;
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.addStringToken (1, "a");
	rules.addStringToken (2, "b");
	
	tokenizer.tokenize ("\n ab");
	
	rules.setTokenAction (1, [&](Token &tok, Tokenizer *inst) {
		if (tok.tokenId != 1) qCritical() << "token id was:" << tok.tokenId;
		else if (inst != &tokenizer) qCritical () << "Passed tokenizer did not match";
		else if (tok.row != 1) qCritical() << "Bad row";
		else if (tok.column != 1) qCritical() << "Bad column";
		else if (tok.value != "a") qCritical() << "Bad value";
		else invoked = true;
		
		tok.tokenId = -1;
		return true;
	});
	
	CHECK_TOKEN_VALUE(tokenizer, 2, 1, 2, "b");
	QVERIFY(invoked);
	QVERIFY(tokenizer.atEnd ());
}

void TokenizerTest::tokenHandlerErrors () {
	bool invoked = false;
	Tokenizer tokenizer;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	rules.addStringToken (1, "a");
	
	tokenizer.tokenize ("\n a");
	
	rules.setTokenAction (1, [&](Token &, Tokenizer *) {
		invoked = true;
		return false;
	});
	
	// 
	Token tok = tokenizer.nextToken ();
	QCOMPARE(tok.tokenId, -1);
	
	QVERIFY(tokenizer.atEnd ());
	QVERIFY(tokenizer.hasError ());
	QCOMPARE(tokenizer.errorColumn (), 1);
	QCOMPARE(tokenizer.errorRow (), 1);
	QCOMPARE(tokenizer.errorPosition (), 2);
	QVERIFY(invoked);
}

void TokenizerTest::multipleRuleSets () {
	Tokenizer tokenizer;
	
	int aCount = 0;
	int bCount = 0;
	
	TokenizerRules &rules = tokenizer.defaultTokenizerRules ();
	TokenizerRules second;
	
	rules.addStringToken (1, "a");
	second.addStringToken (2, "b");
	
	rules.setTokenAction (1, [&](Token &, Tokenizer *t) {
		t->setCurrentTokenizerRules ("second");
		aCount++;
		return true;
	});
	
	second.setTokenAction (2, [&](Token &, Tokenizer *t) {
		t->setCurrentTokenizerRules (QString ());
		bCount++;
		return true;
	});
	
	tokenizer.addTokenizerRules ("second", second);
	
	// 
	tokenizer.tokenize ("aba");
	
	CHECK_TOKEN_VALUE(tokenizer, 1, 0, 0, "a");
	CHECK_TOKEN_VALUE(tokenizer, 2, 0, 1, "b");
	CHECK_TOKEN_VALUE(tokenizer, 1, 0, 2, "a");
	QVERIFY(tokenizer.atEnd ());
	QVERIFY(!tokenizer.hasError ());
	QCOMPARE(aCount, 2);
	QCOMPARE(bCount, 1);
	
}

QTEST_MAIN(TokenizerTest)
#include "tst_tokenizer.moc"
