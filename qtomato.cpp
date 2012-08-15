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

#include "qtomato.h"

/*
 * TODO
 * - countdown color w/o dark themes
 * - option widget positioning
 * - qt single application
 * - translations
 */

QTomato::QTomato()
{
    QCoreApplication::setOrganizationName("pviotti");
    QCoreApplication::setApplicationName("QTomato");

    readSettings();

    createTrayIcon();
    createOptionDialog();
    initializeTimers();

    setWindowTitle(tr("QTomato"));
    setFixedSize(OPTION_WIDTH, OPTION_HEIGHT);
    setWindowIcon(QIcon(TOMATO_ICON));

    state = OFF;
    countdownIndex = pomodoroPeriod;
    trayIcon->show();
}

QTomato::~QTomato()
{
    writeSettings();
}


/* ==================================================================================
                                  Slot functions
   ================================================================================== */

void QTomato::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        timerSwitched();
        break;
    case QSystemTrayIcon::MiddleClick:
        isHidden() ? showNormal() : hide();
        break;
    default:;
    }
}

void QTomato::tomatoPeriodChanged()
{
    pomodoroPeriod = tomatoSlider->value();
    pomodoroValueLabel->setText(QString::number(pomodoroPeriod) + tr(" minutes"));
}

void QTomato::shortBreakPeriodChanged()
{
    shortBreakPeriod = shortBreakSlider->value();
    shortBreakValueLabel->setText(QString::number(shortBreakPeriod) + tr(" minutes"));
}

void QTomato::longBreakPeriodChanged()
{
    longBreakPeriod = longBreakSlider->value();
    longBreakValueLabel->setText(QString::number(longBreakPeriod) + tr(" minutes"));
}

void QTomato::soundNotificationCheckboxChanged()
{
    isSoundEnabled = soundCheckbox->isChecked();
}

void QTomato::pomodoroTimerExpired()
{
    switch (state) {
    case OFF:
        break;  // Should not happen
    case ON:
        state = BREAK;
        trayIcon->showMessage(tr("Pomodoro expired!"),
                              tr("This pomodoro has expired!\nPlease take a break."),
                              QSystemTrayIcon::Information, MINUTE_MS / 2);

        if (isSoundEnabled) {
            // Play a notification sound - NB: QSound does not work
            Phonon::MediaObject* sound;
            sound = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(BELL_SOUND));
            sound->play();
        }

        tomatoTimer->stop();
        countdownTimer->stop();

        trayIcon->setIcon(QIcon(TOMATO_ICON));
        pomodoroCollected++;
        collectedAction->setText("Reset [" + QString::number(pomodoroCollected) + " pomodoro]");
        pomodoroCollected == 0 ? collectedAction->setDisabled(true) :
                                 collectedAction->setDisabled(false);

        if (pomodoroCollected % 3 == 0) { // Long break
            tomatoTimer->setInterval(longBreakPeriod * MINUTE_MS);
            countdownIndex = longBreakPeriod;
        } else {    // Short break
            tomatoTimer->setInterval(shortBreakPeriod * MINUTE_MS);
            countdownIndex = shortBreakPeriod;
        }

        tomatoTimer->start();
        countdownTimer->start();

        updateCountdown();

        break;
    case BREAK:
        state = ON;
        trayIcon->showMessage(tr("A new pomodoro period started!"),
                              tr("Get busy!"),
                              QSystemTrayIcon::Information, MINUTE_MS / 2);

        tomatoTimer->setInterval(pomodoroPeriod * MINUTE_MS);
        tomatoTimer->start();

        countdownIndex = pomodoroPeriod;
        countdownTimer->setInterval(MINUTE_MS);
        countdownTimer->start();
        updateCountdown();

        break;
    }
}

void QTomato::countdownTimerExpired()
{
    updateCountdown();
}

void QTomato::timerSwitched()
{
    switch(state) {
    case OFF:
        state = ON;
        tomatoTimer->setInterval(pomodoroPeriod * MINUTE_MS);
        tomatoTimer->start();

        countdownIndex = pomodoroPeriod;
        countdownTimer->setInterval(MINUTE_MS);
        countdownTimer->start();
        updateCountdown();

//        toggleAction->setText(tr("Switch off"));
        break;
    default:
        state = OFF;
        trayIcon->setIcon(QIcon(TOMATO_ICON));
        tomatoTimer->stop();
        countdownTimer->stop();
//        toggleAction->setText(tr("Switch on"));
        break;
    }
}

void QTomato::pomodoroCounterReset()
{
    pomodoroCollected = 0;
    collectedAction->setText("Reset [" +
                             QString::number(pomodoroCollected) + " pomodoro]");
    collectedAction->setDisabled(true);
}

void QTomato::updateCountdown()
{
    QPixmap pixmap(16,16);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QPen pen;
    QString strNumber = QString::number(countdownIndex);
    if (state == ON)
        pen.setColor(QColor::fromRgb(255, 255, 255));     // white - FIXME: only fits on dark themes
    else
        pen.setColor(QColor::fromRgb(39, 152, 3));        // green
    painter.setPen(pen);
    painter.drawText(0,0,16,16,Qt::AlignHCenter | Qt::AlignVCenter, strNumber);
    trayIcon->setIcon(QIcon(pixmap));
    trayIcon->setToolTip(QString::number(countdownIndex) + tr(" minutes left"));
    countdownIndex--;
}


/* ==================================================================================
                                GUI and init functions
   ================================================================================== */

