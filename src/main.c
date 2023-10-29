/*

	kurz & knackig
	2005 (c)ode warp+supah
	4k, released at evoke, cologne

	-
	markus@unik.de

	cc -lglfw -framework OpenGL audio.c face.c global.c main.c particle.c particlesystem.c spring.c utility.c vec.c
	(Add -lpng when capturing frames)

*/

#include <stdlib.h>
#include "global.h"
#include "particlesystem.h"
#include "utility.h"
#include "audio.h"

#ifdef CAPTURE_FRAMES
#include <png.h>
#endif

// object defs
#define	FLOOR				0			// singles
#define	CLOTH				1
#define STICKS_START		2			// arrays
#define STICKS_COUNT		98
#define BOXES_START			100
#define BOXES_COUNT			200
#define BOXES_2_START		300
#define BOXES_2_COUNT		200

// sphere in floor
#define FLOOR_SPHERE_RADIUS	4400.0f

// time
#define	START_TIME			0
#define	OVERALL_TIME		1
#define	PHYSICS_TIME		2
#define TIME_DELTA			0.03f
#define	PHYSICS_UPDATE_TIME	15.0f
#define	STICKS_TIME			0
#define	BOXES_TIME			27000
#define	CLOTH_TIME			48000
#define	BOXES_2_TIME		76000
#define	EXPLODE_TIME		84000
#define	END_TIME			105000

//  debug
#define	TIME_OFFSET			0

// audio
#define	DURATION			(END_TIME / 1000)
#define	SAMPLERATE			44100

// textures
#define	TEXTURE_COUNT		3
#define	FLOOR_TEXTURE		1

// camera
vec							sim[2];

// objects
particlesystem				ps[MAX_PARTICLE_SYSTEMS];

// times
float						times[3];

// textures
int							tex[][4] = {
								{ 0xeeaaaaaa, 0xddeeeeee, 0xddeeeeee, 0xeeaaaaaa },	// floor
								{ 0xeeffffff, 0xeec4d4c4, 0xeec4d4c4, 0xeea0b0a0 },	// cloth
								{ 0xeeeeffee, 0xeeeeffee, 0xeeeeffee, 0xeeeeffee },	// boxes
							};

// light
float						light_pos[]			= {-30.0f, 85.0f, 30.0f, 1.0f};

// misc
int							explode				= 0;
float						coll_sphere_rad		= 0.0f;

// audio
#if defined(AUDIO) || defined(AUDIO_SAVE)
wavdata_t*					wav;
#endif

// capture frames
#ifdef CAPTURE_FRAMES
float 						capture_time = 0.0f;
unsigned long				capture_index = 0;
uint8_t*    				capture_buffer = NULL;
#endif

void set_cam(float ex, float ey, float ez, float tx, float ty, float tz) {
	sim[0].x = ex;	sim[0].y = ey;	sim[0].z = ez;
	sim[1].x = tx;	sim[1].y = ty;	sim[1].z = tz;
}

static inline
float getTime() {
#ifdef CAPTURE_FRAMES
	return capture_time;
#else
	#ifndef SAFE_CODE
		return (float)GetTickCount();
	#else
		return (float)(glfwGetTime() * 1000.0);
	#endif
#endif
}

