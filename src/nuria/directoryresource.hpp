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

#ifndef NURIA_DIRECTORYRESOURCE_HPP
#define NURIA_DIRECTORYRESOURCE_HPP

#include "resource.hpp"

namespace Nuria {

/** Map of entry names of a directory to their interface name. */
typedef QMap< QString /* entry */, QString /* interface name */ > DirectoryEntries;

/**
 * \brief Interface for the \c 'Directory' resource interface
 * 
 * The \c Directory interface lets you build a hierarchical resource structure,
 * much like a virtual file system ("VFS"). The interface only has two required
 * slots: \c list and \c get.
 * 
 * \par Slot: list
 * Acquires the list of known contained resources.
 * 
 * Arguments: None.
 * 
 * Result: DirectoryEntries
 * 
 * \par Slot: get
 * Acquires a contained resource by name. 
 * 
 * Arguments: \c name of type QString
 * 
 * Result: ResourcePointer
 * 
 */
class DirectoryResource : public Resource {
	Q_OBJECT
public:
	
	/** Constructor. */
	explicit DirectoryResource (QObject *parent = 0);
	
	/** Destructor. */
	~DirectoryResource () override;
	
	/**
	 * Lists the contents of the directory.
	 * 
	 * On success, \a callback is called with a DirectoryEntries result.
	 */
	virtual Invocation list (InvokeCallback callback, int timeout = -1) = 0;
	
	/**
	 * Gets the resource \a name.
	 * 
	 * On success, \a callback is called with a ResourcePointer result.
	 * If no resource called \a name is known to the directory, \a callback
	 * shall be called with a resultState of Resource::ResourceNotFoundError.
	 */
	virtual Invocation get (const QString &name, InvokeCallback callback, int timeout = -1) = 0;
	
	// Resource interface
	QString interfaceName () const override;
	Invocation properties (InvokeCallback callback, int timeout = -1) override;
	
protected:
	Invocation invokeImpl (const QString &slot, const QVariantMap &arguments,
	                       InvokeCallback callback, int timeout) override;
	
};

} // namespace Nuria

Q_DECLARE_METATYPE(Nuria::DirectoryEntries);

#endif // NURIA_DIRECTORYRESOURCE_HPP
