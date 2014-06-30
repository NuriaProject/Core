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