#ifdef CAPTURE_FRAMES
int write_image_to_png_file(const char* path, size_t width, size_t height, size_t bpp, uint8_t* pixel_data)
{
	// Ripped from the internet and adjusted.
    FILE*           fp;
    
    png_structp     png_ptr = NULL;
    png_infop       info_ptr = NULL;

    uint8_t         color_type;
    size_t          bytes_per_pixel = bpp / 8;
    
    size_t          x, y, i;
    png_byte**      row_pointers = NULL;
    uint8_t*        pixel_ptr = NULL;
    
    // Set error state until data is written successfully
    int             status = -1;

    // Evaluate color type based on given bpp
    switch(bpp) {
        case 8:
            color_type = PNG_COLOR_TYPE_GRAY;
        break;
        case 16:
            color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
        break;
        case 24:
            color_type = PNG_COLOR_TYPE_RGB;
        break;
        case 32:
            color_type = PNG_COLOR_TYPE_RGB_ALPHA;
        break;
        default:
            // Unknown color type, return with error
            return status;
    }

    // Open output file
    fp = fopen(path, "wb+");
    if(!fp) {
        goto fopen_failed;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }
    
    // Set up error handling
    if(setjmp(png_jmpbuf(png_ptr))) {
        goto png_failure;
    }
    
    // Set image attributes
    png_set_IHDR(png_ptr, info_ptr, width, height, /* depth */ 8, color_type,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    // Initialize png data rows
    row_pointers = png_malloc(png_ptr, sizeof(png_byte*) * height);
    pixel_ptr = pixel_data;
    for(y=0; y<height; y++) {
        png_byte* row = png_malloc(png_ptr, sizeof(uint8_t) * width * bytes_per_pixel);
        row_pointers[height - 1 - y] = row;
        for(x=0; x<width; x++) {
            for(i=0; i<bytes_per_pixel; i++) {
                *row++ = (png_byte)*pixel_ptr++;
            }
        }
    }
    
    // Write the image data to "fp"
    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    // File successfully written, set status accordingly
    status = 0;
    
    for (y=0; y<height; y++) {
        png_free(png_ptr, row_pointers[y]);
    }
    png_free(png_ptr, row_pointers);
    
 png_failure:
 png_create_info_struct_failed:
    png_destroy_write_struct(&png_ptr, &info_ptr);
 png_create_write_struct_failed:
    fclose(fp);
 fopen_failed:
    return status;
}

void write_frame(const char* base_path, size_t width, size_t height, unsigned int index) {

    char        image_path[1024];

    snprintf(image_path, sizeof(image_path), "%s_%06u.png", base_path, index);

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, capture_buffer);

    write_image_to_png_file(image_path, width, height, 24, capture_buffer);
}
#endif

static inline
void init() {

	int i;

	// setup gl
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glEnable(GL_LIGHT0);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.8f);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0003f);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0000025f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80.0, 4.0 / 3.0, 1.0, 2000.0);

	glMatrixMode(GL_MODELVIEW);

	// init geometry
	srand(303);

	// floor
	ps_plane(&ps[FLOOR], 2000.0f, 2000.0f, 0.0f);
	ps_calc_normals(&ps[FLOOR]);
	ps[FLOOR].texture = FLOOR_TEXTURE;

	// cloth
	ps_plane(&ps[CLOTH], 110.0f, 110.0f, 2.0f);
	ps_translate(&ps[CLOTH], 0.0f, 90.0f, 0.0f);
	ps[CLOTH].particles[0].invm = 0.0f;
	ps[CLOTH].particles[49].invm = 0.0f;
	ps[CLOTH].particles[2450].invm = 0.0f;
	ps[CLOTH].particles[2499].invm = 0.0f;
	ps[CLOTH].texture--;	// cloth texture is default set texture - 1

	// sticks
	for(i=STICKS_START; i<STICKS_START + STICKS_COUNT; i++) {

		ps_box(&ps[i], 1.75f, 60.0f, 4.0f, /*0.5f, 0.5f,*/ randf(3.5f, 6.5f));
		ps_translate(&ps[i], randf(-350.0f, 350.0f), randf(120.0f, 300.0f), randf(-350.0f, 350.0f));
	}

	srand(21280);

	// boxes
	for(i=BOXES_START; i<BOXES_START + BOXES_COUNT; i++) {

		ps_box(&ps[i], randf(3.0f, 45.0f), randf(1.0f, 45.0f), randf(3.0f, 45.0f), /*0.5f, 0.5f,*/ randf(4.5f, 8.5f));
		ps_set_force(&ps[i], 1.0f, 1.0f, 1.0f);
		ps_translate(&ps[i], randf(-650.0f, 650.0f), randf(200.0f, 650.0f) + (float)((i - BOXES_START) * 12), randf(-650.0f, 650.0f));

		// BOXES_2_START = BOXES_START + 200
		ps_box(&ps[200 + i], 16.0f, 16.0f, 16.0f, /*0.5f, 0.5f,*/ 6.0f);
		ps_set_force(&ps[200 + i], 1.0f, 1.0f, 1.0f);
		ps_translate(&ps[200 + i], randf(-300.0f, 300.0f), 17.0f + (float)((i - BOXES_START) * 2), randf(-1000.0f, 1000.0f));
	}

	// upload textures
	for(i=0; i<TEXTURE_COUNT; i++)
		upload_texture(tex[i]);

	// audio
