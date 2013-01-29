
/*
 * cube.cpp
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#include "cube.h"
#include "err.h"

// cube ///////////////////////////////////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | |v7---|-|v4
//  |/      |/
//  v2------v3

// vertex coords array for glDrawArrays() =====================================
// A cube has 6 sides and each side has 2 triangles, therefore, a cube consists
// of 36 vertices (6 sides * 2 tris * 3 vertices = 36 vertices). And, each
// vertex is 3 components (x,y,z) of floats, therefore, the size of vertex
// array is 108 floats (36 * 3 = 108).
GLfloat vertices[]  = { 1, 1, 1,  -1, 1, 1,  -1,-1, 1,      // v0-v1-v2 (front)
                       -1,-1, 1,   1,-1, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1,-1, 1,   1,-1,-1,      // v0-v3-v4 (right)
                        1,-1,-1,   1, 1,-1,   1, 1, 1,      // v4-v5-v0

                        1, 1, 1,   1, 1,-1,  -1, 1,-1,      // v0-v5-v6 (top)
                       -1, 1,-1,  -1, 1, 1,   1, 1, 1,      // v6-v1-v0

                       -1, 1, 1,  -1, 1,-1,  -1,-1,-1,      // v1-v6-v7 (left)
                       -1,-1,-1,  -1,-1, 1,  -1, 1, 1,      // v7-v2-v1

                       -1,-1,-1,   1,-1,-1,   1,-1, 1,      // v7-v4-v3 (bottom)
                        1,-1, 1,  -1,-1, 1,  -1,-1,-1,      // v3-v2-v7

                        1,-1,-1,  -1,-1,-1,  -1, 1,-1,      // v4-v7-v6 (back)
                       -1, 1,-1,   1, 1,-1,   1,-1,-1 };    // v6-v5-v4

// normal array
GLfloat normals[]   = { 0, 0, 1,   0, 0, 1,   0, 0, 1,      // v0-v1-v2 (front)
                        0, 0, 1,   0, 0, 1,   0, 0, 1,      // v2-v3-v0

                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v0-v3-v4 (right)
                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v4-v5-v0

                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v6-v1-v0

                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v1-v6-v7 (left)
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v7-v2-v1

                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v7-v4-v3 (bottom)
                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v3-v2-v7

                        0, 0,-1,   0, 0,-1,   0, 0,-1,      // v4-v7-v6 (back)
                        0, 0,-1,   0, 0,-1,   0, 0,-1 };    // v6-v5-v4

// color array
GLfloat colors[]    = { 1, 1, 1,   1, 1, 0,   1, 0, 0,      // v0-v1-v2 (front)
                        1, 0, 0,   1, 0, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1, 0, 1,   0, 0, 1,      // v0-v3-v4 (right)
                        0, 0, 1,   0, 1, 1,   1, 1, 1,      // v4-v5-v0

                        1, 1, 1,   0, 1, 1,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   1, 1, 0,   1, 1, 1,      // v6-v1-v0

                        1, 1, 0,   0, 1, 0,   0, 0, 0,      // v1-v6-v7 (left)
                        0, 0, 0,   1, 0, 0,   1, 1, 0,      // v7-v2-v1

                        0, 0, 0,   0, 0, 1,   1, 0, 1,      // v7-v4-v3 (bottom)
                        1, 0, 1,   1, 0, 0,   0, 0, 0,      // v3-v2-v7

                        0, 0, 1,   0, 0, 0,   0, 1, 0,      // v4-v7-v6 (back)
                        0, 1, 0,   0, 1, 1,   0, 0, 1 };    // v6-v5-v4



Cube::Cube()
    : m_VboID(0)
    , m_IsInitialized(false)
    , m_HasVBO(false)

    , m_NormalArrayEnabled(0)
    , m_ColorArrayEnabled(0)
    , m_VertexArrayEnabled(0)
{
}

Cube::~Cube()
{
    glDeleteBuffersARB(1, &m_VboID);
}

bool Cube::HandleEvent(const SDL_Event& event)
{
	return false;
}

bool Cube::Initialize()
{
    m_HasVBO  = glewGetExtension("GL_ARB_vertex_buffer_object");
    ASSERT( m_HasVBO, "VBOs not supported!" );
    if ( m_HasVBO ) {
        // create vertex buffer objects, you need to delete them when program exits
        // Try to put both vertex coords array, vertex normal array and vertex color in the same buffer object.
        // glBufferDataARB with NULL pointer reserves only memory space.
        // Copy actual data with 2 calls of glBufferSubDataARB, one for vertex coords and one for normals.
        // target flag is GL_ARRAY_BUFFER_ARB, and usage flag is GL_STATIC_DRAW_ARB
        glGenBuffersARB(1, &m_VboID);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_VboID);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices)+sizeof(normals)+sizeof(colors), 0, GL_STATIC_DRAW_ARB);
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(vertices), vertices);                             // copy vertices starting from 0 offest
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices), sizeof(normals), normals);                // copy normals after vertices
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices)+sizeof(normals), sizeof(colors), colors);  // copy colours after normals

    }
    m_IsInitialized = true;
    return m_IsInitialized;
}

void Cube::Render(long ticks)
{
    // save the initial ModelView matrix before modifying ModelView matrix
//    glPushMatrix();

//    // tramsform camera
//    glTranslatef(0, 0, -m_CameraDistance);
//    glRotatef(m_CameraAngleX, 1, 0, 0);   // pitch
//    glRotatef(m_CameraAngleY, 0, 1, 0);   // heading

    // bind VBOs with IDs and set the buffer offsets of the bound VBOs
    // When buffer object is bound with its ID, all pointers in gl*Pointer()
    // are treated as offset instead of real pointer.

    // enable vertex arrays
    glGetIntegerv( GL_NORMAL_ARRAY, &m_NormalArrayEnabled );
    if ( !m_NormalArrayEnabled )  {
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    glGetIntegerv( GL_COLOR_ARRAY, &m_ColorArrayEnabled );
    if ( !m_ColorArrayEnabled) {
        glEnableClientState(GL_COLOR_ARRAY);
    }
    glGetIntegerv( GL_VERTEX_ARRAY, &m_VertexArrayEnabled );
    if (!m_VertexArrayEnabled) {
        glEnableClientState(GL_VERTEX_ARRAY);
    }

    // Render with VBO - if available
    if ( m_HasVBO ) {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_VboID);
        // before draw, specify vertex and index arrays with their offsets
        glNormalPointer(GL_FLOAT, 0, (void*)sizeof(vertices));
        glColorPointer(3, GL_FLOAT, 0, (void*)(sizeof(vertices)+sizeof(normals)));
        glVertexPointer(3, GL_FLOAT, 0, 0);
    } else {
        // before draw, specify vertex arrays
        glNormalPointer(GL_FLOAT, 0, normals);
        glColorPointer(3, GL_FLOAT, 0, colors);
        glVertexPointer(3, GL_FLOAT, 0, vertices);
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);

    if (!m_VertexArrayEnabled)  {
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    }
    if (!m_ColorArrayEnabled)   {
        glDisableClientState(GL_COLOR_ARRAY);
    }
    if (!m_NormalArrayEnabled) {
        glDisableClientState(GL_NORMAL_ARRAY);
    }

    if ( m_HasVBO ){
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
//    glPopMatrix();
}

void Cube::PostRender( )
{

}
