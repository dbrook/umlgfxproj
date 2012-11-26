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
#include <QtOpenGL>   // Pull in the Qt-abstracted OpenGL interface
#include <math.h>     // As with any good OpenGL program, there's a 
                      // healthy amount of under-the-hood mathematics!



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

        // Look dead-on at the scene to start (no initial rotations)
        xRot = 0;
        yRot = 0;
        zRot = 0;

        // Position Offset Initializer
        xPos = yPos = zPos = 0;

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
        auxR = auxG = auxB = auxA = 0;
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
        return QSize( 50, 50 );
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
                emit ambientLightChanged( false );

        } else {
                // Light is off when called, so turn it back on
                glEnable( GL_LIGHT0 );

                qglClearColor( qtGray.light() );
                ambientLight = true;
                emit ambientLightChanged( true );
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
        updateGL();                // Commit the change to the scene
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
        axxColor[0] = (float) ((auxR - 500.0) * -1) / 100.0;
        updateGL();
}

void GLWidget::auxGreen( int userGreen )
{
        auxG = userGreen;
        auxColor[1] = (float) auxG / 100.0;
        axxColor[1] = (float) ((auxG - 500.0) * -1) / 100.0;
        updateGL();
}

void GLWidget::auxBlue( int userBlue )
{
        auxB = userBlue;
        auxColor[2] = (float) auxB / 100.0;
        axxColor[2] = (float) ((auxB - 500.0) * -1) / 100.0;
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
 * These functions will closely mimic those which we've seen already
 * w.r.t. the GLUT functions. In fact, Qt actually implements GLUT,
 * so the coding is frighteningly similar.
 */
void GLWidget::initializeGL()
{
        // The background color is set here.
        //qglClearColor( qtPurple.dark() );
        qglClearColor( qtGray.light() );

        // Make a QtLogo (this has been provided to us), (make it green!)
        logo = new QtLogo( this, 64 );
        logo->setColor( qtPurple.dark() );

//        glutSolidTeapot( 2.0 );

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

        // Initialize the movable light
        static GLfloat flPos[4] = { 1.0, 0.0, 0.1, 0.0 };
        glLightfv( GL_LIGHT1, GL_POSITION, flPos );

        static GLfloat llPos[4] = { -1.0, 0.0, 0.1, 0.0 };
        glLightfv( GL_LIGHT2, GL_POSITION, llPos );

        /*
         * Commented-out because colors are now individually
         * controlled via the aux_ slots.
         */
        //static GLfloat flCol[4] = { 0.0, 0.8, 0.0, 1.0 };
        //glLightfv( GL_LIGHT1, GL_DIFFUSE, flCol );
}

// Basically the redraw call back from GLUT
void GLWidget::paintGL()
{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glLoadIdentity();

        glTranslatef( 0.0, 0.0, -10.0 );        // Need -10 to be visible!
        glRotatef( xRot / 16.0, 1.0, 0.0, 0.0 );
        glRotatef( yRot / 16.0, 0.0, 1.0, 0.0 );
        glRotatef( zRot / 16.0, 0.0, 0.0, 1.0 );

        glTranslatef( -xPos, -yPos, -zPos );

        glLightfv( GL_LIGHT1, GL_DIFFUSE, auxColor );
        glLightfv( GL_LIGHT2, GL_DIFFUSE, axxColor );

        // Have the logo redraw itself based on the new rotations!
        logo->draw();
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

        // Ugh. gotta remember to make sure I'm spelling things right.
        // "side" != "size"
        // Setting up the viewport
        glViewport( (width - side) / 2, (height - side) / 2, side, side );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();

        // Seems this does some Qt-system-specific checks first.
#ifdef QT_OPENGL_ES_1
        glOrthof( -0.5, +0.5, -0.5, +0.5, 4.0, 15.0 );
#else
        glOrtho( -0.5, +0.5, -0.5, +0.5, 4.0, 15.0 );
#endif
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
