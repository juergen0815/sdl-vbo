/************************************************************************/
/* pei::Engine (©)Copyright 2009-12 Jürgen Schober                      */
/*                                                                      */
/* This Software is distributed as Freeware. It may be copied and       */
/* distributed (even as a companion to a commercial product) as long as */
/* the archive stays complete. No further agreement is necessary.       */
/************************************************************************/

#ifndef SPHERE_H
#define SPHERE_H

#include "err.h"
#include "entity.h"
#include "vector.h"

#include <vector>

class Sphere : public Entity
{
public:
    enum {
        NORMAL,
        SPECULAR
    };

private:
    int  m_VboID;
    int  m_IdxBufferID;

    typedef std::vector<Vector> VertexArray;
    typedef std::vector<Vector> ColorArray;
    typedef std::vector<unsigned int> IndexArray;

    int         m_Stride;
    VertexArray m_VertexBuffer; // linear buffer
    VertexArray m_NormalBuffer; // linear buffer
    ColorArray  m_ColorBuffer;  // color buffer overlays Vertex Array
    IndexArray  m_IndexArray;   // standard array to map vertices to tris

    float       m_Radius;
    Vector      m_Position;
    Vector      m_Rotation;
public:
    Sphere();

    virtual ~Sphere();

private:
    void MakeSphere( float meridians, float parallels );

protected:
    virtual bool Initialize( );

    virtual void Render( long ticks );

    virtual bool HandleEvent( const SDL_Event& event ) { return false; }

};


#endif /* SPHERE_H */
