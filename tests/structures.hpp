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
