/*
 * Filename: window.hpp
 * 
 * This file is a combination of original work and a template
 * provided from the Qt project maintainers. The project page
 * can be found here:
 *  http://doc.qt.digia.com/qt/opengl-hellogl-window-h.html
 *
 * Header file showing interfaces available to the QT Window.
 * When this is instantiated, we basically have the program
 * sit in an event loop waiting for us to mutate controls
 * (which are the private data members).
 */

/* LICENSE INFORMATION
   File has been considerably modified from its source author:
   Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).

   Original License: BSD
 */

#ifndef _WINDOW_H
#define _WINDOW_H

#include <QWidget>     // The widgets like slider, button, etc...

// Forward declarations of the necessary QWidgets we'll use
class QSlider;
class GLWidget;
class QPushButton;

/*
 * The window we create publicly-inherits from the far-reaching
 * QWidget class as per the API requirements.
 */
class Window: public QWidget
{
Q_OBJECT

public:
        Window();

protected:
        // Handles the pressing of keys
        void keyPressEvent( QKeyEvent *event );

private:
        // Creates a slider ideal for the adjustment of angles
        QSlider *createSlider();

        // Creates a Qt slider for frame-buffer coordinates
        QSlider *unitSlider();

        // Names of the mutable widgets to be implemented
        GLWidget *glWidget;
        QSlider *xSlider;
        QSlider *ySlider;
        QSlider *zSlider;
        QPushButton *roomLightSwitch;
        QPushButton *auxiliarySwitch;
        QPushButton *oppositeSwitch;
        QSlider *redSlider, *grnSlider, *bluSlider, *alpSlider;
};

#endif    //_WINDOW_H
