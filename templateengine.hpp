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

#ifndef NURIA_TEMPLATEENGINE_HPP
#define NURIA_TEMPLATEENGINE_HPP

#include "essentials.hpp"
#include <QVariant>
class QIODevice;

namespace Nuria {

class TemplateEnginePrivate;

/**
 * This class provides a simple to use templating engine for text data.
 * 
 * \par Variables
 * Data is read as byte stream. To access bound variables use:
 * \codeline <=Variablename>
 * 
 * A bound variable may be of any type that can be converted to a QString.
 * Also supported are lists and maps which are registered to the Nuria::Variant
 * system.
 * Variable names may consist of the following characters: a-z, 0-9, "_", "-"
 * and ".". Names are always matched case-sensitive!
 * 
 * \par Working with lists
 * Lists support direct access of elements (Just like maps do). To directly
 * access an item at a known index write the index after a point:
 * \codeline <=List.123>
 * \note Valid indexes are: 0 <= Index < Length of list
 * 
 * To access the first or last item in a list, use "first" or "last"
 * respectively:
 * \codeline <=List.first> ... <=List.last>
 * 
 * To access an item at a random index, use "random":
 * \codeline <=List.random>
 * \note To insert the length of a list use the \b Length modifier.
 * 
 * \par Modifiers
 * A modifier is a special command which evaluates a block only if its
 * statement is \c true. A modifier always begins with "%" instead of
 * "=" to be easily distinguishable. Most modifiers can have an
 * "<%else>" block which are evaluated when the statement turned out to
 * be \c false. A block is always ended with "<%%>". The modifiers which
 * can have an \b else block can also be negated using the negation operator
 * "!" written before the modifier itself.
 * \n
 * Example:
 * \code
 * Available languages:
 * <%!Empty:Langs>
 * <%Each:Langs>"<=Value>" <%%>
 * <%else>
 * None!
 * <%%>
 * \endcode
 * The example above first checks if "Langs" (which is a list) is not empty.
 * If it's not, it will iterate over all elements writing the value of each
 * one in quotation marks. If the list is empty, it will simply output "None!".
 * 
 * \par The 'Each' modifier
 * As mentioned above, you can also bind lists and maps. This way you can
 * easily have repetitive elements in your output. To do so use a 'each'
 * block:
 * \codeline <%Each:Variable>...<%%>
 * \note Blocks can be nested.
 * \note Blocks always end with "<%%>".
 * Inside an each block use \a Key and \a Value to access the key of a map or
 * the value of a map or list respectively.
 * \note \b Each does not support an \b else block nor does it support the
 * negation operator!
 * 
 * \par QVariantMap and QObject
 * You can also directly access values of a bound QVariantMap and a property
 * of a bound QObject* by delimting the the variable name and the property name
 * with a dot (".")
 * \codeline <=ObjectOrMap.Property>
 * It is also possible to call methods in a bound QObject* by appending "()" at
 * the end of the property name. Note that you can only invoke public slots
 * and that the slot can't take any argument.
 * \codeline <=Object.slot()>
 * \note The object must live in the same thread as the caller as the slot is
 * invoked using a Qt::DirectConnection!
 * \note You can apply all modifiers to all variable types. This also applies
 * to objects, maps and lists.
 * 
 * For example, you have a bound QObject "object" with a slot named "users()"
 * which when called returns a list of strings. You can output all elements as
 * you'd expect using \b Each:
 * \codeline <%Each:object.users()><=Value><%%>
 * 
 * \par The 'Has' modifier
 * If you need to check if there is a bound variable with a specific name
 * you can use the \b Has modifier:
 * \codeline Foo: <%Has:Foo><=Foo><%%>
 * Inside a 'Has' block, you can use a 'else' marker to output data if the
 * 'Has' condition fails:
 * \codeline Foo: <%Has:Foo><=Foo><%else><i>Not set</i><%%>
 * 
 * \par The 'Empty' modifier
 * Some times you don't care if a variable really exists or not. All you
 * want to know if a variable is empty (as in: Is defined (or not) and
 * contains data). For this use-case use \b Empty:
 * \codeline Your name <%Empty:Name>is empty<%else>is set<%%>
 * 
 * \par Inline templates
 * If you need to evaluate a variable like a template, use "$":
 * \codeline <$Translation.Welcome>
 * The variable is executed in the current scope and thus has access to all
 * bound variables.
 * \note The variable must be a string. If you need to execute a list or map
 * combine it with \b Each.
 * \note You're free to call inline templates redundant. For security reasons,
 * TemplateEngine allows a recursion depth of up to 50 calls. This should be
 * enough for any normal use-case and defend against attacks.
 * 
 * \par Additional modifiers for maps and lists
 * \note This section refers to \b Each blocks.
 * Often you need to know additional data when working with lists. For example
 * you need to know the current index. Or the total number of items. To handle
 * such situations this engine provides two special variables: \b CurrentIndex
 * and \b TotalLength. These variables represent the current item index in a
 * list or map and the total item count respectively. These variables are
 * also available in sub-scopes. Both refer to the next \b Each block. So, if
 * you nest multiple \b Each blocks, you only have access to the value from the
 * highest block. You can't access the value of parent blocks.
 * 
 * Beside these two special variables, various special modifiers are provided:
 * - A \b FirstItem block is only executed when this is the first item.
 * - A \b LastItem block is only executed when this is the last item.
 * - A \b EvenItem block is only executed when the current index is \a even.
 * - A \b OddItem block is only executed when the current index is \a odd.
 * 
 * \code 
 * <%Each:List>
 * <=CurrentIndex>: <=Value>
 * (<%FirstItem>First<%%> <%LastItem>Last<%%> <%OddItem>Odd<%%> <%EvenItem>Even<%%>)<br />
 * <%%>
 * \endcode
 * 
 * \par The 'Length' modifier
 * The \b Length modifier lets you access the length of strings and the item
 * count of lists and maps. This modifier is special as it only expands to
 * an integer, and thus doesn't open a new block and can't be negated. It is
 * used with the assignment operator "=":
 * \codeline <=Length:StringOrMap>
 * 
 */
class NURIA_CORE_EXPORT TemplateEngine {
public:
	
