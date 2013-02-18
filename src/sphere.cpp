#include "sphere.h"

#include <GL/glew.h>

#include <cmath>

#include <boost/filesystem.hpp>

const int sCcolumns = 32;
const int sRows    = 12;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923f
#endif

const static std::size_t NUM_COLORS = 12;
static Vector sColors[NUM_COLORS] = // Rainbow Of Colors
{
    {1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
    {0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
    {0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

Sphere::Sphere( float radius /* = 1.0f */ )
    : m_VboID(-1)
    , m_IdxBufferID(-1)
    , m_Stride(1) // needed if/when we pack color + vertex into one array
    , m_VertexBuffer( sCcolumns*sRows*m_Stride )    // use the same memory pool for vertex and texture coords
    , m_ColorBuffer( sCcolumns*sRows*m_Stride )
    , m_Radius(radius)
    , m_Position( { 0, 0, 3 } )
    , m_Scale( { 1,1,1 } )
    , m_Rotation( { 0,0,0,0 } )
{
    // we might just want to create this in DoInitialize - and throw away the data we don't need locally
    MakeSphere( sCcolumns, sRows );
}

Sphere::~Sphere()
{
    // shouldn't be done in d'tor...might be weakly linked to e.g. event handler...but vbo must be released from render thread
    if ( m_VboID > 0 ) {
        glDeleteBuffers(1, (GLuint*)&m_VboID);
    }
    if ( m_IdxBufferID > 0 ) {
        glDeleteBuffers(1, (GLuint*)&m_IdxBufferID);
    }
}

void Sphere::MakeSphere( float columns, float rows )
{
    std::vector< std::vector< Vector > > rings;

    const float RAD180 = M_PI; // PI in RAD
    const float RAD360 = M_PI*2; // 2*PI in RAD

    ++rows;
    int lastColumn = columns - 1;
    int lastRow = rows - 1;

    // Add two extra vertices at center bottom and top
    m_VertexBuffer.resize( columns*rows*m_Stride );
    m_NormalBuffer.resize( columns*rows*m_Stride );
    m_ColorBuffer.resize( columns*rows*m_Stride );

    // generate index array; we got rows * columns * 2 tris
    m_IndexArray.resize( columns * rows * 3 * 2 ); // 3 vertices per tri, 2 tri per quad = 6 entries per iteration

    auto vit = m_VertexBuffer.begin();
    auto nit = m_NormalBuffer.begin();
    auto cit = m_ColorBuffer.begin();
    int looper(0);
    int iv(0);

    int numVertices = columns*rows;

    // from http://www.math.montana.edu/frankw/ccp/multiworld/multipleIVP/spherical/learn.htm

    // need one extra ring to close the gap (overlaps 0)
    float segmentAngle = RAD180/lastRow;
    float segmentSize  = RAD360/columns;
    for( float y = 0; y < rows; ++y ){  //0-PI
        std::vector< Vector > segments;
        float theta = y * segmentAngle;
        for( float x = 0; x < columns; ++x ) { //0-2PI
            float phi = x * segmentSize;
            // vertex
            auto& vertex = *vit; ++vit;
            vertex[ Vector::X ] = m_Radius * std::sin(phi) * std::cos(theta);
            vertex[ Vector::Y ] = m_Radius * std::sin(phi) * std::sin(theta);
            vertex[ Vector::Z ] = m_Radius * std::cos(phi);

            // Add normal vectors - at vertex direction from center (-{0,0,0})
            auto& normal = *nit; ++nit;
            normal = vertex.Normalized();

            // vertex color
            auto& color = *cit; ++cit;
            color = sColors[iv*NUM_COLORS/numVertices]; ++iv;

            // vertices don't need to be set just yet. We just index them here

            // this needs work: we use a row * col vertex and texture array
            // to extract triangles, the index array needs to be calculated appropriately
            //        0  1  2...n
            //        +--+--+...
            //        |\ |\ |
            //        | \| \|
            //        +--+--+
            // n*y +  0' 1' 2'...(n+1)*y

            // e.g. t[0] = { 0,1,1'} { 1',0',1 } ...
            // top tri
            int
            idx = int((int(x + 0) % lastColumn) + columns *(int(y+0)%(int)rows)); m_IndexArray[ looper++ ] = idx;  // 0x0
            idx = int((int(x + 1) % lastColumn) + columns *(int(y+0)%(int)rows)); m_IndexArray[ looper++ ] = idx;  // 1x0
            idx = int((int(x + 0) % lastColumn) + columns *(int(y+1)%(int)rows)); m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row

            // bottom tri
            idx = int((int(x + 1) % lastColumn) + columns *(int(y+0)%(int)rows)); m_IndexArray[ looper++ ] = idx; // 0x0
            idx = int((int(x + 1) % lastColumn) + columns *(int(y+1)%(int)rows)); m_IndexArray[ looper++ ] = idx; // 0x1 - bottom row
            idx = int((int(x + 0) % lastColumn) + columns *(int(y+1)%(int)rows)); m_IndexArray[ looper++ ] = idx; // 1x1 - bottom row
        }
    }
}

bool Sphere::Initialize( )
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

void Sphere::Render( long ticks )
{
    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();

    m_Rotation[ Vector::X ] += 45.0f * float(ticks) / 1000.0f;
    m_Rotation[ Vector::Y ] += 90.0f * float(ticks) / 1000.0f;

    glTranslatef( m_Position[Vector::X], m_Position[Vector::Y], m_Position[Vector::Z] );
    glScalef( m_Scale[Vector::X], m_Scale[Vector::Y], m_Scale[Vector::Z] );
    glRotatef( m_Rotation[ Vector::X ], 1, 0, 0);
    glRotatef( m_Rotation[ Vector::Y ], 0, 1, 0);
    glRotatef( m_Rotation[ Vector::Z ], 0, 0, 1);

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

