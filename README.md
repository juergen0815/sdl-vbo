sdl-vbo

Author:

	Jurgen Schober
	
Date:
   
	January, 2013
	
Short:
  
	Example using vertex buffers in OpenGL.

Description:

    Simple example how to use vertex buffer objects in OpenGL with SDL, glew and C++11.
    
    	
	All sdl-xx-examples are written in C++11 using MinGW gcc 4.6 and are Windows only. I'm using
	Eclipse Juno as Development IDE.

Libs used:

	boost_thread
	boost_system
	boost_filesystem
	boost_iostreams
	glew
	+ OpenGL

License:

	Use as is. No license other then the ones included with third party libraries are required.

Compiler used:

	MinGW with Eclipse Juno (gcc4.6 with -std=c++0x). Windows only. Linux might just work, MacOS will 
	need some work due to the fact OSX needs to run the render loop in the main loop (compiles with 
	LVM compiler). This example runs a render thread decoupled from the main thread.

Have fun
Jurgen