void QTomato::createOptionDialog()
{
    QLabel *pomodoroLabel = new QLabel(tr("Pomodoro duration:"));
    QLabel *shortBreakLabel = new QLabel(tr("Short break duration:"));
    QLabel *longBreakLabel = new QLabel(tr("Long break duration:"));

    pomodoroValueLabel = new QLabel(QString::number(pomodoroPeriod) + tr(" minutes"));
    shortBreakValueLabel = new QLabel(QString::number(shortBreakPeriod) + tr(" minutes"));
    longBreakValueLabel = new QLabel(QString::number(longBreakPeriod) + tr(" minutes"));

    tomatoSlider = new QSlider(Qt::Horizontal);
    tomatoSlider->setValue(pomodoroPeriod);
    tomatoSlider->setMaximum(60);

    shortBreakSlider = new QSlider(Qt::Horizontal);
    shortBreakSlider->setValue(shortBreakPeriod);
    shortBreakSlider->setMaximum(60);

    longBreakSlider = new QSlider(Qt::Horizontal);
    longBreakSlider->setValue(longBreakPeriod);
    longBreakSlider->setMaximum(60);

    soundCheckbox = new QCheckBox(tr("Sound notifications"));
    soundCheckbox->setChecked(isSoundEnabled);

    connect(tomatoSlider, SIGNAL(valueChanged(int)),
            this, SLOT(tomatoPeriodChanged()));
    connect(shortBreakSlider, SIGNAL(valueChanged(int)),
            this, SLOT(shortBreakPeriodChanged()));
    connect(longBreakSlider, SIGNAL(valueChanged(int)),
            this, SLOT(longBreakPeriodChanged()));
    connect(soundCheckbox, SIGNAL(toggled(bool)),
            this, SLOT(soundNotificationCheckboxChanged()));

    QHBoxLayout *horBox1 = new QHBoxLayout;
    QHBoxLayout *horBox2 = new QHBoxLayout;
    QHBoxLayout *horBox3 = new QHBoxLayout;

    horBox1->addWidget(pomodoroLabel);
    horBox1->addStretch();
    horBox1->addWidget(pomodoroValueLabel);

    horBox2->addWidget(shortBreakLabel);
    horBox2->addStretch();
    horBox2->addWidget(shortBreakValueLabel);

    horBox3->addWidget(longBreakLabel);
    horBox3->addStretch();
    horBox3->addWidget(longBreakValueLabel);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horBox1);
    mainLayout->addWidget(tomatoSlider);
    mainLayout->addLayout(horBox2);
    mainLayout->addWidget(shortBreakSlider);
    mainLayout->addLayout(horBox3);
    mainLayout->addWidget(longBreakSlider);
    mainLayout->addStretch();
    mainLayout->addSpacing(10);
    mainLayout->addWidget(soundCheckbox);

    setLayout(mainLayout);
}


void QTomato::createTrayIcon()
{
//    toggleAction = new QAction(tr("Switch on"), this);
//    connect(toggleAction, SIGNAL(triggered()), this, SLOT(timerSwitched()));

    collectedAction = new QAction(tr("Reset") + " (" +
                                  QString::number(pomodoroCollected) + " pomodoro)", this);
    connect(collectedAction, SIGNAL(triggered()), this, SLOT(pomodoroCounterReset()));
    pomodoroCollected == 0 ? collectedAction->setDisabled(true) :
                             collectedAction->setDisabled(false);

    optionAction = new QAction(tr("&Options"), this);
    connect(optionAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    trayIconMenu = new QMenu(this);
//    trayIconMenu->addAction(toggleAction);
    trayIconMenu->addAction(collectedAction);
    trayIconMenu->addAction(optionAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    trayIcon->setIcon(QIcon(TOMATO_ICON));

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void QTomato::initializeTimers()
{
    tomatoTimer = new QTimer;
    connect(tomatoTimer, SIGNAL(timeout()), this, SLOT(pomodoroTimerExpired()));

    countdownTimer = new QTimer;
    connect(countdownTimer, SIGNAL(timeout()), this, SLOT(countdownTimerExpired()));
}


/* ==================================================================================
                           Save and load settings functions
   ================================================================================== */

void QTomato::writeSettings()
 {
     QSettings settings;

     settings.beginGroup("OptionWindow");
     settings.setValue("pos", pos());
     settings.endGroup();

     settings.beginGroup("Pomodoro");
     settings.setValue("collected", pomodoroCollected);
     settings.setValue("pomodoroDuration", pomodoroPeriod);
     settings.setValue("shortBreakDuration", shortBreakPeriod);
     settings.setValue("longBreakDuration", longBreakPeriod);
     settings.setValue("soundEnabled", isSoundEnabled);
     settings.endGroup();
 }

 void QTomato::readSettings()
 {
     QSettings settings;

     QDesktopWidget *desktop = QApplication::desktop();
     int defaultX = desktop->width() - OPTION_WIDTH;
     int defaultY = desktop->height() -OPTION_HEIGHT;

     settings.beginGroup("OptionWindow");
     move(settings.value("pos", QPoint(defaultX, defaultY)).toPoint());
     settings.endGroup();

     settings.beginGroup("Pomodoro");
     pomodoroCollected = settings.value("collected", 0).toInt();
     pomodoroPeriod = settings.value("pomodoroDuration", DEFAULT_POMODORO).toInt();
     shortBreakPeriod = settings.value("shortBreakDuration", DEFAULT_SHORT_BREAK).toInt();
     longBreakPeriod = settings.value("longBreakDuration", DEFAULT_LONG_BREAK).toInt();
     isSoundEnabled = settings.value("soundEnabled", true).toBool();
     settings.endGroup();
 }
