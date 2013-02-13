/*
 * camera.h
 *
 *  Created on: 2013-01-16
 *      Author: jurgens
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "err.h"
#include "entity.h"
#include "vector.h"

class Camera : public Entity
{
    bool   m_MouseLeftDown;
    bool   m_MouseMiddleDown;
    bool   m_MouseRightDown;
    float  m_MouseX, m_MouseY;
    float  m_CameraSpeed;

    Vector m_CameraAngle;
    Vector m_CameraPosition;

    Vector m_JoyStickMotionAxis;
    Vector m_JoystickOrientationAxis;
    SDL_Joystick *m_Joystick;
public:
    Camera( SDL_Joystick* joystick );

    virtual ~Camera();

private:
    virtual bool HandleEvent( const SDL_Event& event ); // -> ?? override; not working

    virtual bool Initialize( );

    virtual void Render( long ticks );

    float GetJoystickAxisValue( int index );
};


#endif /* CAMERA_H_ */
