/*
 * Filename: asset.cpp
 *
 * Implementation of the code to interface with lib3ds.
 * Original Source: http://www.donkerdump.nl/node/207
 *
 * The texture processing aspects have been adapted from:
 *   http://www.gamedev.net/topic/490141-lib3ds-texture-coordinates-and-vbos/
 */

#include "asset.hpp"
#include <iostream>

Asset3ds::Asset3ds(std::string filename)
{
        // Constructor will immediately try to open the model file
        m_TotalFaces = 0;
        m_model = lib3ds_file_load(filename.c_str());

        if (!m_model) {
                std::cerr << "ERROR: The file name passed was not found.\n";
                throw 1;
        }
}

Asset3ds::~Asset3ds()
{
        // Clean up ALL the OpenGL buffers
        glDeleteBuffers(1, &m_VertexVBO);
        glDeleteBuffers(1, &m_NormalVBO);
        glDeleteBuffers(1, &m_TexCoordVBO);

        if (m_model != NULL) {
                lib3ds_file_free(m_model);
        }
}

void Asset3ds::CreateVBO()
{
        assert(m_model != NULL);

        /*
         * Use helper function to determine the number of faces will be needed
         * What this will do is allow us to make ample space for:
         *   + object vertices
         *   + normal vectors
         *   + texturing coordinates
         */
        GetFaces();
        Lib3dsVector *vertices  = new Lib3dsVector  [m_TotalFaces * 3];
        Lib3dsVector *normals   = new Lib3dsVector  [m_TotalFaces * 3];
        Lib3dsTexel  *texCoords = new Lib3dsTexel   [m_TotalFaces * 3];

        /*
         * We will now iteratively build the entire model's mesh
         */
        Lib3dsMesh * mesh;
        unsigned int FinishedFaces = 0;

        for (mesh = m_model->meshes; mesh != NULL; mesh = mesh->next) {
                lib3ds_mesh_calculate_normals(mesh, &normals[FinishedFaces * 3]);

                /*
                 * Deal with each face and check if there are actual texture
                 * coordinates. The mesh object has a texels member function to
                 * do this check for us so we don't muck up the file parsing.
                 */
                for (unsigned int cur_face = 0;
                     cur_face < mesh->faces;
                     cur_face++) {

                        Lib3dsFace * face = &mesh->faceL[cur_face];
                        for (unsigned int i = 0; i < 3; i++) {

                                // Parse optional texture coordinates
                                if (mesh->texels) {
                                        memcpy( &texCoords[FinishedFaces * 3 + i],
                                                mesh->texelL[face->points[i]],
                                                sizeof(Lib3dsTexel) );
                                }

                                // Parse and copy the object coordinates
                                memcpy( &vertices[FinishedFaces * 3 + i],
                                                mesh->pointL[face->points[i]].pos,
                                                sizeof(Lib3dsVector) );
                        }
                        // Increment the array offset for proper data copying
                        FinishedFaces++;
                }
        }

        /*
         * Now that the vertices have all been copied over from the file, we
         * have to actually generate a Vertex Buffer Object and store it so
         * the GPU has access to it.
         */

        //
        // WHY DOES THIS SEGFAULT!? (-- it doesn't anymore)
        // Weird application states, don't use GLEW, Qt is doing a GREAT
        // amount of work for us as it turns out! (Thanks, Qt, you're amazing)
        //
        glGenBuffers( 1, &m_VertexVBO );
        glBindBuffer( GL_ARRAY_BUFFER, m_VertexVBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof(Lib3dsVector) * 3 * m_TotalFaces,
                        vertices, GL_STATIC_DRAW );

        // Generate another Vertex Buffer Object and store the normals in it
        glGenBuffers( 1, &m_NormalVBO );
        glBindBuffer( GL_ARRAY_BUFFER, m_NormalVBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof(Lib3dsVector) * 3 * m_TotalFaces,
                        normals, GL_STATIC_DRAW );

        // Make a 3rd VBO with the texture coordinates in it
        glGenBuffers( 1, &m_TexCoordVBO );
        glBindBuffer( GL_ARRAY_BUFFER, m_TexCoordVBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof(Lib3dsTexel) * 3 * m_TotalFaces, texCoords, GL_STATIC_DRAW );

        // Clean up our allocated memory
        delete vertices;
        delete normals;
        delete texCoords;

        // We no longer need lib3ds
        lib3ds_file_free( m_model );
        m_model = NULL;
}

void Asset3ds::GetFaces()
{
        assert( m_model != NULL );

        m_TotalFaces = 0;
        Lib3dsMesh * mesh;
        // Loop through every mesh
        for ( mesh = m_model->meshes; mesh != NULL; mesh = mesh->next ) {

                // Add the number of faces this mesh has to the total faces
                m_TotalFaces += mesh->faces;

        }
}

GLuint * Asset3ds::GetTexCoordVBO( void )
{
        // This is a HIDEOUS DISGUSTING EVIL thing to do, we've all come to
        // terms with it and acknowledge it. Sorry C++.
        // Blah blah blah wrong function / class scope
        return &( this->m_TexCoordVBO );
}

void Asset3ds::Draw() const
{
        assert(m_TotalFaces != 0);

        // Enable vertex and normal arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Bind the vbo with the normals
        glBindBuffer(GL_ARRAY_BUFFER, m_NormalVBO);

        // The pointer for the normals is NULL which means that OpenGL will use the currently bound vbo
        glNormalPointer(GL_FLOAT, 0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, m_TexCoordVBO);

        //////////
        //// THIS MIGHT NEED TO BE A 2
        //////////
        glTexCoordPointer(3, GL_FLOAT, 0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, m_VertexVBO);
        glVertexPointer(3, GL_FLOAT, 0, NULL);

        // Render the triangles
        glDrawArrays(GL_TRIANGLES, 0, m_TotalFaces * 3);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
