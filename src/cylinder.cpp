#include "cylinder.h"

#include <GL/glew.h>

#include <cmath>

#include <boost/filesystem.hpp>

const int columns = 32;
const int rows    = 8;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Cylinder::Cylinder( )
    : m_VboID(-1)
    , m_IdxBufferID(-1)
    , m_Stride(1) // needed if/when we pack color + vertex into one array
    , m_VertexBuffer( columns*rows*m_Stride )    // use the same memory pool for vertex and texture coords
    , m_ColorBuffer( columns*rows*m_Stride )
    , m_Radius(4.0f)
{
    // we might just want to create this in DoInitialize - and throw away the data we don't need locally
    MakeCylinder( columns, rows );
}

Cylinder::~Cylinder()
{
    // shouldn't be done in d'tor...might be weakly linked to e.g. event handler...but vbo must be released from render thread
    if ( m_VboID > 0 ) {
        glDeleteBuffers(1, (GLuint*)&m_VboID);
    }
    if ( m_IdxBufferID > 0 ) {
        glDeleteBuffers(1, (GLuint*)&m_IdxBufferID);
    }
}

void Cylinder::MakeCylinder( float columns, float rows )
{
    std::vector< std::vector< Vector > > rings;

    const float RAD180 = M_PI; // PI in RAD
    const float RAD360 = M_PI*2; // 2*PI in RAD

    int lastColumn = columns - 1;
    int lastRow = rows - 1;

    // Add two extra vertices at center bottom and top
    m_VertexBuffer.resize( columns*rows*m_Stride + 2 );
    m_NormalBuffer.resize( columns*rows*m_Stride + 2 );
    m_ColorBuffer.resize( columns*rows*m_Stride + 2 );

    // generate index array; we got rows * columns * 2 tris
    m_IndexArray.resize( columns * lastRow * 3 * 2 + columns*3*2 ); // 3 vertices per tri, 2 tri per quad = 6 entries per iteration

    const float height = 6;
    auto vit = m_VertexBuffer.begin();
    auto nit = m_NormalBuffer.begin();
    auto cit = m_ColorBuffer.begin();
    int looper(0);

    // need one extra ring to close the gap (overlaps 0)
    double segmentAngle = RAD180/rows;
    double segmentSize  = RAD360/columns;
    for( float y = 0; y < rows; ++y ){  //0-PI
        std::vector< Vector > segments;
        for( float x = 0; x < columns; ++x ) { //0-2PI
            float phi = x * segmentSize;
            // vertex
            auto& vertex = *vit; ++vit;
            // Cylinder
            vertex[ Vector::X ] = std::cos(phi) * m_Radius / 2.0f; //std::cos(theta) * std::sin(phi);
            vertex[ Vector::Y ] = y / rows * height - height/2; // std::sin(theta) * std::cos(phi);
            vertex[ Vector::Z ] = std::sin(phi) * m_Radius / 2.0f; // std::cos(phi);

            // Add normal vectors - at vertex direction from center (at y pos)
            auto& normal = *nit; ++nit;
            normal = Vector( vertex ).Sub( {  0, vertex[ Vector::Y ], 0  } ).Normalize();

            // vertex color
            auto& color = *cit; ++cit;
            color = { 1.0f, 1.0f, 1.0f, 1.0f };

            // skip last column/row - already indexed
            if ( y < lastRow ) {
                // vertices don't need to be set just yet. We just index them here

                // top tri
                int
                idx = int((int(x + 0) % lastColumn) + columns*y);     m_IndexArray[ looper++ ] = idx;  // 0x0
                idx = int((int(x + 1) % lastColumn) + columns*y);     m_IndexArray[ looper++ ] = idx;  // 1x0
                idx = int((int(x + 0) % lastColumn) + columns*(y+1)); m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row

                // bottom tri
                idx = int((int(x + 1) % lastColumn) + columns*y);     m_IndexArray[ looper++ ] = idx; // 0x0
                idx = int((int(x + 1) % lastColumn) + columns*(y+1)); m_IndexArray[ looper++ ] = idx; // 0x1 - bottom row
                idx = int((int(x + 0) % lastColumn) + columns*(y+1)); m_IndexArray[ looper++ ] = idx; // 1x1 - bottom row
                idx = 0;
            }
        }
    }

    // we share vertices with first and last ring -> problem with normals. This point away from center, only center is correct
    auto& vertex = *vit; ++vit;
    vertex = { 0,-height/2, 0 };   // bottom - center
    auto& normal = *nit; ++nit;
    normal = { 0, -1, 0 };       // point down
    auto& color = *cit; ++cit;
    color = { 1.0f, 0.0f, 0.0f, 1.0f };

    vertex = *vit; ++vit;
    vertex = { 0, height/2, 0 };   // top - center
    normal = *nit; ++nit;
    normal = { 0, +1, 0 };       // point up
    color = *cit; ++cit;
    color = { 0.0f, 0.0f, 1.0f, 1.0f };

    int bottomIdx = rows;
    int topIdx    = rows+1;

    // close top and bottom
    for( int x = 0; x < columns; ++x ) { //0-2PI
        // bottom
        int
        idx = (x + 0) % lastColumn; m_IndexArray[ looper++ ] = idx;  // 0x0 - readability!
        idx = (x + 1) % lastColumn; m_IndexArray[ looper++ ] = idx;  // 1x0
        idx = bottomIdx;            m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row
    }
    for( int x = 0; x < columns; ++x ) { //0-2PI
        // top
        int x1 = x + columns * (rows-1);
        int
        idx = (x1 + 1) % lastColumn; m_IndexArray[ looper++ ] = idx;  // 1x0
        idx = (x1 + 0) % lastColumn; m_IndexArray[ looper++ ] = idx;  // 0x0 - readability!
        idx = topIdx;                m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row
    }
}

