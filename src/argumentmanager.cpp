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

#include "nuria/argumentmanager.hpp"

#include <QCoreApplication>
#include <QStringList>
#include <QFile>

namespace Nuria {

struct VariableData {
	QString value;
	bool append;
};

struct ArgumentManagerPrivate {
	QMap< QString, VariableData > variables;
};

}

// Global static instance
static Nuria::ArgumentManager *instance = 0;
static Nuria::ArgumentManagerPrivate *g_data = 0;

static void matchArgument (const QString &argument) {
	
	// Regular expression for parsing "[name]=[value]"
	static QRegExp rx ("[ \\t]*([a-z0-9\\.\\-_]+)[ \\t]*(\\+?\\=)(.*)", Qt::CaseInsensitive);
	
	// Does it match?
	if (rx.indexIn (argument) == 0 && rx.captureCount () == 3) {
		QString name = rx.cap (1).toLower ();
		
		if (rx.cap (2) == QLatin1String ("+=")) {
			
			Nuria::VariableData data = g_data->variables.value (name);
			
			if (!g_data->variables.contains (name))
				data.append = true;
			
			data.value += rx.cap (3);
			g_data->variables.insert (name, data);
			
		} else {
			Nuria::VariableData data;
			data.value = rx.cap (3);
			data.append = false;
			
			g_data->variables.insert (name, data);
		}
		
	}
	
}

Nuria::ArgumentManager::ArgumentManager (QObject *parent)
	: QObject (parent)
{
	
	// 
	g_data = new Nuria::ArgumentManagerPrivate;
	
	// 
	QStringList args = qApp->arguments ();
	QString configPath = qApp->applicationDirPath () + "/settings.cfg";
	
	// Has the user passed a different path to the settings file?
	for (int i = 1; i < args.length (); i++) {
		const QString &cur = args.at (i);
		
		if (cur.startsWith (QLatin1String ("nuria.settings="), Qt::CaseInsensitive)) {
			configPath = cur.mid (cur.indexOf ('=') + 1);
			break;
		}
		
	}
	
	// Load additional settings from settings.cfg
	QFile config (configPath);
	if (config.open (QIODevice::ReadOnly | QIODevice::Text)) {
		
		while (!config.atEnd ()) {
			QString line = config.readLine ();
			
			if (line.indexOf ('#') != -1)
				line = line.left (line.indexOf ('#'));
			else if (line.endsWith (QLatin1Char ('\n')))
				line.chop (1);
			else if (line.endsWith (QLatin1String ("\r\n")))
				line.chop (2);
			
			if (!line.isEmpty ())
				matchArgument (line);
			
		}
		
	}
	
	// Parse command line arguments
	for (int i = 1; i < args.length (); i++)
		matchArgument (args.at (i));
	
}

Nuria::ArgumentManager::~ArgumentManager () {
	delete g_data;
}

QString Nuria::ArgumentManager::getValue (const QString &path, const QString &defaultValue) {
	if (!instance) {
		instance = new ArgumentManager (qApp);
	}
	
	// Use iterators so we can access the value by reference.
	QMap< QString, Nuria::VariableData >::ConstIterator it =
			g_data->variables.constFind (path.toLower ());
	
	if (it == g_data->variables.constEnd ())
		return defaultValue;
	
	// Appending?
	if (it.value ().append)
		return defaultValue + it.value ().value;
	
	return it.value ().value;
	
}

int Nuria::ArgumentManager::getInt (const QString &path, int defaultValue, int min, int max, bool *ok) {
	
	// Read ...
	QString intStr = getValue (path);
	
	// Not set?
	if (intStr.isEmpty ()) {
		if (ok) {
			*ok = true;
		}
		
		return defaultValue;
	}
	
	// Parse integer
	bool resultOk = false;
	int result = intStr.toInt (&resultOk);
	
	// Valid result?
	bool valid = (resultOk && result >= min && result <= max);
	
	// Set 'error'
	if (ok) {
		*ok = valid;
	}
	
	// Return value
	if (!valid) {
		return defaultValue;
	}
	
	return result;
}

bool Nuria::ArgumentManager::contains (const QString &path) {
	
	if (!instance) {
		instance = new ArgumentManager (qApp);
	}
	
	return g_data->variables.contains (path.toLower ());
	
}
