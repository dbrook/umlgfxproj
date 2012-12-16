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

        /*
         * Two direct layouts will be present, the first is OpenGL,
         * and the second is just a vertical bar of all other controls.
         * The controls SHOULD NOT stretch as the window is resized, but
         * the OpenGL widget is allowed to.
         */
        QHBoxLayout *mainLayout = new QHBoxLayout;
        mainLayout->addWidget( glWidget );
        QVBoxLayout *mainControls = new QVBoxLayout;
        mainControls->setAlignment( Qt::AlignTop );
        mainLayout->addLayout( mainControls );

        /*
         * Scene rotation controls can now be added as children
         */
        QGroupBox *scRotation = new QGroupBox( "Scene Rotation" );
        scRotation->setAlignment( Qt::AlignHCenter );
        mainControls->addWidget( scRotation );

        // Helper function assists in making consistent sliders
        xSlider = createSlider();
        ySlider = createSlider();
        zSlider = createSlider();

        // Labels will help the user see what is being changed
        QLabel *xRotLabel = new QLabel( "Around X Axis" );
        QLabel *yRotLabel = new QLabel( "Around Y Axis" );
        QLabel *zRotLabel = new QLabel( "Around Z Axis" );

        // Populate the rotation controllers. Within EACH AND EVERY 
        // QGroupBox we need another layout. The LAYOUT is the actual
        // parent to all widgets within. This makes life a bit easier.
        QVBoxLayout *rotLayout = new QVBoxLayout;
        scRotation->setLayout( rotLayout );
        rotLayout->addWidget( xRotLabel );
        rotLayout->addWidget( xSlider );
        rotLayout->addWidget( yRotLabel );
        rotLayout->addWidget( ySlider );
        rotLayout->addWidget( zRotLabel );
        rotLayout->addWidget( zSlider );
        
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

        /*
         * NEW FEATURE!
         * A new group to handle different perspective modes
         */
        QGroupBox *projections = new QGroupBox( "Projection Mode" );
        projections->setAlignment( Qt::AlignHCenter );
        mainControls->addWidget( projections );
        QVBoxLayout *projLayout = new QVBoxLayout;
        projections->setLayout( projLayout );
        
        p_pers = new QRadioButton( "Perspective" );
        p_orth = new QRadioButton( "Orthographic" );
        projLayout->addWidget( p_pers );
        projLayout->addWidget( p_orth );
        
        // Connect to the right slots
        connect( p_pers, SIGNAL(clicked()),
                 glWidget, SLOT(p_Perspective()) );
        connect( p_orth, SIGNAL(clicked()),
                 glWidget, SLOT(p_Orthographic()) );
        
        // Make sure the perspective mode is checked
        p_pers->setChecked( true );

        /*
         * Now set up a group box for handling all the lighting needs
         */
        QGroupBox *scLighting = new QGroupBox( "Scene Lighting" );
        scLighting->setAlignment( Qt::AlignHCenter );
        mainControls->addWidget( scLighting );
        QVBoxLayout *lightingLayout = new QVBoxLayout;
        scLighting->setLayout( lightingLayout );

        // Room light toggle operation
        roomLightSwitch = new QPushButton( "Room Light" );
        lightingLayout->addWidget( roomLightSwitch );
        connect( roomLightSwitch, SIGNAL(clicked()),
                        glWidget, SLOT(lightAmbientToggle(void)) );

        // Right hand supplemental light toggle
        auxiliarySwitch = new QPushButton( "Right Light" );
        lightingLayout->addWidget( auxiliarySwitch );
        connect( auxiliarySwitch, SIGNAL(clicked()),
                        glWidget, SLOT(auxLightToggle(void)) );

        // Left hand opposite light toggle
        oppositeSwitch = new QPushButton( "Left Light" );
        lightingLayout->addWidget( oppositeSwitch );
        connect( oppositeSwitch, SIGNAL(clicked()),
                        glWidget, SLOT(oppositeLightToggle(void)) );

        // Flashlight position sliders
        // Need some labels so people know what variables are being changed
        QLabel *redLabel = new QLabel( "Red" );
        QLabel *grnLabel = new QLabel( "Green" );
        QLabel *bluLabel = new QLabel( "Blue" );

        // used another helper function here to make these sliders easier
        redSlider = unitSlider();
        bluSlider = unitSlider();
        grnSlider = unitSlider();

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

        /*
         * VERY IMPORTANT: 
         * Make it so only the GL frame buffer expands (the layout will NOT
         * scale in a visually appealing way if this is not set this way).
         */
        mainLayout->setStretch( 0, 1 );

        setLayout( mainLayout );            // Commit the layout of widgets

        /*
         * Initialize values of the sliders
         * VERY IMPORTANT! This will override whatever the glwidget has in
         * its constructor for initial parameters because the act of doing
         * a setValue() on them sends a signal to the glwidget as a result
         * of the connect() earlier. 
         * There's also no error checking implemented in the sliders so
         * do NOT set them out of bounds in the unitSlider() helper function.
         */
        xSlider->setValue( 30 * 16 );
        ySlider->setValue( 345 * 16 );
        zSlider->setValue( 345 * 16 );
        redSlider->setValue( 200 );
        grnSlider->setValue(  10 );
        bluSlider->setValue(  10 );

        // Give the main window a helpful but simple title
        setWindowTitle( tr( "Qt/OpenGL Final Project" ) );
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

        // Things go REALLY weird if these sliders are allowed
        // to span the full range of RGB color values as processed
        // within GLWidget::aux[colorName], so make sure the sliders
        // don't allow it to happen.
        slider->setRange( 1, 99 );
        slider->setSingleStep( 2 );
        slider->setPageStep( 10 );
        slider->setTickInterval( 20 );
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
