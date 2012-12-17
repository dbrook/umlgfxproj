/*
 * Filename: main.cpp
 * 
 * This file is a combination of original work and a template
 * provided from the Qt project maintainers. The project page
 * can be found here:
 *  http://doc.qt.digia.com/qt/opengl-hellogl-main-cpp.html
 *
 * This file contains the main() function that will launch into
 * the QApplication for the OpenGL framebuffer contained in a
 * QWidget as well as the controls associated with the scene.
 *
 * It's important to note that the application will sit in an
 * event loop waiting for the user to interact with it, just
 * like regular OpenGL we've seen this semester.
 */

/* LICENSE INFORMATION
   File has been considerably modified from its source author:
   Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).

   Original License: BSD
 */

#include <QApplication>     // Needed to pull in Qt app. framework
#include <QDesktopWidget>   // Pulls in the Qt - Window Manager i-face
#include <string>
#include <iostream>

#include "window.hpp"       // Actual interface to the GUI window

/***********************************************************************
 * Main begins program execution
 **********************************************************************/

int main( int argc, char *argv[] )
{
        // Make a QApplication that can take any command line arguments
        QApplication app( argc, argv );

        if (argc != 2) {
                std::cerr << "You must provide a model file path (relative to working directory)" << std::endl;
                exit( 0 );
        }

        Window window;   // Creates a window (widgets will go in there)
        window.resize( window.sizeHint() );

        // Is window big enough to warrant starting maximized?
        int desktopArea = QApplication::desktop()->width()
                        * QApplication::desktop()->height();
        int widgetArea = window.width() * window.height();

        // Actually prepare to display the window now
        if (((float) widgetArea / (float) desktopArea) < 0.75f)
                window.show();
        else
                window.showMaximized();

        // Start running the application code for the GUI.
        return app.exec();
}
