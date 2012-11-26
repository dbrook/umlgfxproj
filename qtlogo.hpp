/*
 * Filename: qtlogo.hpp
 * 
 * This file comes from:
 *  http://doc.qt.digia.com/qt/opengl-hellogl.html
 * 
 * See LICENSE for details
 *
 * File Description:
 * A little segment of the program to create a Qt logo from scratch.
 */


#ifndef _QTLOGO_H
#define _QTLOGO_H

#include <QObject>
#include <QColor>

class Patch;
// Forward declaration
struct Geometry;
// Forward declaration

class QtLogo: public QObject
{
public:
        // constructor and destructor
        QtLogo( QObject *parent, int d = 64, qreal s = 1.0 );
        ~QtLogo();

        // member functions to set color and redraw
        void setColor( QColor c );
        void draw() const;

private:
        void buildGeometry( int d, qreal s );

        // data members
        QList<Patch *> parts;
        Geometry *geom;
};

#endif    //_QTLOGO_H
