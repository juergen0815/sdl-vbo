/************************************************************************/
/* pei::Engine (©)Copyright 2009-12 Jürgen Schober                      */
/*                                                                      */
/* This Software is distributed as Freeware. It may be copied and       */
/* distributed (even as a companion to a commercial product) as long as */
/* the archive stays complete. No further agreement is necessary.       */
/************************************************************************/

#include "sphere.h"
//#include "arrow.h"

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

        // TODO: Revise that! Top and bottom need TRI_FAN, rest TRI_STRIP

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
               // TODO: use 4th component for face direction! (must flip > 180dgr)
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
