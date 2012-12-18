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

// Texturing is very wonky, and calls / relevant code to texturing can
// be disabled by setting this preprocessor directive to 0.
#define TEXTURE_MODE_ON 0

#include <QtGui>      // Pull in the actual interface to the GUI elems
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

        ///////////////////////////////////////
        // Attempt to load whatever asset
        ///////////////////////////////////////
        QStringList args = QCoreApplication::arguments();

        // Pure AWESOME cascading member function calls! WHeeeeeeeeee
        char *assetName = args.at(1).toLocal8Bit().data();

        asset = new Asset3ds(assetName);

        // Look dead-on at the scene to start (no initial rotations)
        // WARNING: This is overruled by the slider settings in window.cpp!!
        setXRotation( 0 );
        setYRotation( 0 );
        setZRotation( 0 );

        // Position Offset Initializer
        xPos = yPos = 0;

        // Push model BACK a bit to start. This is important because
        // some are ungodly huge and need to be visible at least in some
        // fashion right away, less the user thinks something failed.
        zPos = -20.0;

        // Initialize scaling factor to 1, no scale change
        scaleFactor = 1.0;

        // Initialize motion state bools
        moveUp_ = moveDn_ = moveRight_ = moveLeft_ = false;

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

        // Default size of orthographic projection mode
        // (These are changed with the mouse wheel when in PROJECTION).
        ortho_left = ortho_top = -3.0;
        ortho_bottom = ortho_right = 3.0;

        // Begin the updating of the GLwidget using the QTimerEvent here
        startTimer( 20 );
}

/*
 * Destructor (empty since the QWidgets
 * will take care of themselves)
 */
GLWidget::~GLWidget()
{
}

/*
 * At a minimum, we expect a 250 x 250 px size.
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
        return QSize( 650, 650 );
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

void GLWidget::forward  ( float amount )
{
        if (!perspectiveMode) {
                ortho_bottom -= amount;
                ortho_top    += amount;
                ortho_left   += amount;
                ortho_right  -= amount;
        } else {
                zPos += amount;
        }
        // Evil way of forcing the redraw to happen
        resizeGL( this->width(), this->height() );
        updateGL();
}

void GLWidget::backward ( float amount )
{
        if (!perspectiveMode) {
                ortho_bottom += amount;
                ortho_top    -= amount;
                ortho_left   -= amount;
                ortho_right  += amount;
        } else {
                zPos -= amount;
        }
        // Evil way of forcing the redraw to happen
        resizeGL( this->width(), this->height() );
        updateGL();
}

void GLWidget::strafeL  ( bool on )
{
        moveLeft_ = on;
}

void GLWidget::strafeR  ( bool on )
{
        moveRight_ = on;
}

void GLWidget::incrElev ( bool on )
{
        moveUp_ = on;
}

void GLWidget::decrElev ( bool on )
{
        moveDn_ = on;
}

/*
 * Panning functions for the scene's viewport
 */
void GLWidget::panHorizontal( int direction )
{
        if (direction < 0)       xPos -= 0.5;
        else                     xPos += 0.5;
        updateGL();
}

void GLWidget::panVertical( int direction )
{
        if (direction < 0)       yPos -= 0.5;
        else                     yPos += 0.5;
        updateGL();
}

