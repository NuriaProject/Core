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

#include "templateengine.hpp"

#include <QMetaMethod>
#include <QStringList>
#include <QIODevice>
#include <QRegExp>
#include <QDebug>

#include "variant.hpp"

// 
namespace Nuria {
class TemplateEnginePrivate {
public:
	QString templateData;
	QVariantMap variables;
	
};
}

Nuria::TemplateEngine::TemplateEngine (const QString &templData) {
	
	// 
	this->d_ptr = new TemplateEnginePrivate;
	this->d_ptr->templateData = templData;
	
}

Nuria::TemplateEngine::TemplateEngine (QIODevice *readDevice) {
	
	// 
	this->d_ptr = new TemplateEnginePrivate;
	
	// Read from readDevice
	QByteArray data = readDevice->readAll ();
	
	// Store as UTF-8 data.
	this->d_ptr->templateData = QString::fromUtf8 (data.constData (), data.length ());
	
}

Nuria::TemplateEngine::~TemplateEngine () {
	delete this->d_ptr;
}

QString Nuria::TemplateEngine::templateData () const {
	return this->d_ptr->templateData;
}

void Nuria::TemplateEngine::bind (const QString &name, const QVariant &data) {
	this->d_ptr->variables.insert (name, data);
}

void Nuria::TemplateEngine::bind (const QString &name, QObject *object) {
	
	this->d_ptr->variables.insert (name, QVariant::fromValue (static_cast< QObject * > (object)));
	
}

void Nuria::TemplateEngine::unbind (const QString &name) {
	this->d_ptr->variables.remove (name);
}

void Nuria::TemplateEngine::clearBindings () {
	this->d_ptr->variables.clear ();
}

void Nuria::TemplateEngine::setTemplateData (const QString &templData) {
	this->d_ptr->templateData = templData;
}

QString Nuria::TemplateEngine::generate () {
	
	return generate (this->d_ptr->templateData, this->d_ptr->variables);
	
}

/**
 * Helper function for generate(). Returns the value for \a name from vars.
 * Also supports directly accessing QVariantMaps and QObject properties.
 * If \a has is \c true, then this method won't invoke any method but just
 * check if \a name exists. If \a name doesn't exist, the returned QVariant
 * will be invalid.
 */
static QVariant replacementData (const QString &name, const QVariantMap &vars, bool has = false) {
	using namespace Nuria;
	
	// Check for name
	QVariantMap::ConstIterator it = vars.constFind (name);
	if (it != vars.constEnd ()) {
		
		if (has) {
			return true;
		}
		
		return it.value ();
	}
	
	// Is it possibly a map or a QObject*?
	int idx = name.indexOf (QLatin1Char ('.'));
	if (idx == -1) {
		return QVariant ();
	}
	
	// 
	QString objName = name.left (idx);
	
	// 
	QVariant variant = vars.value (objName);
	
	// Nothing found?
	if (!variant.isValid ()) {
		return QVariant ();
	}
	
	// 
	QString property = name.mid (idx + 1);
	
	// Is variant a map?
	if (Variant::isMap (variant)) {
		Variant::Iterator var = Variant::find (variant, property);
		
		if (!var.isValid ()) {
			return QVariant ();
		} else if (has) {
			return true;
		}
		
		return var.value ();
	}
	
	// Is variant a list?
	if (Variant::isList (variant)) {
		
		int items = Variant::itemCount (variant);
		
		// 
		if (items == 0) {
			return QVariant ();
		}
		
		// Is the 'property' an integer?
		bool isInt = false;
		int index = property.toInt (&isInt);
		if (isInt) {
			
			// Check bounding
			if (index < 0 || index >= items) {
				return QVariant ();
			}
			
			// Return item at that position
			Variant::Iterator it = Variant::begin (variant);
			it += index;
			
			return *it;
			
		}
		
		// First, Last or Random?
		if (!property.compare (QLatin1String ("first"), Qt::CaseInsensitive)) {
			return Variant::begin (variant).value ();
		} else if (!property.compare (QLatin1String ("last"), Qt::CaseInsensitive)) {
			return (Variant::end (variant)--).value ();
		} else if (!property.compare (QLatin1String ("random"), Qt::CaseInsensitive)) {
			int index = qrand () % items;
			return (Variant::begin (variant) + index).value ();
		}
		
		// No match
		return QVariant ();
		
	}
	
	// Fail if variant is not a QObject*
	if (variant.userType () != qMetaTypeId< QObject * > ()) {
		return QVariant ();
	}
	
	// It is a QObject*.
	QObject *obj = variant.value< QObject * > ();
	
	// Does the property end with ()?
	if (property.endsWith (QLatin1String ("()"))) {
		
		// Try to invoke a slot called 'property'
		const QMetaObject *meta = obj->metaObject ();
		int slot = meta->indexOfSlot (qPrintable (property));
		
		// Failed.
		if (slot == -1) {
			return QVariant ();
		}
		
		// 
		QMetaMethod method = meta->method (slot);
		
		// Dynamically construct return type
		const char *typeName = method.typeName ();
		int type = QMetaType::type (typeName);
		
		if (!*typeName || type == 0) {
			return QVariant ();
		}
		
		// Should we only check if there is a method?
		if (has) {
			return true;
		}
		
		QVariant result (type, (void *)0);
		
		// The following is insane:
		QGenericReturnArgument retArg (typeName, result.data ());
		
		// Invoke
		bool success = method.invoke (obj, Qt::DirectConnection, retArg);
		
		// Done.
		if (success) {
			return result;
		}
		
		// Failed.
		return QVariant ();
		
	}
	
	// It is probably a property.
	return obj->property (qPrintable (property));
	
}

