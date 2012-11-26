/*
 * Filename: window.cpp
 * 
 * This file is a combination of original work and a template
 * provided from the Qt project maintainers. The project page
 * can be found here:
 *  http://doc.qt.digia.com/qt/opengl-hellogl.html
 * 
 * See LICENSE for details
 *
 * File Description:
 * Code to connect signals/slots in the main window's GUI as well
 * as setup the widget layout of said main window.
 */


#include <QtGui>
#include "glwidget.hpp"   // Local project include
#include "window.hpp"     // Local project include

/*
 * Default (and only) constructor to setup layout and signals/slots.
 */
Window::Window()
{
        // Construct an instance of the main window.
        glWidget = new GLWidget;

        // Place and initialize the delicious GUI elements
        xSlider = createSlider();
        ySlider = createSlider();
        zSlider = createSlider();

        /*
         * Interconnect the changes of the sliders to our GLWidget
         * 
         * Connecting _valueChanged signals that come from the sliders to the
         * appropriate slots we created in the glWidget. Therefore, users are
         * able to change the orientation of the object by dragging sliders.
         *
         * Because we bind them both ways, however ... any changed made in
         * the frame buffer is reflected on the sliders, too.
         */
        connect( xSlider, SIGNAL(valueChanged(int)),
                        glWidget, SLOT(setXRotation(int)) );

        connect( glWidget, SIGNAL(xRotationChanged(int)),
                        xSlider, SLOT(setValue(int)) );

        connect( ySlider, SIGNAL(valueChanged(int)),
                        glWidget, SLOT(setYRotation(int)) );

        connect( glWidget, SIGNAL(yRotationChanged(int)),
                        ySlider, SLOT(setValue(int)) );

        connect( zSlider, SIGNAL(valueChanged(int)),
                        glWidget, SLOT(setZRotation(int)) );

        connect( glWidget, SIGNAL(zRotationChanged(int)),
                        zSlider, SLOT(setValue(int)) );

        // Labels will help the user see what is being changed
        QLabel *xRotLabel = new QLabel( "Around X Axis" );
        QLabel *yRotLabel = new QLabel( "Around Y Axis" );
        QLabel *zRotLabel = new QLabel( "Around Z Axis" );

        // Place a couple of main layouts (GL frame buffer, control area)
        QHBoxLayout *mainLayout = new QHBoxLayout;
        mainLayout->addWidget( glWidget );

        // Make a group area for the rotation functions
        QVBoxLayout *mainControls = new QVBoxLayout;

        // (make the controls not span vertically)
        mainControls->setAlignment( Qt::AlignTop );

        mainLayout->addLayout( mainControls );
        QGroupBox *scRotation = new QGroupBox( "Scene Rotation" );
        scRotation->setAlignment( Qt::AlignHCenter );
        mainControls->addWidget( scRotation );

        // Populate the rotation controllers
        QVBoxLayout *rotLayout = new QVBoxLayout;
        scRotation->setLayout( rotLayout );       // Need to make a layout WITHIN the groupbox

        rotLayout->addWidget( xRotLabel );
        rotLayout->addWidget( xSlider );
        rotLayout->addWidget( yRotLabel );
        rotLayout->addWidget( ySlider );
        rotLayout->addWidget( zRotLabel );
        rotLayout->addWidget( zSlider );


        /*
         * Now set up a group box for handling all the lighting needs
         */
        QGroupBox *scLighting = new QGroupBox( "Scene Lighting" );
        scLighting->setAlignment( Qt::AlignHCenter );
        mainControls->addWidget( scLighting );
        QVBoxLayout *lightingLayout = new QVBoxLayout;
        scLighting->setLayout( lightingLayout );

        // Room light toggle operation
        roomLightSwitch = new QPushButton( "Room Light Master" );
        lightingLayout->addWidget( roomLightSwitch );
        connect( roomLightSwitch, SIGNAL(clicked()),
                        glWidget, SLOT(lightAmbientToggle(void)) );

        // Right hand supplemental light toggle
        auxiliarySwitch = new QPushButton( "Right Light Master" );
        lightingLayout->addWidget( auxiliarySwitch );
        connect( auxiliarySwitch, SIGNAL(clicked()),
                        glWidget, SLOT(auxLightToggle(void)) );

        // Left hand opposite light toggle
        oppositeSwitch = new QPushButton( "Left Light Master" );
        lightingLayout->addWidget( oppositeSwitch );
        connect( oppositeSwitch, SIGNAL(clicked()),
                        glWidget, SLOT(oppositeLightToggle(void)) );

        // Flashlight position sliders
        // Need some labels so people know what variables are being changed
        QLabel *redLabel = new QLabel( "Red" );
        QLabel *grnLabel = new QLabel( "Green" );
        QLabel *bluLabel = new QLabel( "Blue" );

        redSlider = unitSlider();
        bluSlider = unitSlider();
        grnSlider = unitSlider();
        alpSlider = unitSlider();

        lightingLayout->addWidget( redLabel );
        lightingLayout->addWidget( redSlider );
        lightingLayout->addWidget( grnLabel );
        lightingLayout->addWidget( grnSlider );
        lightingLayout->addWidget( bluLabel );
        lightingLayout->addWidget( bluSlider );

        connect( redSlider,  SIGNAL(valueChanged(int)),
                 glWidget,   SLOT(auxRed(int)) );
        connect( grnSlider,  SIGNAL(valueChanged(int)),
                 glWidget,   SLOT(auxGreen(int)) );
        connect( bluSlider,  SIGNAL(valueChanged(int)),
                 glWidget,   SLOT(auxBlue(int)) );
        connect( alpSlider,  SIGNAL(valueChanged(int)),
                 glWidget,   SLOT(auxAlpha(int)) );


        // VERY IMPORTANT: Make it so only the GL frame buffer expands
        // (the layout will NOT scale in a visually appealing way
        // if this is not set this way).
        mainLayout->setStretch( 0, 1 );

        setLayout( mainLayout );            // Commit the layout of widgets

        // Initialize values of the sliders
        ////// NOTE:: This will override the initialize GL camera setting /////
        xSlider->setValue( 30 * 16 );
        ySlider->setValue( 345 * 16 );
        zSlider->setValue( 345 * 16 );
        redSlider->setValue( 100 );
        grnSlider->setValue(  10 );
        bluSlider->setValue(  10 );

        // Give the main window a helpful but simple title
        setWindowTitle( tr( "Qt/OpenGL Multiple Light Source Test Interface" ) );
}

