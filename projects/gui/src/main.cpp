/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cutechessapp.h"

#include <QTextStream>
#include <QStringList>
#include <QMetaType>

#include <board/genericmove.h>
#include <board/move.h>
#include <board/side.h>
#include <moveevaluation.h>

int main(int argc, char* argv[])
{
	// Register types for signal / slot connections
	qRegisterMetaType<Chess::GenericMove>("Chess::GenericMove");
	qRegisterMetaType<Chess::Move>("Chess::Move");
	qRegisterMetaType<Chess::Side>("Chess::Side");
	qRegisterMetaType<MoveEvaluation>("MoveEvaluation");

	CuteChessApplication app(argc, argv);

	QStringList arguments = app.arguments();
	arguments.takeFirst(); // application name

	// Use trivial command-line parsing for now
	QTextStream out(stdout);
	while (!arguments.isEmpty())
	{
		if (arguments.first() == QLatin1String("-v") ||
			arguments.first() == QLatin1String("--version"))
		{
			out << "Cute Chess " << CUTECHESS_VERSION << endl;
			out << "Using Qt version " << qVersion() << endl << endl;
            out << "Modified by cdcbb in May 2016 " << endl;
			out << endl << endl;

			return 0;
		}
		else
		{
			out << "Unknown argument: " << arguments.first() << endl;
		}
		arguments.takeFirst();
	}
	app.newDefaultGame();
	return app.exec();
}
