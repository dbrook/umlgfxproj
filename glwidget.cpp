/*
 * Filename: glwidget.cpp
 * 
 * This file is a combination of original work and a template
 * provided from the Qt project maintainers. The project page
 * can be found here:
 *  http://doc.qt.digia.com/qt/opengl-hellogl.html
 * 
 * See LICENSE for details
 *
 * File Description:
 * This file contains (most importantly) the redefined member
 * functions to actually construct an OpenGL scene as well as
 * other Qt-specific bindings for the widget itself.
 */

#include <QtGui>      // Pull in the actual interface to the GUI elems
#include <iostream>
#include <math.h>     // As with any good OpenGL program, there's a 
                      // healthy amount of under-the-hood mathematics!

// [Degrees] to Radians helper function
static float toRadians( float degrees )
{
        return degrees * (M_PI / 180.0);
}

// Project local includes
#include "glwidget.hpp" // grab our GLWidget class
#include "qtlogo.hpp"   // get the Qt framework's logo (to be shown)


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

/*
 * Constructor to setup the scene
 */
GLWidget::GLWidget( QWidget *parent ) :
                QGLWidget( QGLFormat( QGL::SampleBuffers ), parent )
{
        // We'll start WITHOUT a logo first because it's put in by 
        // GLWidget::initializeGL() anyway.
        logo = 0;

        // Attempt to load whatever asset
        asset = new Asset3ds("models/elecloco/Locomotive chs4 072.3DS");


        std::cerr << "Loaded a new asset" << std::endl ;

        // Look dead-on at the scene to start (no initial rotations)
        setXRotation( 0 );
        setYRotation( 0 );
        setZRotation( 0 );

        // Position Offset Initializer
        xPos = yPos = 0;

        // Push model BACK a bit to start. This is important because
        // some are ungodly huge and need to be visible at least in some
        // fashion right away, less the user thinks something failed.
        zPos = -5.0;

        // These colors must be generated via CMYK values otherwise
        // it seems lighting will simply NOT WORK.
        qtGreen = QColor::fromCmykF( 0.2, 0.1, 0.9, 0.3 );
        qtPurple = QColor::fromCmykF( 0.4, 0.4, 0.0, 0.1 );
        qtGray = QColor::fromCmykF( 0.0, 0.0, 0.0, 0.75 );
        qtRed = QColor::fromCmykF( 0.1, 0.7, 0.9, 0.2 );
        qtDark = QColor::fromCmykF( 0.0, 0.0, 0.0, 0.85 );

        // Initially going to say the ambient light is on
        // (as is the movable light)
        ambientLight = true;
        flashlightOn = true;
        oppositeOn   = true;

        // The starting values for the variable color light
        auxRed(10);
        auxGreen(10);
        auxBlue(10);
        
        // Make the default on instantiation be perspective projection
        p_Perspective();
}

/*
 * Destructor (empty since the QWidgets
 * will take care of themselves)
 */
GLWidget::~GLWidget()
{
}

/*
 * At a minimum, we expect a 50 x 50 px size.
 */
QSize GLWidget::minimumSizeHint() const
{
        return QSize( 250, 250 );
}

/*
 * The default size of the widget is to be 400 x 400 px.
 */
QSize GLWidget::sizeHint() const
{
        return QSize( 450, 450 );
}

/*
 * Helper function to normalize rotation angles
 * (basically we don't want these exceeding 359 DEG.)
 */
static void qNormalizeAngle( int &angle )
{
        while (angle < 0)
                angle += 360 * 16;
        while (angle > 360 * 16)
                angle -= 360 * 16;
}

/*
 * These are our SLOTS, so they will be run each time a GUI widget
 * attached to them is changed.
 * EX: Moving a slider widget updates an integer and sends it as
 *     "angle" to these functions, which then mutate the scene.
 */
void GLWidget::setXRotation( int angle )
{
        qNormalizeAngle( angle );        // Helper will fix the angle
        if (angle != xRot) {
                xRot = angle;
                emit xRotationChanged( angle );
                updateGL();              // Given to us from QGLWidget!
        }
}

void GLWidget::setYRotation( int angle )
{
        qNormalizeAngle( angle );        // Helper will fix the angle
        if (angle != yRot) {
                yRot = angle;
                emit yRotationChanged( angle );
                updateGL();              // Given to us from QGLWidget!
        }

}

void GLWidget::setZRotation( int angle )
{
        qNormalizeAngle( angle );        // Helper will fix the angle
        if (angle != zRot) {
                zRot = angle;
                emit zRotationChanged( angle );
                updateGL();              // Given to us from QGLWidget!
        }
}