/* 
 * A helper function to streamline the creation of the sliders.
 * (ensures suitable ranges, stepping-values, tick intervals, and page steps)
 */
QSlider *Window::createSlider()
{
        // Make a slider object oriented vertically.
        QSlider *slider = new QSlider( Qt::Horizontal );

        slider->setRange( 0, 360 * 16 );
        slider->setSingleStep( 16 );
        slider->setPageStep( 15 * 16 );
        slider->setTickInterval( 30 * 16 );

        // Tick-marks should be set to show which field they correspond do
        slider->setTickPosition( QSlider::TicksAbove );
        return slider;
}

/*
 * A helper function to streamline the creation of sliders
 * that will cover 100 units of space. The down-side here is
 * that the integer values of the slider must be mapped to
 * the OpenGL frame buffer.
 *
 * NOTE: ONLY allowing the slider to go to 1 and not zero.
 * This was because of a weird bug where the scene would
 * get painted by the pointed light source if at any point
 * all the color field sliders went to 0 and were brought back.
 */
QSlider *Window::unitSlider( void )
{
        QSlider *slider = new QSlider( Qt::Horizontal );
        slider->setRange( 1, 500 );
        slider->setSingleStep( 5 );
        slider->setPageStep( 10 );
        slider->setTickInterval( 50 );
        slider->setTickPosition( QSlider::TicksAbove );
        return slider;
}

/*
 * Exit on an escape ... ignore all others
 */
void Window::keyPressEvent( QKeyEvent *e )
{
        // Typical WASD controls for the scene
        if (e->key() == Qt::Key_W)
                glWidget->forward();
        else if (e->key() == Qt::Key_S)
                glWidget->backward();
        else if (e->key() == Qt::Key_A)
                glWidget->strafeL();
        else if (e->key() == Qt::Key_D)
                glWidget->strafeR();
        else if (e->key() == Qt::Key_E)
                glWidget->incrElev();
        else if (e->key() == Qt::Key_Z)
                glWidget->decrElev();

        if (e->key() == Qt::Key_Escape)
                close();
        else
                QWidget::keyPressEvent( e );
}
