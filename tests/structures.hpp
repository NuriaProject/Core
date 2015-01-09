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

#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <nuria/essentials.hpp>
#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QMap>

struct NURIA_INTROSPECT Simple {
	Simple () {}
	
	int digit = 0;
	QString string;
	float number = 0.f;
	bool boolean = false;
};

struct NURIA_INTROSPECT Custom {
	Custom () {}
	
	int foo;
	QDateTime dateTime;
};

struct NURIA_INTROSPECT Complex {
	Complex () {}
	
	Simple simple;
	int outer;
};

struct NURIA_INTROSPECT Recursive {
	Recursive () {}
	~Recursive () { delete recurse; }
	
	Recursive *recurse = nullptr;
	int depth;
};

struct NURIA_INTROSPECT Fail {
	Fail () {}
	
	bool works;
	QList< int > someList;
};

struct SomeCustomType {
	SomeCustomType () {}
	
	bool works = false;
	
	// For QVariant::convert conversion
	static QString toString (const SomeCustomType &)
	{ return "foo"; }
	
	static SomeCustomType fromString (const QString &str) {
		qDebug("fromString: %s", qPrintable (str));
		SomeCustomType t;
		t.works = (str == "foo");
		return t;
	}
	
};

struct NURIA_INTROSPECT WithCustomType {
	WithCustomType () {}
	
	SomeCustomType custom;
};

struct NURIA_INTROSPECT CustomConverter {
	int integer;
	
	bool a;
	bool b;
};

struct NURIA_INTROSPECT WithConstructor {
	
	WithConstructor (int integer) {
		this->integer = integer;
		qDebug("int %i", integer);
	}
	
	WithConstructor () {
		qDebug("Wrong ctor!");
	}
	
	int integer;
	QString string;
};

Q_DECLARE_METATYPE(SomeCustomType)
Q_DECLARE_METATYPE(Simple*)

#endif // STRUCTURES_HPP
