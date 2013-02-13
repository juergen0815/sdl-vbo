/************************************************************************/
/* pei::Engine (©)Copyright 2009-12 Jürgen Schober                      */
/*                                                                      */
/* This Software is distributed as Freeware. It may be copied and       */
/* distributed (even as a companion to a commercial product) as long as */
/* the archive stays complete. No further agreement is necessary.       */
/************************************************************************/

#include "sphere.h"

#include <GL/glew.h>

#include <cmath>

#include <boost/filesystem.hpp>

const int columns = 32;
const int rows    = 8;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere( )
    : m_VboID(-1)
    , m_IdxBufferID(-1)
    , m_Stride(1) // needed if/when we pack color + vertex into one array
    , m_VertexBuffer( columns*rows*m_Stride )    // use the same memory pool for vertex and texture coords
    , m_ColorBuffer( columns*rows*m_Stride )
    , m_Radius(4.0f)
{
    // we might just want to create this in DoInitialize - and throw away the data we don't need locally
    MakeSphere( columns, rows );

#if 0




    // generate index array; we got rows * columns * 2 tris
    m_IndexArray.resize( (rows-1) * (columns-1) * 3 * 2 ); // 3 vertices per tri, 2 tri per quad = 6 entries per iteration

    int looper(0);
    // width x height is always a quad, not a rect
    const float width_2  = 5.0f; // 4.4f - for columns = 45
    const float height_2 = 5.0f;

    // generate vertex array
    const float xstep = (2*width_2)/columns; // mesh sub divider - 0.2f
    const float ystep = (2*height_2)/rows; // mesh sub divider - 0.2f
    const float amp  = 0.85f; // "height" of wave
    const float numWaves = 16.0f; // num of sin loops (or waves)
    auto vit = m_VertexBuffer.begin();

    // I think we need an additional row/column to finish this mesh ??
    for ( float y = 0; y < rows; ++y )
    {
        for ( float x = 0; x < columns; ++x )
        {
            Vector& vertex = *vit; ++vit;
            vertex[ Vector::X ] = x * xstep - width_2; // -4.4 ... +4.4
            vertex[ Vector::Y ] = y * ystep - height_2; // -4.4 ... +4.4
            // maybe I should shift this for each row, huh, norm x to "length" of column (0.0 - 1.0)
            vertex[ Vector::Z ] = std::sin( (x/columns) * numWaves ) * amp; // make z a big "wavy"

            // calc texture positions
            Vector& texCoord = *vit; ++vit;
            texCoord[ Vector::U ] = x/(columns-1);
            texCoord[ Vector::V ] = y/(rows-1);

            // this needs work: we use a row * col vertex and texture array
            // to extract triangles, the index array needs to be calculated appropriately
            //        0  1  2...n
            //        +--+--+...
            //        |\ |\ |
            //        | \| \|
            //        +--+--+
            // n*y +  0' 1' 2'...(n+1)*y

            // e.g. t[0] = { 0,1,1'} { 1',0',1 } ...

            // skip last column/row - already indexed
            if ( x < (columns-1) && y < (rows-1) ) {
                // vertices don't need to be set just yet. We just index them here

                // top tri
                int
                idx = int(x + 0 + columns*y);     m_IndexArray[ looper++ ] = idx;  // 0x0
                idx = int(x + 1 + columns*y);     m_IndexArray[ looper++ ] = idx;  // 1x0
                idx = int(x + 0 + columns*(y+1)); m_IndexArray[ looper++ ] = idx;  // 1x1 - bottom row

                // bottom tri
                idx = int(x + 1 + columns*y);     m_IndexArray[ looper++ ] = idx; // 0x0
                idx = int(x + 0 + columns*(y+1)); m_IndexArray[ looper++ ] = idx; // 1x1 - bottom row
                idx = int(x + 1 + columns*(y+1)); m_IndexArray[ looper++ ] = idx; // 0x1 - bottom row
                idx = 0;
            }
        }
    }
#endif
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
        float theta = y * segmentAngle;
        for( float x = 0; x < columns; ++x ) { //0-2PI
            float phi = x * segmentSize;
            // vertex
            auto& vertex = *vit; ++vit;
            // Cylinder
#if 1
            vertex[ Vector::X ] = std::cos(phi) * m_Radius / 2.0f; //std::cos(theta) * std::sin(phi);
            vertex[ Vector::Y ] = y / rows * height - height/2; // std::sin(theta) * std::cos(phi);
            vertex[ Vector::Z ] = std::sin(phi) * m_Radius / 2.0f; // std::cos(phi);

            // Add normal vectors - at vertex direction from center (at y pos)
            auto& normal = *nit; ++nit;
            normal = Vector( vertex ).Sub( {  0, vertex[ Vector::Y ], 0  } ).Normalize();
#else
            // Sphere
            vertex[ Vector::X ] = std::cos(theta) * std::sin(phi) * m_Radius / 2.0f;
            vertex[ Vector::Y ] = std::sin(theta) * std::sin(phi) * m_Radius / 2.0f;
            vertex[ Vector::Z ] = std::cos(phi) * m_Radius / 2.0f;
#endif

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

#if 0

#include <render/tex_surface.h>

#include <math/math.h>

namespace shape {
    Sphere::Sphere(  float r /* = 1.0f */ )
        : m_NormalsDplList(0)
        , m_Radius(r)
        , m_ShowNormals(false)
    {
        float meridians = 32 * m_Radius; if ( meridians < 4 ) meridians = 4;
        float parallels = 32 * m_Radius; if ( parallels < 4 ) parallels = 4;
        MakeSphere( meridians, parallels );
    }

    Sphere::Sphere( float meridians, float parallels )
        : m_NormalsDplList(0)
        , m_Radius(1.0)
        , m_ShowNormals(false)
    {
        MakeSphere( meridians, parallels );
    }

    Sphere::~Sphere() {}

    void Sphere::MakeSphere( float meridians, float parallels )
    {
        pei::render::VertexArrayRenderTarget *rt = new pei::render::VertexArrayRenderTarget;
        SetRenderTarget( pei::RenderTargetInterfacePtr(rt) );

        std::vector< std::vector< pei::Vector4f> > rings;
        std::vector< int > polyVector;

        // need one extra ring to close the gap (overlaps 0)
        double segmentAngle = pei::RAD180/meridians;
        double segmentSize  = pei::RAD360/parallels;
        for( float theta = 0; theta <= (pei::RAD180+0.1f); theta = theta + segmentAngle ){  //0-PI
            int p = 0;
            std::vector< pei::Vector4f > segments;
            for( double phi = 0;phi < pei::RAD360; phi = (phi + segmentSize)){ //0-2PI
               float x = (float) (cos(theta)* sin(phi));
               float y = (float) (sin(theta)* sin(phi));
               float z = (float)  cos(phi);

               pei::Vector4f vec( x, y, z );
               segments.push_back( vec );
               p++;
            }
            rings.push_back( segments );
            polyVector.push_back( p );
        }
        // create linear array of vertices - need to resort to create QUAD  (or TRIANGLE) STRIPS
        // Top and Bottom should be triangle fan...
        unsigned int p = 0;
        unsigned int r = 0;
        unsigned int n = rings.size();
        std::vector<pei::Vector4f>& ring0 = rings[r];
        while ( ++r < n ) {
            std::vector<pei::Vector4f>& ring1 = rings[r%n];

            pei::Vector4f& vtx0 = ring0.at(0);
            pei::Vector4f& vtx1 = ring1.at(0);
            vertexBuffer.push_back( vtx0 );
            vertexBuffer.push_back( vtx1 );

            normalBuffer.push_back( vtx0.Normalized() ); // normal vector is just the normalized vector from the center to the point (give a center 0,0,0)
            normalBuffer.push_back( vtx1.Normalized() ); // normal vector is just the normalized vector from the center to the point (give a center 0,0,0)

            int polys = 2;
            int segments = ring0.size();
            for ( int y = 1; y < segments+1; y++ ) {
                // Need to flip every other poly to generate triangle strips
                pei::Vector4f& vtx2 = ring1.at(y%segments);
                pei::Vector4f& vtx3 = ring0.at(y%segments);
                // flip for every other segment (TRI-STRIP)
                vertexBuffer.push_back( vtx2 );
                vertexBuffer.push_back( vtx3 );

                normalBuffer.push_back( vtx2.Normalized() ); // normal vector is just the normalized vector from the center to the point (give a center 0,0,0)
                normalBuffer.push_back( vtx3.Normalized() ); // normal vector is just the normalized vector from the center to the point (give a center 0,0,0)

                polys += 2;
            }
            // make this some triangles some times
            rt->AddPolygon( GL_TRIANGLE_STRIP, polys, p );
//            rt->AddPolygon( GL_QUAD_STRIP, polys, p );
            p += polys;
            ring0 = ring1;
        }

        // copy normals into array - maybe I should just change everything to using vectors instead...
        n = 0;
        unsigned int numNormals(normalBuffer.size());
        float *normals = new float[ numNormals*3];
        for ( n = 0; n < numNormals; ++n ) {
            pei::Vector4f& vec1 = normalBuffer[n];
            normals[n++] = vec1.x();
            normals[n++] = vec1.y();
            normals[n++] = vec1.z();
        }

        // copy vertices into array
        unsigned int numVertices = vertexBuffer.size();
        GLushort *indices = new GLushort[ numVertices ];
        GLfloat *vertices = new GLfloat[ numVertices * 3 ];
        unsigned int v = 0; unsigned short i = 0;
        for ( n = 0; n < numVertices; ++n ) {
            pei::Vector4f& vertex = vertexBuffer[n];
            indices[i] = i; ++i;
            vertices[v++] = vertex.x();
            vertices[v++] = vertex.y();
            vertices[v++] = vertex.z();
        }

        rt->SetVertices( numVertices*3, vertices );
        rt->SetNormals( numNormals*3, normals );
        rt->SetIndices( numVertices, indices );

        delete [] vertices;
        delete [] normals;
        delete [] indices;
    }

    bool Sphere::OnInit( pei::RenderProfilePtr& profile )
    {
    #if 0
        // normal vectors, just for fun
        m_NormalsDplList = glGenLists(1);
        glNewList( m_NormalsDplList, GL_COMPILE);
        for ( unsigned int i = 0; i < vertexBuffer.size() && i < normalBuffer.size(); i++ ) {
            pei::Vector4f vertex = vertexBuffer.at(i);
            pei::Vector4f normal = pei::Vector4f::Add( normalBuffer.at(i), vertex );
            Arrow::Draw( vertex, normal, pei::Color(1.0f,0.8f,0.2f,1.0) );
        }
        glEndList();
    #endif
        return true;
    }

    void Sphere::OnDraw( pei::RenderProfilePtr& profile, pei::SurfacePtr& buffer, const pei::RenderParam& param  )
    {
    #if 0
        // draw the normals - dbg/sample
        if ( m_ShowNormals && ( param.GetRenderPass() == pei::DEFAULT_PASS ) && (m_NormalsDplList > 0) ) {
            glCallList(m_NormalsDplList);
        }
    #endif
        bool cullface = profile->Disable( GL_CULL_FACE );
        pei::Drawable::OnDraw( profile, buffer, param );
        if ( cullface ) profile->Enable( GL_CULL_FACE);
    }
}

#endif
