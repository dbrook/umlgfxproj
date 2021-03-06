/*
 * Filename: window.hpp
 * 
 * This file is a combination of original work and a template
 * provided from the Qt project maintainers. The project page
 * can be found here:
 *  http://doc.qt.digia.com/qt/opengl-hellogl.html
 * 
 * See LICENSE for details
 *
 * File Description:
 * Header file showing interfaces available to the QT Window.
 * When this is instantiated, we basically have the program
 * sit in an event loop waiting for us to mutate controls
 * (which are the private data members).
 */


#ifndef _WINDOW_H
#define _WINDOW_H

#include <QWidget>     // The widgets like slider, button, etc...

// Forward declarations of the necessary QWidgets we'll use
class QSlider;
class GLWidget;
class QPushButton;
class QRadioButton;
class QDoubleSpinBox;

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
        // Handles the pressing of keys and wheel motion in the scene
        void keyPressEvent( QKeyEvent *event );
        void keyReleaseEvent( QKeyEvent *event );

        // This should probably have been in the GLWidget object, because
        // now scrolling anywhere (not over a scroll-accepting widget) will
        // automagically be captured by the GLWidget.
        void wheelEvent( QWheelEvent *event );

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
        QPushButton *masterReset;
        QSlider *redSlider, *grnSlider, *bluSlider, *alpSlider;
        QRadioButton *p_orth, *p_pers;
        QDoubleSpinBox *modifyScale;
};

#endif    //_WINDOW_H