	/**
	 * Constructs a template engine instance with templateData set
	 * to \a templData.
	 */
	TemplateEngine (const QString &templData = QString ());
	
	/**
	 * Constructs a template engine instance. The templateData is read from
	 * \a readDevice. Data is interpreted as UTF-8. The \a readDevice must
	 * be open and readable. All data will be read out of it. The device
	 * won't be closed afterwards and ownership will remain at the caller.
	 * \note \a readDevice can be closed after the instance has been
	 * created.
	 */
	TemplateEngine (QIODevice *readDevice);
	
	~TemplateEngine ();
	
	/**
	 * Returns the current template data.
	 * Is empty by default.
	 */
	QString templateData () const;
	
	/**
	 * Binds variable \a name to \a data.
	 */
	void bind (const QString &name, const QVariant &data);
	
	/**
	 * \overload
	 * Convenience overload for binding QObject*s.
	 */
	void bind (const QString &name, QObject *object);
	
	/**
	 * Removes the variable \a name.
	 */
	void unbind (const QString &name);
	
	/**
	 * Clears the list of bound variables.
	 */
	void clearBindings ();
	
	/**
	 * Sets the template data.
	 * \note This won't clean bound variables.
	 */
	void setTemplateData (const QString &templData);
	
	/**
	 * Generates the result of the template.
	 */
	QString generate ();
	
	/**
	 * \overload
	 */
	static QString generate (const QString &templateData, const QVariantMap &variables);
	
private:
	
	TemplateEnginePrivate *d_ptr;
	
};

}

#endif // NURIA_TEMPLATEENGINE_HPP
