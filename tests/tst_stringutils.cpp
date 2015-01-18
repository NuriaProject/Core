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

#include <nuria/stringutils.hpp>

#include <QtTest/QtTest>

using namespace Nuria;

Q_DECLARE_METATYPE(Nuria::StringUtils::CheckState);

class StringUtilsTest : public QObject {
	Q_OBJECT
private slots:
	
	void checkValidUtf8_data ();
	void checkValidUtf8 ();
	void checkValidUtf8Benchmark_data ();
	void checkValidUtf8Benchmark ();
	
};

void StringUtilsTest::checkValidUtf8_data () {
	QTest::addColumn< QByteArray > ("data");
	QTest::addColumn< StringUtils::CheckState > ("state");
	QTest::addColumn< int > ("pos");
	
	StringUtils::CheckState valid = StringUtils::Valid;
	StringUtils::CheckState incomplete = StringUtils::Incomplete;
	StringUtils::CheckState failed = StringUtils::Failed;
	
	// 
	QTest::newRow ("valid-ascii") << QByteArray (" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]"
	                                             "^_`abcdefghijklmnopqrstuvwxyz{|}~") << valid << 0;
	QTest::newRow ("valid-2") << QByteArray ("foo\xC3\xA4") << valid << 0;
	QTest::newRow ("valid-3") << QByteArray ("bar\xE2\x82\xAC") << valid << 0;
	QTest::newRow ("valid-4") << QByteArray ("baz\xF0\x9D\x84\x9E") << valid << 0;
	QTest::newRow ("valid-mixed") << QByteArray ("\xC3\xA4\xE2\x82\xAC\xF0\x9D\x84\x9E\xE2\x82\xAC\xC3\xA4")
	                              << valid << 0;
	QTest::newRow ("valid-4-max") << QByteArray ("nuria\xF4\x8F\xBF\xBF") << valid << 0; // U+10FFFF
	
	QTest::newRow ("fail-2-length") << QByteArray ("foo\xC3zzz") << failed << 3;
	QTest::newRow ("fail-3-length") << QByteArray ("bar\xE2\x82zzz") << failed << 3;
        QTest::newRow ("fail-4-length") << QByteArray ("baz\xF0\x9D\x84zzz") << failed << 3;
	
	QTest::newRow ("incomplete-2-length") << QByteArray ("foo\xC3") << incomplete << 3;
	QTest::newRow ("incomplete-3-length") << QByteArray ("bar\xE2\x82") << incomplete << 3;
	QTest::newRow ("incomplete-4-length") << QByteArray ("baz\xF0\x9D\x84") << incomplete << 3;
	
	QTest::newRow ("fail-1-seq") << QByteArray ("yadda\x80") << failed << 5;
	QTest::newRow ("fail-5-seq") << QByteArray ("yadda\xFB") << failed << 5;
	
	QTest::newRow ("inflated-2") << QByteArray ("foo\xC0\xC0") << failed << 3;
	QTest::newRow ("inflated-3") << QByteArray ("bar\xE0\x82\xA2") << failed << 3;
	QTest::newRow ("inflated-4") << QByteArray ("baz\xF0\x02\x02\xAC") << failed << 3;
	
	QTest::newRow ("fail-4-range") << QByteArray ("nuria\xF4\x90\x80\x80") << failed << 5; // U+10FFFF + 1
	
	QTest::newRow ("surrogates-lower") << QByteArray ("\xED\xA0\x80") << failed << 0; // U+D800
	QTest::newRow ("surrogates-upper") << QByteArray ("\xED\xBF\xBF") << failed << 0; // U+DFFF
	
	QTest::newRow ("valid-min") << QByteArray ("\xC2\x80") << valid << 0; // U+0080
	QTest::newRow ("valid-utf-8") << QByteArray ("Hello-µ@ßöäüàá-UTF-8!!") << valid << 0;
}

void StringUtilsTest::checkValidUtf8 () {
	QFETCH(Nuria::StringUtils::CheckState, state);
	QFETCH(QByteArray, data);
	QFETCH(int, pos);
	
	int foundPos = 0;
	QCOMPARE(StringUtils::checkValidUtf8 (data.constData (), data.length (), foundPos), state);
	QCOMPARE(foundPos, pos);
}

void StringUtilsTest::checkValidUtf8Benchmark_data () {
	QTest::addColumn< QByteArray > ("data");
	
	// About 16MiB each
	QTest::newRow ("ascii") << QByteArray (16 * 1024 * 1024, '*');
	QTest::newRow ("utf-8") << QByteArray ("0123456789ABCDEF\xF0\x9D\x84\x9E").repeated (838861);
	QTest::newRow ("utf-8-only") << QByteArray ("\xC3\xA4\xE2\x82\xAC\xF0\x9D\x84\x9E").repeated (1864135);
}

void StringUtilsTest::checkValidUtf8Benchmark () {
	QFETCH(QByteArray, data);
	
	const char *dataPtr = data.constData ();
	int dataLen = data.length ();
	int pos = 0;
	
	QBENCHMARK {
		StringUtils::checkValidUtf8 (dataPtr, dataLen, pos);
	}
	
}

QTEST_MAIN(StringUtilsTest)
#include "tst_stringutils.moc"
