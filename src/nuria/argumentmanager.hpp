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

#ifndef NURIA_ARGUMENTMANAGER_HPP
#define NURIA_ARGUMENTMANAGER_HPP

#include <QObject>
#include <QMap>

#include "essentials.hpp"
#include <climits>

namespace Nuria {

/** 
 * \brief Provides convenient access to arguments passed to the launcher.
 * 
 * The argument manager provides easy access to arguments passed to
 * the launcher.
 * The syntax of an argument is the following:
 * <pre>[Path]=[Value]</pre>
 * The \a path is a virtual path to the setting the user wants to modify.
 * A point (".") is used as a delimeter. Service authors must use the \a serviceName
 * as "root". Widget authors should use something similar.
 * The root "Nuria" is reserved for the system itself. Paths are not case sensitive.
 * The path must not contain whitespaces.
 */
class NURIA_CORE_EXPORT ArgumentManager : public QObject {
	Q_OBJECT
public:
	
	~ArgumentManager ();
	
	/**
	 * Returns the value of \a path. If no value can be found
	 * \a defaultValue is returned instead.
	 */
	static QString getValue (const QString &path, const QString &defaultValue = QString ());
	
	/**
	 * Returns the value of \a path. The value is expected to be a integer
	 * in range of \a min and \a max. If the given value is not a integer
	 * or doesn't fit into the given range \a ok will be set to \c false
	 * and \a defaultValue will be returned.
	 */
	static int getInt (const QString &path, int defaultValue = 0,
			   int min = INT_MIN, int max = INT_MAX, bool *ok = 0);
	
	/**
	 * Returns \c true if there is an argument with \a path. If not
	 * returns \c false.
	 */
	static bool contains (const QString &path);
	
private:
	ArgumentManager (QObject *parent = 0);
	
};
}

#endif // NURIA_ARGUMENTMANAGER_HPP