bool Cylinder::Initialize( )
{
    bool hasVBO  = glewGetExtension("GL_ARB_vertex_buffer_object");
    ASSERT( hasVBO, "VBOs not supported!" );

    glGenBuffers(1, (GLuint*)&m_VboID);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
    int bufSize = sizeof(Vector)*m_VertexBuffer.size()
                + sizeof(Vector)*m_NormalBuffer.size()
                + sizeof(Vector)*m_ColorBuffer.size();
    glBufferData(GL_ARRAY_BUFFER, bufSize, 0, GL_STATIC_DRAW);
    std::size_t offset(0);
    // Vertex buffer
    float *vertices = (float*)&m_VertexBuffer[0];
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_VertexBuffer.size(), vertices);
    // Normal buffer
    offset += sizeof(Vector)*m_VertexBuffer.size();
    float *normals = (float*)&m_NormalBuffer[0];
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_NormalBuffer.size(), normals);
    // Color buffer
    offset += sizeof(Vector)*m_NormalBuffer.size();
    float *colors = (float*)&m_ColorBuffer[0];
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Vector)*m_ColorBuffer.size(), colors);

    // Index Buffer
    glGenBuffers(1, (GLuint*)&m_IdxBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IdxBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*m_IndexArray.size(), &m_IndexArray[0], GL_STATIC_DRAW);

    // TODO: We can delete local storage here

    return true;
}

void Cylinder::Render( long ticks )
{
    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    m_Rotation[ Vector::X ] += 25.0f * float(ticks) / 1000.0f;

    glTranslatef( +5, 1, 0);
    glRotatef( m_Rotation[ Vector::X ], 1, 0, 0);   // pitch
    glRotatef( 0, 0, 1, 0);   // heading

    int vertexArrayEnabled;
    glGetIntegerv( GL_VERTEX_ARRAY, &vertexArrayEnabled );
    if (!vertexArrayEnabled) {
        glEnableClientState(GL_VERTEX_ARRAY);
    }
    int normalArrayEnabled;
    glGetIntegerv( GL_NORMAL_ARRAY, &normalArrayEnabled );
    if ( !normalArrayEnabled )  {
        glEnableClientState(GL_NORMAL_ARRAY);
    }
    int colorArrayEnabled;
    glGetIntegerv( GL_COLOR_ARRAY, &colorArrayEnabled );
    if ( !colorArrayEnabled) {
        glEnableClientState(GL_COLOR_ARRAY);
    }

    int blend_enabled;
    glGetIntegerv(GL_BLEND, &blend_enabled);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboID);
    // before draw, specify vertex and index arrays with their offsets
    std::size_t offset(0);
    glVertexPointer(4, GL_FLOAT, m_Stride*sizeof(Vector), (void*)offset);

    offset += sizeof(Vector)*m_VertexBuffer.size();
    glNormalPointer(GL_FLOAT, m_Stride*sizeof(Vector), (void*)offset);

    offset += sizeof(Vector)*m_NormalBuffer.size();
    glColorPointer(4, GL_FLOAT, m_Stride*sizeof(Vector), (void*)offset);

    // use index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IdxBufferID);
    glDrawElements( GL_TRIANGLES, m_IndexArray.size(), GL_UNSIGNED_INT, (void*)0 );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!vertexArrayEnabled)  {
        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    }
    if ( !normalArrayEnabled )  {
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    if ( !colorArrayEnabled) {
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glPopMatrix();
}

