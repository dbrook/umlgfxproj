/*
 * Filename: asset.hpp
 *
 * This file contains public interfaces to the asset loading code that
 * is going to produce OpenGL scenes from Blender OBJ files.
 *
 * Much of the inspiration for this code comes from:
 * http://www.donkerdump.nl/node/207
 */

#ifndef _ASSET_H
#define _ASSET_H

// Needed to properly get OpenGL initialized for VBO details
// Thanks to:
// http://www.opengl.org/discussion_boards/showthread.php/172481-glGenBuffer-was-not-declared
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>

#include <string>
#include <cstring>
#include <cassert>

class Asset3ds
{
public:
        // Constructor takes the name of the file that will be opened.
        // This MUST be in .3ds format, hence the lib3ds dependency.
        Asset3ds(std::string filename);

        // Draw the scene into the OpenGL framebuffer.
        // This is used in GLWidget::paintGL();
        virtual void Draw() const;

        // Copy the vertices and normals (vectors) into the GPU.
        // This must be done at the frame buffer init.
        virtual void CreateVBO();

        // Returns texture coordinate vertex buffer object
        // This is normally bad practice (to return a pointer to data
        // in another class' scope, but OpenGL being what it is we couldn't
        // come up with another way because IT loves globals so much.
        virtual GLuint *GetTexCoordVBO( void );

        // Destructor. Qt may be nice about its own cleanup, but glDeleteBuffer
        // needs to be called on the private VBO members of this class.
        virtual ~Asset3ds();

protected:
        void GetFaces();                   // internal use
        unsigned int m_TotalFaces;
        Lib3dsFile * m_model;              // a 3ds file pointer (to our model)

        GLuint m_VertexVBO, m_NormalVBO, m_TexCoordVBO;   // vertex and normal buffer objects
};

#endif    // _ASSET_H
