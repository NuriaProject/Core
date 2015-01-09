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

#include <QtTest/QtTest>

#include <nuria/lazyconditionwalker.hpp>
#include <nuria/lazyevaluation.hpp>

class LazyConditionWalkerTest : public QObject {
	Q_OBJECT
private slots:
	
	void testLazyConditionHandlers ();
	void testFieldHandlers ();
	void testVariantHandlers ();
	
	void ensureDepthFirstOrder ();
	
	void doNotWalkArguments ();
	void doWalkArguments ();
	
	void manipulateCondition ();
	void manipulateField ();
	void manipulateValue ();
	void manipulateArgument ();
};

void LazyConditionWalkerTest::testLazyConditionHandlers () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (val (1) == val (2) && val (3) != val (4) &&
				 val (1) > val (2) && val (3) >= val (4) &&
				 val (1) < val (2) || val (3) <= val (4));
	
	QVector< int > result;
	QVector< int > expected {
		LazyCondition::Equal, LazyCondition::NonEqual, LazyCondition::LogicAnd, 
		LazyCondition::Greater, LazyCondition::LogicAnd,
		LazyCondition::GreaterEqual, LazyCondition::LogicAnd, LazyCondition::Less,
		LazyCondition::LogicAnd, LazyCondition::LessEqual, LazyCondition::LogicOr
	};
	
	Callback testFunc = Callback::fromLambda([&](QVariantList, QVariant c) {
			    result.append (c.value< LazyCondition > ().type ());
	});
	
	// 
	LazyConditionWalker walker;
	walker.onCondition (LazyCondition::Empty, testFunc);
	walker.onCondition (LazyCondition::Single, testFunc);
	walker.onCondition (LazyCondition::Equal, testFunc);
	walker.onCondition (LazyCondition::NonEqual, testFunc);
	walker.onCondition (LazyCondition::Greater, testFunc);
	walker.onCondition (LazyCondition::GreaterEqual, testFunc);
	walker.onCondition (LazyCondition::Less, testFunc);
	walker.onCondition (LazyCondition::LessEqual, testFunc);
	walker.onCondition (LazyCondition::LogicAnd, testFunc);
	walker.onCondition (LazyCondition::LogicOr, testFunc);
	
	// 
	walker.walk (condition);
	QCOMPARE(result, expected);
	
}

void LazyConditionWalkerTest::testFieldHandlers () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (val (1) == arg (0) && test ("foo") == Field (51, QVariant ()));
	
	QVector< int > result;
	QVector< int > expected {
		Field::Argument, Field::TestCall, 51 /* Custom type */
	};
	
	
	Callback testFunc = Callback::fromLambda([&](QVariantList, Field f) {
			    result.append (f.customType ());
	});
	
	// 
	LazyConditionWalker walker;
	walker.onField (Field::Value, testFunc);
	walker.onField (Field::Argument, testFunc);
	walker.onField (Field::TestCall, testFunc);
	walker.onField (51, testFunc);
	
	// 
	walker.walk (condition, false);
	
	QCOMPARE(result, expected);
	
}

void LazyConditionWalkerTest::testVariantHandlers () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (arg (0) == val(5) && (val(QString()) == val(true)));
	
	QVector< int > result;
	QVector< int > expected {
		QMetaType::Int, QMetaType::QString, QMetaType::Bool
	};
	
	
	Callback testFunc = Callback::fromLambda([&](QVariantList, QVariant v) {
			    result.append (v.userType ());
	});
	
	// 
	LazyConditionWalker walker;
	walker.onVariant (qMetaTypeId< LazyCondition > (), testFunc); // Doesn't trigger
	walker.onVariant (qMetaTypeId< Field > (), testFunc); // Doesn't trigger
	walker.onVariant (QMetaType::Int, testFunc);
	walker.onVariant (QMetaType::QString, testFunc);
	walker.onVariant (QMetaType::Bool, testFunc);
	
	// 
	walker.walk (condition, false);
	QCOMPARE(result, expected);
	
}

void LazyConditionWalkerTest::ensureDepthFirstOrder () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (test ("foo", 5));
	
	QVector< int > result;
	QVector< int > expected {
		QMetaType::Int, qMetaTypeId< Field > (), qMetaTypeId< LazyCondition > ()
	};
	
	// 
	Callback testFunc = Callback::fromLambda([&](QVariantList, QVariant v) {
			    result.append (v.userType ());
	});
	
	// 
	LazyConditionWalker walker;
	walker.onVariant (QMetaType::Int, testFunc);
	walker.onField (Field::TestCall, testFunc);
	walker.onCondition (LazyCondition::Single, testFunc);
	
	// 
	walker.walk (condition, true);
	QCOMPARE(result, expected);
	
}


void LazyConditionWalkerTest::doNotWalkArguments () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (test ("foo", 5));
	
	// 
	int intSeen = 0;
	Callback testFunc = Callback::fromLambda([&](QVariantList, QVariant v) {
			    intSeen++;
	});
	
	// 
	LazyConditionWalker walker;
	walker.onVariant (QMetaType::Int, testFunc);
	
	// 
	walker.walk (condition, false);
	QCOMPARE(intSeen, 0);
}

