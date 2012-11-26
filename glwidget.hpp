/*
 * Filename: glwidget.hpp
 * 
 * This file is a combination of original work and a template
 * provided from the Qt project maintainers. The project page
 * can be found here:
 *  http://doc.qt.digia.com/qt/opengl-hellogl.html
 * 
 * See LICENSE for details
 *
 * File Description:
 * Header file showing interfaces available to the actual widget
 * that will hold all of the OpenGL goodies.
 */


#ifndef _GLWIDGET_H
#define _GLWIDGET_H

#include <QGLWidget>   // The OpenGL "canvas" of sorts
class QtLogo;
// We'll use this for dummy test data for now

/*
 * The window we create publically-inherits from the far-reaching
 * QWidget class as per the API requirements.
 */
class GLWidget: public QGLWidget
{
Q_OBJECT

public:
        // Constructor and destructor for the OpenGL frame
        GLWidget( QWidget *parent = 0 );

        // Makes sure any OpenGL-specific data structures are cleaned
        ~GLWidget();

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

        // HIGLY recommend checking out SIGNALS & SLOTS in the README
        /*
         * These receiver slots will accept a new angle as an integer.
         * The slider widgets actually "emit" (send a SIGNAL) that is
         * basically "heard" by the GLWidget object.
         *
         * Essentially, signals/slots are used to allow objects to
         * interact with each other!
         */
public slots:
        /*
         * Change scene rotation via these slots
         */
        void setXRotation( int angle );
        void setYRotation( int angle );
        void setZRotation( int angle );

        void forward  ( void );
        void backward ( void );
        void strafeL  ( void );
        void strafeR  ( void );
        void incrElev ( void );
        void decrElev ( void );

        /*
         * Control function to emulate the room light being switched
         * on or off. This is a little bit of a "cheat" because while
         * we can easily turn the light sources on or off individually,
         * the background color is NOT impacted at all by this, so
         * this slot will also fudge the background to make it darker.
         */
        void lightAmbientToggle( void );

        void oppositeLightToggle( void );

        /*
         * Turn the spotlight on or off depending on the
         */
        void auxLightToggle( void );

        /*
         * Slots to adjust color components manually
         */
        void auxRed   ( int userRed );
        void auxGreen ( int userGreen );
        void auxBlue  ( int userBlue );
        void auxAlpha ( int userAlpha );

signals:
        /*
         * These signals get emitted so the GUI widgets can reflect
         * the change in scene rotation parameters (because the user
         * used the mouse to change the scene, for example).
         */
        void xRotationChanged( int angle );
        void yRotationChanged( int angle );
        void zRotationChanged( int angle );

        /*
         * The current lighting state should get emitted to update
         * GUI widgets (for example: turning the "Turn On" button
         * to say "Turn Off" when the light source is enabled).
         *
         * (Might not actually implement these, but good to have
         * if we want to easily turn such an option on later)
         */
        void ambientLightChanged( bool qOn );

protected:
        /*
         * VERY VERY VERY REDICULOUSLY IMPORTANT INFO RIGHT HERE!!
         * 
         * Initializing the OpenGL systems, resizing of the viewport,
         * and actually drawing the scenes are handled by redefining
         * the skeleton functions given by QGLWidget (the framework's
         * copy of it is essentially empty and we put our OpenGL code
         * in these by simply redefining the member functions).
         */
        void initializeGL();
        void paintGL();
        void resizeGL( int width, int height );  // Called on every resize

        // Mouse-button-was-pressed within the framebuffer (EVENT HANDLER)
        void mousePressEvent( QMouseEvent *event );

        // Mouse-was-moved within the framebuffer (EVENT HANDLER)
        void mouseMoveEvent( QMouseEvent *event );

private:
        /*
         * Since we'll only have a single instance of this GLWidget,
         * these are kind of like the globals we had in the Angel exs.
         */
        QtLogo *logo;      // The logo object that will show on the screen
        int xRot;          // X-Axis orientation value (DEGREES)
        int yRot;          // Y-Axis orientation value (DEGREES)
        int zRot;          // Z-Axis orientation value (DEGREES)

        int xPos, yPos, zPos;

        QPoint lastPos;    // The last position the mouse was in (QPoint)
        QColor qtGreen;    // A shortcut to getting a real green
        QColor qtPurple;   // A shortcut to getting a purple
        QColor qtGray;     // A dark grey that will hopefully make a good b-g.
        QColor qtRed;
        QColor qtDark;     // A super-dark gray to simulate a dark room

        bool ambientLight; // Is the [fudged] room light presently on or off?
        bool flashlightOn; // State variable indicating flashlight mode
        bool oppositeOn;   // State of the opposite / complement light

        /*
         * Track the components of the colorable light. These will have to
         * be normalized on a 0 to 1 floating point scale to be useful.
         * (Why? Because QSlider widgets ONLY send int to other slots)
         */
        int auxR, auxG, auxB, auxA;
        GLfloat auxColor[4], axxColor[4];
};

#endif    //_GLWIDGET_H
