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

#ifndef QTOMATO_H
#define QTOMATO_H

#include <QtGui>
#include <phonon/MediaObject>
#include <phonon/MediaSource>


QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QLabel;
class QMenu;
class QSlider;
class QTimer;
class QSystemTrayIcon;
QT_END_NAMESPACE


#define DEFAULT_POMODORO 25
#define DEFAULT_SHORT_BREAK 5
#define DEFAULT_LONG_BREAK 15

#define MINUTE_MS 60000

#define OPTION_WIDTH 270
#define OPTION_HEIGHT 200

#define TOMATO_ICON ":/images/tomato-48.png"
#define BELL_SOUND ":/sounds/deskbell.wav"

class QTomato : public QDialog
{
    Q_OBJECT

public:
    QTomato();
    ~QTomato();

public slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void tomatoPeriodChanged();
    void shortBreakPeriodChanged();
    void longBreakPeriodChanged();
    void soundNotificationCheckboxChanged();

    void pomodoroTimerExpired();
    void countdownTimerExpired();

    void timerSwitched();

    void pomodoroCounterReset();

private:
    void createOptionDialog();
    void createTrayIcon();
    void initializeTimers();

    void updateCountdown();

    void readSettings();
    void writeSettings();

    // GUI
//    QAction *toggleAction;
    QAction *collectedAction;
    QAction *optionAction;
    QAction *quitAction;

    QSlider *tomatoSlider;
    QSlider *shortBreakSlider;
    QSlider *longBreakSlider;

    QCheckBox *soundCheckbox;

    QLabel *pomodoroValueLabel;
    QLabel *shortBreakValueLabel;
    QLabel *longBreakValueLabel;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    // Timers
    QTimer *tomatoTimer;
    QTimer *countdownTimer;

    // Model
    int countdownIndex;

    int pomodoroPeriod;
    int shortBreakPeriod;
    int longBreakPeriod;

    int pomodoroCollected;

    bool isSoundEnabled;

    enum TIMER_STATE { OFF, ON, BREAK };
    TIMER_STATE state;
};

#endif // QTOMATO_H