#if defined(AUDIO) || defined(AUDIO_SAVE)
	audio_prerender(&wav, DURATION, SAMPLERATE);
#endif

#ifdef AUDIO_SAVE
	audio_save(AUDIO_FILE_PATH, wav);
#endif

	// init time vars
	times[PHYSICS_TIME] = STICKS_TIME;
	times[START_TIME]	= getTime();

#ifdef AUDIO
	audio_play(wav);
#endif
}

static inline
void run() {

	int					i;
	float				s, c;
	int					actual_count;
	particlesystem*		actual_objects;

	// not so stiff objects in last scene
	static float		stiffness_first		= 1.0f;
	static float		stiffness_second	= 1.0f;

	// clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// global time
	times[OVERALL_TIME] = (getTime() - times[START_TIME]) + TIME_OFFSET;

	// scenes/timeline
	// sticks
	if((times[OVERALL_TIME] >= STICKS_TIME) && (times[OVERALL_TIME] < BOXES_TIME)) {
		actual_objects = &ps[STICKS_START];
		actual_count = STICKS_COUNT;
		s = sin(times[OVERALL_TIME] * 0.0005f);
		c = cos(times[OVERALL_TIME] * 0.0004f);
		set_cam(s * 230.0f, s * c * 30.0f + 50.0f, c * 250.0f, s * 30.0f, 20.0f + c * 45.0f, 0.0f);
	}
	// boxes
	if((times[OVERALL_TIME] >= BOXES_TIME) && (times[OVERALL_TIME] < CLOTH_TIME)) {
		actual_objects = &ps[BOXES_START];
		actual_count = BOXES_COUNT;
		s = sin(times[OVERALL_TIME] * 0.0003f);
		c = cos(times[OVERALL_TIME] * 0.0004f);
		set_cam(s * 373.0f, s * 30.0f + 48.0f, c * 420.0f, s * 130.0f, 20.0f * s + 40.0f, 80.0f * c + 50.0f);
	}
	// cloth
	if((times[OVERALL_TIME] >= CLOTH_TIME) && (times[OVERALL_TIME] < BOXES_2_TIME)) {
		actual_objects = &ps[CLOTH];
		actual_count = 1;
		coll_sphere_rad = FLOOR_SPHERE_RADIUS;
		s = sin(times[OVERALL_TIME] * 0.0004f);
		c = cos(times[OVERALL_TIME] * 0.00045f);
		set_cam(s * 40.0f, 100.0f + s * 20.0f, -50.0f + c * 30.0f, c * 30.0f, 50.0f, 0.0f);

		// unlock fixed particles
		if(times[OVERALL_TIME] >= CLOTH_TIME + 13000) {
			coll_sphere_rad = 0.0f;
			ps[CLOTH].particles[0].invm = 1.0f;
			ps[CLOTH].particles[49].invm = 1.0f;
		}
	}
	// boxes_2
	if((times[OVERALL_TIME] >= BOXES_2_TIME) && (times[OVERALL_TIME] < END_TIME)) {
		stiffness_first = 0.2f;
		stiffness_second = 0.1f;
		actual_objects = &ps[BOXES_2_START];
		actual_count = BOXES_2_COUNT;
		s = sin(times[OVERALL_TIME] * 0.0004f);
		c = cos(times[OVERALL_TIME] * 0.0004f);
		set_cam(s * 250.0f, c * 30.0f + 50.0f, c * 400.0f, s * 30.0f, 50.0f + s * 10.0f, 48.0f * s);
	}
	// pseudo fade-out
	if(times[OVERALL_TIME] > (END_TIME - 5000)) {
		light_pos[1] -= 0.8f;
		light_pos[0] += 20.0f;
	}

	// physics
	if(times[OVERALL_TIME] - times[PHYSICS_TIME] > PHYSICS_UPDATE_TIME) {

		// integrate + relax single particle systems
		for(i=0; i<actual_count; i++) {
			ps_integrate(&actual_objects[i], TIME_DELTA);
			ps_relax(&actual_objects[i], coll_sphere_rad, stiffness_first);
			ps_relax(&actual_objects[i], coll_sphere_rad, stiffness_second);
		}

		// shit hits the fan
		if((times[OVERALL_TIME] > EXPLODE_TIME) && (explode < 100)) {
			for(i=0; i<BOXES_2_COUNT; i++)
				ps_set_force(&ps[BOXES_2_START + i], 0.3f, 0.6f, 0.3f);
			explode++;
		}
		//*/

		times[PHYSICS_TIME] = times[OVERALL_TIME];
	}

	// set light pos
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

	// set camera w/ some kewl roll
	c = 0.85f + cos(0.0002f * times[OVERALL_TIME]) * 0.15f;
	gluLookAt(sim[0].x, sim[0].y, sim[0].z, sim[1].x, sim[1].y, sim[1].z, 1.0f - c, c, 0.0f);

	// render
	glPushMatrix();
	glCullFace(GL_FRONT);		// reflections
	glScalef(1.0, -1.0, 1.0);
	ps_render_range(actual_objects, actual_count, 0);
	glCullFace(GL_BACK);
	glPopMatrix();

	// objects
	ps_render_range(actual_objects, actual_count, 1);

	// floor
	//glPushMatrix();
	glRotatef(30.0f, 0.0f, 1.0f, 0.0f);
	ps_render(&ps[FLOOR]);
	//glPopMatrix();
}