void GLWidget::forward  ( void )
{

}

void GLWidget::backward ( void )
{

}

void GLWidget::strafeL  ( void )
{

}

void GLWidget::strafeR  ( void )
{

}

void GLWidget::incrElev ( void )
{

}

void GLWidget::decrElev ( void )
{

}

/*
 * Slots for manipulating the light source. This is a pretty straight
 * forward example of a toggling function using a state variable, thus
 * taking the burden of tracking the state to the object itself.
 */
void GLWidget::lightAmbientToggle( void )
{
        if (ambientLight) {
                // Light is presently on, so turn it off when called
                glDisable( GL_LIGHT0 );

                // Change the background to be darker w/o ambient light
                qglClearColor( qtDark.dark() );
                ambientLight = false;

        } else {
                // Light is off when called, so turn it back on
                glEnable( GL_LIGHT0 );

                qglClearColor( qtGray.light() );
                ambientLight = true;
        }
        updateGL();                // Commit the change to the scene
}

/*
 * Turn the auxiliary light on or off. Works (almost) exactly the same
 * as the ambient (room) light toggle switch but doesn't change the
 * background to "trick" the eye into seeing the "room" illuminated.
 */
void GLWidget::auxLightToggle( void )
{
        if (flashlightOn) {
                // Light is presently on, so turn it off
                glDisable( GL_LIGHT1 );
                flashlightOn = false;
        } else {
                // Light is presently off, so turn it on
                glEnable( GL_LIGHT1 );
                flashlightOn = true;
        }
        updateGL();                // Force the change to the scene
}

void GLWidget::oppositeLightToggle( void )
{
        if (oppositeOn) {
                glDisable( GL_LIGHT2 );
                oppositeOn = false;
        } else {
                glEnable( GL_LIGHT2 );
                oppositeOn = true;
        }
        updateGL();
}


/*
 * Auxiliary Light color component functions.
 */
void GLWidget::auxRed( int userRed )
{
        auxR = userRed;
        auxColor[0] = (float) auxR / 100.0;
        axxColor[0] = (float) ((auxR - 100.0) * -1) / 100.0;
        updateGL();
}

void GLWidget::auxGreen( int userGreen )
{
        auxG = userGreen;
        auxColor[1] = (float) auxG / 100.0;
        axxColor[1] = (float) ((auxG - 100.0) * -1) / 100.0;
        updateGL();
}

void GLWidget::auxBlue( int userBlue )
{
        auxB = userBlue;
        auxColor[2] = (float) auxB / 100.0;
        axxColor[2] = (float) ((auxB - 100.0) * -1) / 100.0;
        updateGL();
}

void GLWidget::auxAlpha( int userAlpha )
{
        auxA = userAlpha;
        auxColor[3] = (float) auxA / 100.0;
        axxColor[3] = (float) (auxA - 500.0) * -1;
        updateGL();
}

/* 
 * Turn on perspective projection (this uses predefined stuff in the
 * resize protected function to work ... static stuff)
 */
void GLWidget::p_Perspective( void )
{
        perspectiveMode = true;
        
        // Evil way of forcing the redraw to happen
        resizeGL( this->width(), this->height() );
        updateGL();
}

/* 
 * Similar to the above function, but for orthographic projection
 */
void GLWidget::p_Orthographic( void )
{
        perspectiveMode = false;

        // Evil way of forcing the redraw to happen
        resizeGL( this->width(), this->height() );
        updateGL();
}


/*
 * These functions will closely mimic those which we've seen already
 * w.r.t. the GLUT functions. In fact, Qt actually implements GLUT,
 * so the coding is frighteningly similar.
 */
