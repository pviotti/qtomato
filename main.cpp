/*
 * Copyright 2012 Paolo Viotti <paolo.viotti@gmail.com>
 *
 * This file is part of QTomato.
 *
 * QTomato is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * QTomato is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QTomato; If not, see http://www.gnu.org/licenses/.
 */

#include <QtGui>
#include "qtomato.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(qtomato);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("QTomato"),
                              QObject::tr("I couldn't detect any system tray on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    QTomato qtomato;
    return app.exec();
}
