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

#include "asset.hpp"   // Our new magical asset loading tool
#include <QGLWidget>   // The OpenGL "canvas" of sorts
#include <string>

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

        // Timing for smooth scene motion
        void timerEvent( QTimerEvent *timer );

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

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
         * Change scene rotation and alignment via these slots
         */
        void setXRotation( int angle );
        void setYRotation( int angle );
        void setZRotation( int angle );
        void forward  ( float amount );
        void backward ( float amount );
        void strafeL  ( bool );
        void strafeR  ( bool );
        void incrElev ( bool );
        void decrElev ( bool );

        // Panning offsets: direction is negative to move viewport LEFT / DOWN
        //                               positive to move viewport RIGHT / UP
        void panHorizontal( int direction );
        void panVertical( int direction );

        /*
         * Control function to emulate the room light being switched
         * on or off. This is a little bit of a "cheat" because while
         * we can easily turn the light sources on or off individually,
         * the background color is NOT impacted at all by this, so
         * this slot will also fudge the background to make it darker.
         */
        void lightAmbientToggle( void );
        void oppositeLightToggle( void );
        void auxLightToggle( void );

        /*
         * Slots to adjust color components manually
         */
        void auxRed   ( int userRed );
        void auxGreen ( int userGreen );
        void auxBlue  ( int userBlue );
        void auxAlpha ( int userAlpha );
        
        /* 
         * Slots to change the projection mode
         */
        void p_Perspective( );
        void p_Orthographic( );

        // Realigns the scene and viewing window frame parameters
        // (helpful if you've zoomed off to nowheresville.
        void masterReset( void );
        
signals:
        /*
         * These signals get emitted so the GUI widgets can reflect
         * the change in scene rotation parameters (because the user
         * used the mouse to change the scene, for example).
         */
        void xRotationChanged( int angle );
        void yRotationChanged( int angle );
        void zRotationChanged( int angle );

protected:
        /*
         * IMPORTANT:
         * Initializing the OpenGL systems, resizing of the viewport,
         * and actually drawing the scenes are handled by redefining
         * the skeleton functions given by QGLWidget (the framework's
         * copy of it is essentially empty and we put our OpenGL code
         * in these by simply redefining the member functions).
         * The bulk of the regular OpenGL code like we've seen in the
         * Angel code is done in these functions.
         */
        void initializeGL();
        void paintGL();
        void resizeGL( int width, int height );  // Called on every resize

        // Function to load the textures (they will be "placed" by the asset handler)
        void loadGLTextures( void );

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
        Asset3ds *asset;   // Our new magic asset (must be a 3ds file)

        int xRot;          // X-Axis orientation value (DEGREES)
        int yRot;          // Y-Axis orientation value (DEGREES)
        int zRot;          // Z-Axis orientation value (DEGREES)

        // For camera positions. May be deprecated in favor of a
        // QVector in the near/immediate future.
        GLfloat xPos, yPos, zPos;

        // State flags to determine if the scene needs moving
        bool moveUp_, moveDn_, moveRight_, moveLeft_;

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
         * Track the components of the colorable light. Why INTs? Because
         * QSlider widgets ONLY send int to other slots.
         */
        int auxR, auxG, auxB, auxA;
        GLfloat auxColor[4], axxColor[4];
        
        bool perspectiveMode;  // Are we in perspective mode? TRUE
                               // or orthographic mode? FALSE

        GLfloat ortho_left, ortho_right, ortho_top, ortho_bottom;
};

#endif    //_GLWIDGET_H