void GLWidget::initializeGL()
{
        // The background color is set here.
        //qglClearColor( qtPurple.dark() );
        qglClearColor( qtGray.light() );

/*
        // Make a QtLogo (this has been provided to us), (make it green!)
        logo = new QtLogo( this, 64 );
        logo->setColor( qtPurple.dark() );
*/

        glEnable( GL_DEPTH_TEST );
        glEnable( GL_CULL_FACE );
        glShadeModel( GL_SMOOTH );

        // Need these options to enable light sources
        // (can do GL_LIGHT0, 1, ..., n sources)
        glEnable( GL_LIGHTING );
        glEnable( GL_LIGHT0 );            // The stationary "room" light
        glEnable( GL_LIGHT1 );            // The recoloarable light
        glEnable( GL_LIGHT2 );            // Another light (why not?)
        glEnable( GL_MULTISAMPLE );       // Use hardware to smooth edges!
                                          // (may not work on all platforms)

        // Old light setting (in case we want it back)
        //static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 7.0 };

        // Setup the room's light position. This will be a constant.
        static GLfloat lightPosition[4] = { 0.0, 0.9, 0.5, 1.0 };
        glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );

        // Initialize the side lights
        static GLfloat flPos[4] = { 1.0, 0.0, 0.1, 0.0 };
        glLightfv( GL_LIGHT1, GL_POSITION, flPos );

        static GLfloat llPos[4] = { -1.0, 0.0, 0.1, 0.0 };
        glLightfv( GL_LIGHT2, GL_POSITION, llPos );

        // Assuring we're in the proper context to continue to create a VBO


        // Create the vertex buffer array with the object!
        // NOTE: This fails unless you have the proper context first.

        std::cerr << "Attempting to create the VBO!" << std::endl;
        asset->CreateVBO();
}

// Basically the redraw call back from GLUT
void GLWidget::paintGL()
{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glLoadIdentity();

        glTranslatef( 0.0, 0.0, zPos );
        glRotatef( xRot / 16.0, 1.0, 0.0, 0.0 );
        glRotatef( yRot / 16.0, 0.0, 1.0, 0.0 );
        glRotatef( zRot / 16.0, 0.0, 0.0, 1.0 );

//        glTranslatef( -xPos, -yPos, -zPos );

        glLightfv( GL_LIGHT1, GL_DIFFUSE, auxColor );
        glLightfv( GL_LIGHT2, GL_DIFFUSE, axxColor );
/*
        // Have the logo redraw itself based on the new rotations!
        logo->draw();
 */

        // Have the asset redraw!
        asset->Draw();
}

/*
 * The window resized callback
 *
 * This is made to properly handle the rendering of a scene based off 
 * of the QGLWidget's size parameters. (Called whenever dimensions change)
 */
void GLWidget::resizeGL( int width, int height )
{
        int side = qMin( width, height );

        // Setting up the viewport
        glViewport( (width - side) / 2, (height - side) / 2, side, side );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        /*
         * For perspective projection, since we aren't using GLUT, we need
         * to do a conversion.  
         * 
         * This site helped:
         
        http://jleland.blogspot.com/2008/12/gluperspective-to-glfrustrumf.html

        */
        GLfloat top, bottom, left, right;
        GLfloat fov = 40, near = 0.1, far = 100.0;
        
        if (perspectiveMode) {
//                GLfloat aspect = (GLfloat) width / (GLfloat) height;
                // Always make aspect ratio 0 to avoid nasty looking resizes
                GLfloat aspect = 1.0;
                top = tan( toRadians( fov ) ) * near;
                bottom = -top;
                left = aspect * bottom;
                right = aspect * top;
                
                // See if we're running OpenGL ES and use the proper function
#ifdef QT_OPENGL_ES_1
                glFrustumf( left, right, bottom, top, near, far );
#else
                glFrustum( left, right, bottom, top, near, far );
#endif
                
        } else {
                // See if we're running OpenGL ES and use the proper function
#ifdef QT_OPENGL_ES_1
                glOrthof( -2.0, 2.0, -2.0, 2.0, near, far );
#else
                glOrtho( -2.0, 2.0, -2.0, 2.0, near, far );
#endif
        }
        glMatrixMode( GL_MODELVIEW );
}

/*
 * Mouse clicked handler:
 *  This will update the lastPos object with the place JUST clicked.
 */
void GLWidget::mousePressEvent( QMouseEvent *event )
{
        lastPos = event->pos();
}

/*
 * Mouse displacement handler (within the framebuffer)
 *
 * Mouse rotation is modified X/Y on left-click, X/Z on right-click
 * (user must hold down the button to acheive this behavior)
 */
void GLWidget::mouseMoveEvent( QMouseEvent *event )
{
        int dx = event->x() - lastPos.x();
        int dy = event->y() - lastPos.y();

        if (event->buttons() & Qt::LeftButton) {
                setXRotation( xRot + 8 * dy );
                setYRotation( yRot + 8 * dx );
        }
        /*else {
                setXRotation( xRot + 8 * dy );
                setZRotation( zRot + 8 * dx );
        }*/

        // Update lastPos with the new position so subsequent moves
        // will be handled based off of the present location.
        lastPos = event->pos();
}
