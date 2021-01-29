// Lunar lander game


#include <sys/timeb.h>

#include "headers.h"
#include "gpuProgram.h"
#include "world.h"
#include "ll.h"


GPUProgram *myGPUProgram;	// pointer to GPU program object

World *world;			// the world, including landscape and lander

bool pauseGame = false;

struct timeb startTime;

int screenWidth = 800;  	// width in pixels
float screenAspect = 1.3333;	// 4:3 aspect ratio of the original screen


// Define basic shaders


char *vertexShader =
#ifdef MACOS  
  "#version 330\n"
#else
  "#version 300 es\n"
#endif
  "\n"
  "layout (location = 0) in vec4 position;\n"
  "uniform mat4 MVP;\n"
  "\n"
  "void main()\n"
  "\n"
  "{\n"
  "  gl_Position = MVP * position;\n"
  "}";



char *fragmentShader = 

#ifdef MACOS  
  "#version 330\n"
#else
  "#version 300 es\n"
#endif
  "\n"
  "out mediump vec4 fragColour;\n"
  "\n"
  "void main()\n"
  "\n"
  "{\n"
  "  fragColour = vec4( 0.2, 0.7, 0.4, 1.0 );\n"
  "}";



// Handle a keypress


void keyCallback( GLFWwindow *w, int key, int scancode, int action, int mods )

{
  if (action == GLFW_PRESS)
    
    if (key == GLFW_KEY_ESCAPE)	// quit upon ESC
      exit(0);

    else if (key == 'P')	// p = pause
      pauseGame = !pauseGame;

    else if (key == 'R')	// r = reset lander
      world->resetLander();

    else if (key == '?') 	// ? = output help
      cout << "help" << endl;
}


// Error callback

void errorCallback( int error, const char* description )

{
  cerr << "Error: " << description << endl;
}


  
// Main program


int main( int argc, char **argv )

{
  // Set up GLFW

  GLFWwindow* window;

  if (!glfwInit()) {
    cerr << "GLFW failed to initialize" << endl;
    return 1;
  }
  
  glfwSetErrorCallback( errorCallback );
  
  // Open window

#ifdef MACOS
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#else
  glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
#endif

  window = glfwCreateWindow( screenAspect * screenWidth, screenWidth, "Lunar Lander", NULL, NULL);
  
  if (!window) {
    glfwTerminate();
    cerr << "GLFW failed to create a window" << endl;

#ifdef MACOS
    const char *descrip;
    int code = glfwGetError( &descrip );
    cerr << "GLFW code:  " << code << endl;
    cerr << "GLFW error: " << descrip << endl;
#endif
    
    return 1;
  }

  glfwMakeContextCurrent( window );
  
  glfwSwapInterval( 1 ); // redraw at most every 1 screen scan
  
  // Set OpenGL function bindings

  gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress );

  // Set up callbacks

  glfwSetKeyCallback( window, keyCallback );
  
  // Set up shaders

  myGPUProgram = new GPUProgram();
  myGPUProgram->init( vertexShader, fragmentShader );
  myGPUProgram->activate();

  // Set up world

  world = new World( window );

  // Run

  struct timeb prevTime, thisTime;
  ftime( &prevTime );

  startTime = prevTime;

  while (!glfwWindowShouldClose( window )) {

    // Find elapsed time since last render

    ftime( &thisTime );
    float elapsedSeconds = (thisTime.time + thisTime.millitm / 1000.0) - (prevTime.time + prevTime.millitm / 1000.0);
    prevTime = thisTime;

    // Update the world state

    if (!pauseGame)
      world->updateState( elapsedSeconds );

    // Display the world

    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glClear( GL_COLOR_BUFFER_BIT );
    world->draw();

    glfwSwapBuffers( window );
    
    // Check for new events

    glfwPollEvents();
  }

  glfwDestroyWindow( window );
  glfwTerminate();
  return 0;
}


// A bug in some GL* library in Ubuntu 14.04 requires that libpthread
// be forced to load.  The code below accomplishes this (from MikeMx7f
// http://stackoverflow.com/questions/31579243/segmentation-fault-before-main-when-using-glut-and-stdstring).
//
// You'll also need to modify the Makefile to uncomment the pthread parts

// #include <pthread.h>

// void* simpleFunc(void*) { return NULL; }

// void forcePThreadLink() { pthread_t t1; pthread_create(&t1, NULL, &simpleFunc, NULL); }
