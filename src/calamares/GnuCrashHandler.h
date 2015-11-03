/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2015, Teo Mrnjavac <teo@kde.org>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GNUCRASHHANDLER_H
#define GNUCRASHHANDLER_H

#include <QString>

#include <signal.h>

class QApplication;

class GnuCrashHandler
{
public:
    static void install( const QString& backtracePath,
                         const QList< int >& signalsToCatch );

    static GnuCrashHandler* instance();

    QString backtracePath() const { return m_backtracePath; }

private:
    explicit GnuCrashHandler( const QString& backtracePath,
                              const QList< int >& signalsToCatch );

    void installSignal(int __sig);

    QString m_backtracePath;

    static GnuCrashHandler* s_instance;
};

#endif // GNUCRASHHANDLER_H