void LazyConditionWalkerTest::doWalkArguments () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (test ("foo", 5));
	
	// 
	int intSeen = 0;
	Callback testFunc = Callback::fromLambda([&](QVariantList, QVariant v) {
			    intSeen++;
	});
	
	// 
	LazyConditionWalker walker;
	walker.onVariant (QMetaType::Int, testFunc);
	
	// 
	walker.walk (condition, true);
	QCOMPARE(intSeen, 1);
}

void LazyConditionWalkerTest::manipulateCondition () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (val (1) == val (2) && val (3) < val (4));
	
	Callback testFunc = Callback::fromLambda([&](QVariantList, LazyCondition c) {
			    return LazyCondition (c.left (), LazyCondition::NonEqual, c.right ());
	});
	
	// 
	LazyConditionWalker walker;
	walker.onCondition (LazyCondition::Equal, testFunc);
	
	LazyCondition result = walker.walk (condition);
	LazyCondition left = result.left ().value< LazyCondition > ();
	LazyCondition right = result.right ().value< LazyCondition > ();
	
	QCOMPARE(result.type (), LazyCondition::LogicAnd);
	QCOMPARE(left.type (), LazyCondition::NonEqual);
	QCOMPARE(right.type (), LazyCondition::Less);
	QCOMPARE(left.left (), QVariant (1));
	QCOMPARE(left.right (), QVariant (2));
	QCOMPARE(right.left (), QVariant (3));
	QCOMPARE(right.right (), QVariant (4));
	
}

void LazyConditionWalkerTest::manipulateField () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (val (1) == arg (0) && test ("foo"));
	
	Callback argToValue = Callback::fromLambda([]() {
		return QVariant (1);
	});
	
	Callback testToArgument = Callback::fromLambda([]() {
		return arg (1);
	});
	
	// 
	LazyConditionWalker walker;
	walker.onField (Field::Argument, argToValue);
	walker.onField (Field::TestCall, testToArgument);
	
	// 
	LazyCondition result = walker.walk (condition);
	LazyCondition left = result.left ().value< LazyCondition > ();
	LazyCondition right = result.right ().value< LazyCondition > ();
	Field rightField = right.left ().value< Field > ();
	
	QCOMPARE(result.type (), LazyCondition::LogicAnd);
	QCOMPARE(left.type (), LazyCondition::Equal);
	QCOMPARE(result.right ().userType (), qMetaTypeId< LazyCondition > ());
	QCOMPARE(rightField.type (), Field::Argument);
	QCOMPARE(left.left (), QVariant (1));
	QCOMPARE(left.right (), QVariant (1));
	QCOMPARE(rightField.value (), QVariant (1));
}


void LazyConditionWalkerTest::manipulateValue () {
	using namespace Nuria;
	
	// 
	LazyCondition condition (val (1) == val (false) && val (3) < val (4));
	
	Callback testFunc = Callback::fromLambda([]() {
			    return true;
	});
	
	// 
	LazyConditionWalker walker;
	walker.onVariant (QMetaType::Int, testFunc);
	
	LazyCondition result = walker.walk (condition);
	LazyCondition left = result.left ().value< LazyCondition > ();
	LazyCondition right = result.right ().value< LazyCondition > ();
	
	QCOMPARE(result.type (), LazyCondition::LogicAnd);
	QCOMPARE(left.type (), LazyCondition::Equal);
	QCOMPARE(right.type (), LazyCondition::Less);
	QCOMPARE(left.left (), QVariant (true));
	QCOMPARE(left.right (), QVariant (false));
	QCOMPARE(right.left (), QVariant (true));
	QCOMPARE(right.right (), QVariant (true));
	
}

void LazyConditionWalkerTest::manipulateArgument () {
	
	using namespace Nuria;
	
	// 
	LazyCondition condition (val (1) == arg (1) && test ("foo", arg (2)));
	
	Callback testFunc = Callback::fromLambda([]() {
			    return 1;
	});
	
	// 
	LazyConditionWalker walker;
	walker.onField (Field::Argument, testFunc);
	
	LazyCondition result = walker.walk (condition, true);
	LazyCondition left = result.left ().value< LazyCondition > ();
	LazyCondition right = result.right ().value< LazyCondition > ();
	Field field = right.left ().value< Field > ();
	TestCall call = field.value ().value< TestCall > ();
	
	QCOMPARE(result.type (), LazyCondition::LogicAnd);
	QCOMPARE(left.type (), LazyCondition::Equal);
	QCOMPARE(left.left (), QVariant (1));
	QCOMPARE(left.right (), QVariant (1));
	QCOMPARE(call.name (), QString("foo"));
	QCOMPARE(call.arguments ().length (), 1);
	QCOMPARE(call.arguments ().first ().toInt (), 1);
	
}

QTEST_MAIN(LazyConditionWalkerTest)
#include "tst_lazyconditionwalker.moc"