/**
 * Finds the end of a {%Modifier:...} block, which is always indicated by "{%%}".
 * Returns \c true on success. It stores the start-position of "{%%}" in \a end.
 * \a start is expected to point *after* the starting block!
 * \a elsePos points to the before "{%else}" if one is found in the current scope.
 * It is set to \c -1 else.
 * \a elseLen contains the length of the "{%else}" tag.
 */
static bool findBlock (const QString &data, int start, int &end, int &elsePos, int &elseLen) {
	QRegExp rxBlock (QLatin1String ("<%(else)>|"
					"<%!?[a-z]+(?::[-a-z0-9_.]+(?:\\(\\))?)?>|"
					"<%%>"), Qt::CaseInsensitive);
	
	int curPos = start;
	int blockCount = 1;
	elsePos = -1;
	elseLen = 0;
	end = -1;
	
	while (blockCount > 0 && (curPos = data.indexOf (rxBlock, curPos)) > -1) {
		
		// 
		if (data.at (curPos + 2).unicode () == '%') {
			blockCount--;
			
			if (blockCount == 0) {
				end = curPos;
			}
			
		} else {
			
			if (blockCount == 1 &&
			    !rxBlock.cap (1).compare (QLatin1String ("else"), Qt::CaseInsensitive)) {
				
				elsePos = curPos;
				elseLen = rxBlock.matchedLength ();
				
			} else {
				blockCount++;
			}
			
		}
		
		// 
		curPos += rxBlock.matchedLength ();
		
	}
	
	// Failed?
	if (end == -1) {
		return false;
	}
	
	return true;
}

/** Returns the length of the string, list or map inside \a variant. */
static int lengthOfVariant (const QVariant &variant) {
	
	// Map or list?
	if (Nuria::Variant::isList (variant) ||
	    Nuria::Variant::isMap (variant)) {
		return Nuria::Variant::itemCount (variant);
	}
	
	// Length of a string
	if (variant.type () == QVariant::String) {
		return variant.toString ().length ();
	}
	
	// 
	return 0;
}

