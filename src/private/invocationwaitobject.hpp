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

#ifndef NURIA_INTERNAL_INVOCATIONWAITOBJECT_HPP
#define NURIA_INTERNAL_INVOCATIONWAITOBJECT_HPP

#include <QObject>

namespace Nuria {
namespace Internal {

// Dummy class
class InvocationWaitObject : public QObject {
	Q_OBJECT
signals:
	
	// The only reason for this class is this signal.
	void wakeup ();
	
};

}
}

#endif // NURIA_INTERNAL_INVOCATIONWAITOBJECT_HPP
