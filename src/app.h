/*
 * app.h
 *
 *  Created on: 2013-01-15
 *      Author: jurgens
 */

#ifndef APP_H_
#define APP_H_

#include "worker.h"
#include "entity.h"

#include <boost/shared_ptr.hpp>

class App
{
	boost::shared_ptr< Worker > m_Worker;

	EntityList m_EventHandlerList;
    SDL_Joystick    *m_Joystick;
public:
	App();

	~App();

	void Init( int argc, char *argv[] );

	int Run();

protected:
};


#endif /* APP_H_ */