static QString generateIntern (const QString &templateData, const QVariantMap &variables, int index, int total, int depth) {
	using namespace Nuria;
	
	if (depth > 50) {
		return QString ();
	}
	
	QRegExp rx (QLatin1String ("<(?:"
				   "%(!)?([a-z]+)(?::([-a-z0-9_.]+(?:\\(\\))?))?|"
				   "([=$])(?:(length):)?([-a-z0-9_.]+(?:\\(\\))?)"
				   ")>"), Qt::CaseInsensitive);
	
	QString result = templateData;
	
	int pos = 0;
	while ((pos = result.indexOf (rx, pos)) > -1) {
		
		QString name;
		QString modifier = rx.cap (2);
		bool invert = false;
		bool inlineTemplate = false;
		QString replacement;
		
		if (modifier.isEmpty ()) {
			name = rx.cap (6);
			inlineTemplate = (rx.cap (4).at (0).unicode () == '$');
		} else {
			invert = !rx.cap (1).isEmpty ();
			name = rx.cap (3);
			
		}
		
		if (modifier.isEmpty ()) {
			// Simple replacement or inline template
			QVariant data;
			
			if (!name.compare (QLatin1String ("CurrentIndex"), Qt::CaseInsensitive)) {
				data = index;
			} else if (!name.compare (QLatin1String ("TotalLength"), Qt::CaseInsensitive)) {
				data = total;
			} else {
				data = replacementData (name, variables);
			}
			
			QString method = rx.cap (5);
			if (!method.compare (QLatin1String ("length"), Qt::CaseInsensitive)) {
				
				int len = lengthOfVariant (data);
				replacement = QString::number (len);
				
			} else if (inlineTemplate) {
				
				// Evaluate as template
				replacement = generateIntern (Variant::toValue< QString > (data),
							      variables, index, total, depth + 1);
				
			} else {
				replacement = Variant::toValue< QString > (data);
				
			}
			
			result.replace (pos, rx.matchedLength (), replacement);
			
		} else {
			// Modifier block
			int elsePos;
			int elseLen;
			int end;
			
			// 
			static QString key (QLatin1String ("Key"));
			static QString value (QLatin1String ("Value"));
			
			// Find end of block
			int begin = pos + rx.matchedLength ();
			if (!findBlock (result, begin, end, elsePos, elseLen)) {
				return QString();
			}
			
			// Process modifier
			if (!modifier.compare (QLatin1String ("Each"), Qt::CaseInsensitive)) {
				// Each block
				
				// 
				QString part = result.mid (begin, end - begin);
				
				// Process list or map
				QVariantMap vars = variables;
				QVariant data = replacementData (name, variables);
				
				// Iterator
				Variant::Iterator it = Variant::begin (data);
				Variant::Iterator end = Variant::end (data);
				
				// 
				bool isMap = Variant::isMap (data);
				
				// 
				int i;
				int total = Variant::itemCount (data);
				
				// Iterate over data
				for (i = 0; it != end; ++it, i++) {
					
					// Store 'key' and 'value' variables
					if (isMap) {
						// Only override key variable if there is a key.
						vars.insert (key, it.key ());
					}
					
					vars.insert (value, it.value ());
					
					// Recurse into block
					replacement.append (generateIntern (part, vars, i, total, depth + 1));
					
				}
				
			} else if (!modifier.compare (QLatin1String ("Has"), Qt::CaseInsensitive)) {
				
				// Has?
				bool test = replacementData (name, variables, true).isValid ();
				if ((invert && !test) || (!invert && test)) {
					// Has!
					
					int blockEnd = (elsePos == -1) ? end : elsePos;
					replacement = generateIntern (result.mid (begin, blockEnd - begin),
								      variables, index, total, depth + 1);
					
				} else if (elsePos > -1) {
					// Nope. Execute else-block
					
					int blockBegin = elsePos + elseLen;
					replacement = generateIntern (result.mid (blockBegin, end - blockBegin),
								      variables, index, total, depth + 1);
					
				}
				
			} else if (!modifier.compare (QLatin1String ("Empty"), Qt::CaseInsensitive)) {
				
				// 
				QVariant data = replacementData (name, variables);
				bool test = !data.isValid () || lengthOfVariant (data) < 1;
				
				if ((invert && !test) || (!invert && test)) {
					// Empty
					
					int blockEnd = (elsePos == -1) ? end : elsePos;
					replacement = generateIntern (result.mid (begin, blockEnd - begin),
								      variables, index, total, depth + 1);
					
				} else if (elsePos > -1) {
					// Nope. Execute else-block
					
					int blockBegin = elsePos + elseLen;
					replacement = generateIntern (result.mid (blockBegin, end - blockBegin),
								      variables, index, total, depth + 1);
					
				}
			
			} else {
				// FirstItem, EvenItem, OddItem, LastItem
				
				// 
				bool test;
				
				// Check
				if (!modifier.compare (QLatin1String ("FirstItem"), Qt::CaseInsensitive)) {
					test = (index == 0);
				} else if (!modifier.compare (QLatin1String ("LastItem"), Qt::CaseInsensitive)) {
					test = (index == total - 1);
				} else if (!modifier.compare (QLatin1String ("OddItem"), Qt::CaseInsensitive)) {
					test = (index & 1);
				} else if (!modifier.compare (QLatin1String ("EvenItem"), Qt::CaseInsensitive)) {
					test = !(index & 1);
				} else {
					
					// Unknown modifier
					return result;
					
				}
				
				if ((invert && !test) || (!invert && test)) {
					// Pass
					int blockEnd = (elsePos == -1) ? end : elsePos;
					
					replacement = generateIntern (result.mid (begin, blockEnd - begin),
								      variables, index, total, depth + 1);
					
				} else if (elsePos > -1) {
					// Fail
					int blockBegin = elsePos + elseLen;
					
					replacement = generateIntern (result.mid (blockBegin, end - blockBegin),
								      variables, index, total, depth + 1);
					
				}
				
			}
			
			// Replace
			end += 4; // Length of "<%%>"
			result.replace (pos, end - pos, replacement);
			
		}
		
		// Skip replacement
		pos += replacement.length ();
		
	}
	
	// Done.
	return result;
	
}

QString Nuria::TemplateEngine::generate (const QString &templateData, const QVariantMap &variables) {
	return generateIntern (templateData, variables, 0, 0, 1);
}