#ifndef SAFE_CODE

	void mainCRTStartup() {
	//void __cdecl main() {			// crinkler!1#@??

	#ifdef FULLSCREEN
		hwnd = CreateWindow(	"EDIT", "", WS_VISIBLE | WS_POPUP | WS_MAXIMIZE,
								0, 0, RESX, RESY, 0, 0, 0, 0	);
	#else
		hwnd = CreateWindowEx(	WS_EX_TOPMOST, "EDIT", TITLE, WS_CAPTION | WS_VISIBLE | WS_POPUP,
								0, 0, RESX, RESY, 0, 0, 0, 0	);
	#endif

		device = GetDC(hwnd);

		SetPixelFormat(device, ChoosePixelFormat(device, &pfd), &pfd);
		wglMakeCurrent(device, wglCreateContext(device));

		ShowCursor(0);

		init();

		while(!GetAsyncKeyState(VK_ESCAPE) && (times[OVERALL_TIME] < END_TIME)) {
			run();
			SwapBuffers(device);
		}

		ExitProcess(0);
	}

#else

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
	    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	        glfwSetWindowShouldClose(window, GLFW_TRUE);
	}		

	int main(int argc, char* argv[]) {

		GLFWwindow* window;

		if(!glfwInit())
	  	{
	  		return -1;
	  	}

	#ifdef FULLSCREEN
	  	window = glfwCreateWindow(RESX, RESY, TITLE, glfwGetPrimaryMonitor(), NULL);
	#else
		window = glfwCreateWindow(RESX, RESY, TITLE, NULL, NULL);
	#endif

		if(!window)
		{
			glfwTerminate();
			return -1;
		}

		glfwSetKeyCallback(window, key_callback);
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

	#ifdef CAPTURE_FRAMES		
		capture_buffer = (uint8_t*)malloc(RESX * RESY * 3);
	#endif

		init();

		while(!glfwWindowShouldClose(window) && (times[OVERALL_TIME] < END_TIME)) {

			run();
			
			glfwSwapBuffers(window);
			glfwPollEvents();

		#ifdef CAPTURE_FRAMES
			write_frame(CAPTURE_IMAGE_PATH, RESX, RESY, capture_index);
			capture_index++;
			capture_time += CAPTURE_TIME_DELTA;
		#endif
		}

	#if defined(AUDIO) || defined(AUDIO_SAVE)
		free(wav);
	#endif

	#ifdef CAPTURE_FRAMES		
		free(capture_buffer);
	#endif

		glfwDestroyWindow(window);
		glfwTerminate();
		
		return 0;
	}

#endif
