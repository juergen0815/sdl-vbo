/************************************************************************/
/* pei::Engine (©)Copyright 2009-12 Jürgen Schober                      */
/*                                                                      */
/* This Software is distributed as Freeware. It may be copied and       */
/* distributed (even as a companion to a commercial product) as long as */
/* the archive stays complete. No further agreement is necessary.       */
/************************************************************************/

#ifndef SPHERE_H
#define SPHERE_H

#include <application/app_config.h>

#include <render/drawable.h>

#include <math/vector4f.h>

#include <boost/shared_ptr.hpp>

namespace shape {
    class Sphere;
    typedef boost::shared_ptr<Sphere> SpherePtr;

    class Sphere : public pei::Drawable
    {
        int   m_NormalsDplList;
        float m_Radius;
        bool  m_ShowNormals;

        std::vector<pei::Vector4f> normalBuffer;
        std::vector<pei::Vector4f> vertexBuffer;

    public:
        Sphere( float r  = 1.0f );

        Sphere( float meridians, float parallels );

        virtual ~Sphere();

        void SetRadius( float r ) {
            m_Radius = r;
        }

        void ShowNormals( bool showNormals ) {
            m_ShowNormals = showNormals;
        }

        virtual bool OnInit( pei::RenderProfilePtr& profile );

    protected:
        void MakeSphere( float meridians, float parallels );

        virtual void OnDraw( pei::RenderProfilePtr& profile, pei::SurfacePtr& buffer, const pei::RenderParam& param  );
    };
}

#endif /* CUBE_H */
