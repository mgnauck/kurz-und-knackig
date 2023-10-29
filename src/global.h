#ifndef GLOBAL_H
#define GLOBAL_H

// 4k optimized or safe code
#define SAFE_CODE
#ifdef _DEBUG
	#define SAFE_CODE
#endif

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
#endif

// Audio
#ifdef _WIN32
	// Play audio ONLY on windows (via PlaySound)
	#define AUDIO 
#endif

// Save audio to file
//#define AUDIO_SAVE
#ifdef AUDIO_SAVE
	// Make sure directory exists
	#define AUDIO_FILE_PATH			"output/audio.wav"
#endif

// Video
//#define FULLSCREEN
#define RESX						1920
#define RESY						1080

// Save frames as png files
//#define CAPTURE_FRAMES
#ifdef CAPTURE_FRAMES
	#define CAPTURE_FRAME_RATE		60
	#define CAPTURE_TIME_DELTA		1000.0f / (float)CAPTURE_FRAME_RATE
	// Make sure directory exists. File will receive suffix '_{i}.png'.
	#define CAPTURE_IMAGE_PATH		"output/frame"
#endif

// Data
#define TEXTURE_RES					2
#define PLANE_SUBX					50
#define PLANE_SUBY					50
#define PLANE_TEXX					10.5f
#define PLANE_TEXY					10.5f

#define SHADOWS

#define MAX_PARTICLE_SYSTEMS		550

#ifdef SAFE_CODE

	#define TITLE					"[safe]"

#else

	#ifdef FULLSCREEN
		#define TITLE				""
	#else
		#define TITLE				"[unsafe]"
	#endif

#endif

#ifdef SAFE_CODE

	#define GLFW_INCLUDE_GLCOREARB // GL_BGRA_EXT
	#include <GLFW/glfw3.h>

	#ifdef __APPLE__
		//#include <OpenGL/gl.h>
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