void GLWidget::masterReset( void )
{
        xPos = yPos = 0.0;
        zPos = -20.0;

        ortho_bottom = ortho_right = 5.0;
        ortho_top = ortho_left = -5.0;

        xRot = yRot = zRot = 0.0;

        resizeGL( this->width(), this->height() );
        updateGL();
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

                qglClearColor( qtDark.light() );
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
 * Change the scaling factor of the model.
 * Sometimes models are too big (ugh, like the house one we tried).
 * Pushing it back didn't really seem like an elegant solution, so
 * this lets the user DIRECTLY set the scaling factor and force a
 * redraw of the scene.
 */
void GLWidget::setScaling( double usrFactor )
{
        // Qt provides a double spin box. But we want a float! Bah!

        scaleFactor = (float) usrFactor;

        updateGL();
}


//////////////////////////////////////////////////////////////////////////////
//  Qt OpenGL Base Fundamental functions
//////////////////////////////////////////////////////////////////////////////

/*
 * These functions will closely mimic those which we've seen already
 * w.r.t. the GLUT functions. In fact, Qt actually implements GLUT,
 * so the coding is frighteningly similar.
 */
void GLWidget::initializeGL()
{
        // The background color is set here.
        //qglClearColor( qtPurple.dark() );
        qglClearColor( qtDark.light() );

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
        //static GLfloat lightPosition[4] = { 0.0, 0.9, 0.5, 1.0 };
        static GLfloat lightPosition[4] = { 0.0, 0.0, 1000.0, 1.0 };
        glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );

        // Initialize the side lights
        static GLfloat flPos[4] = { 1000.0, 0.0, 0.1, 0.0 };
        glLightfv( GL_LIGHT1, GL_POSITION, flPos );

        static GLfloat llPos[4] = { -1000.0, 0.0, 0.1, 0.0 };
        glLightfv( GL_LIGHT2, GL_POSITION, llPos );

        // Create the vertex buffer array with the object!
        // NOTE: This fails unless you have the proper context first.

        asset->CreateVBO();

#if TEXTURE_MODE_ON
        // The texture loading interface is very wonky still and
        // needs more debugging time. We've removed all calls and
        // interaction with the outside files that deal with them ... for now.
        loadGLTextures();
        glBindTexture(GL_TEXTURE_2D, *(asset->GetTexCoordVBO()));
#endif
}

// Basically the redraw call back from GLUT
void GLWidget::paintGL()
{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glLoadIdentity();

        glTranslatef( xPos, yPos, zPos );
        glRotatef( xRot / 16.0, 1.0, 0.0, 0.0 );
        glRotatef( yRot / 16.0, 0.0, 1.0, 0.0 );
        glRotatef( zRot / 16.0, 0.0, 0.0, 1.0 );

        // Now we can scale the scene we load, in case it's huge/tiny.
        glScalef( scaleFactor, scaleFactor, scaleFactor );

        glLightfv( GL_LIGHT1, GL_DIFFUSE, auxColor );
        glLightfv( GL_LIGHT2, GL_DIFFUSE, axxColor );

/*
        // Have the logo redraw itself based on the new rotations!
        logo->draw();
 */

#if TEXTURE_MODE_ON
        // This is where we'll put the textures on
        glEnable(GL_TEXTURE_2D);
#endif
        // Have the asset redraw!
        asset->Draw();
#if TEXTURE_MODE_ON
        // Reset the texture state
        glDisable(GL_TEXTURE_2D);
#endif
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
        GLfloat fov = 40, near = 0.1, far = 10000.0;
        
        if (perspectiveMode) {
//                GLfloat aspect = (GLfloat) height / (GLfloat) width;
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
                glOrthof( ortho_left, ortho_right, ortho_top, ortho_bottom, near, far );
#else
                glOrtho( ortho_left, ortho_right, ortho_top, ortho_bottom, near, far );
#endif
        }
        glMatrixMode( GL_MODELVIEW );
}

// Function is adapted from:
//   http://stackoverflow.com/questions/10684705/texture-loading-with-opengl-in-qt
void GLWidget::loadGLTextures()
{
        QImage t;
        QImage b;

        if ( !b.load( "models/iphone/Textures/Iphone_texture.jpg" ) )
        {
                qDebug("Unable to open the image.");
                b = QImage( 16, 16, QImage::Format_RGB32 );
                b.fill( 1 );
        }

        t = QGLWidget::convertToGLFormat( b );
        glGenTextures( 1, asset->GetTexCoordVBO() );
        glBindTexture( GL_TEXTURE_2D, *(asset->GetTexCoordVBO()) );

        glTexImage2D( GL_TEXTURE_2D, 0, 3, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
//        glTexImage2D( GL_TEXTURE_2D, 0, 3, t.height(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

/*
 * The time event will keep looking for updates to the motion key statuses
 */
void GLWidget::timerEvent( QTimerEvent *timer )
{
        /*
         * This area will control all possible directions of camera movement
         */
        if (moveUp_) {
                panVertical(-5.0);
        } else if (moveDn_) {
                panVertical(5.0);
        } else if (moveLeft_) {
                panHorizontal(5.0);
        } else if (moveRight_){
                panHorizontal(-5.0);
        }
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
        else {
                setXRotation( xRot + 8 * dy );
                setZRotation( zRot + 8 * dx );
        }

        // Update lastPos with the new position so subsequent moves
        // will be handled based off of the present location.
        lastPos = event->pos();
}
