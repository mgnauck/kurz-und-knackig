#ifndef GLOBAL_H
#define GLOBAL_H

// 4k optimized or safe
#define SAFE_CODE

#ifdef _DEBUG
	#define SAFE_CODE
#endif

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
#endif

// Audio
#ifdef _WIN32
	// Play audio only on windows (via PlaySound)
	#define AUDIO 
#endif
// Save audio to file
//#define AUDIO_SAVE

// Video
//#define FULLSCREEN
#define RESX					1024
#define RESY					576

// Data
#define TEXTURE_RES				2
#define PLANE_SUBX				50
#define PLANE_SUBY				50
#define PLANE_TEXX				10.5f
#define PLANE_TEXY				10.5f

#define SHADOWS

#define MAX_PARTICLE_SYSTEMS	550

#ifdef SAFE_CODE

	#define TITLE				"[safe]"

#else

	#ifdef FULLSCREEN
		#define TITLE			""
	#else
		#define TITLE			"[unsafe]"
	#endif

#endif

#ifdef SAFE_CODE

	#define GLFW_INCLUDE_GLCOREARB // GL_BGRA_EXT
	#include <GLFW/glfw3.h>

	#ifdef __APPLE__
		#include <OpenGL/gl.h>
		#include <OpenGL/glu.h>
	#endif

	#ifdef _WIN32
		#include <gl/gl.h>
		#include <gl/glu.h>
	#endif

#else

	// 'Unsafe' version (aka 4k optimized). Works only on Windows.
	#include <windows.h>
	#include <gl/gl.h>
	#include <gl/glu.h>

	extern PIXELFORMATDESCRIPTOR	pfd;
	extern HWND						hwnd;
	extern HDC						device;

#endif

#endif