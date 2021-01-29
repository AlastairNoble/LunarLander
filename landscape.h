// landscape.h
//
// The landscape x coordinate is 0 at the leftmost point of the
// terrain and is LANDSCAPE_WIDTH at the right edge, measured in
// meters.  The y coordinate is 0 at the bottommost point of the
// terrain.


#ifndef LANDSCAPE_H
#define LANDSCAPE_H


#include "headers.h"


#define LANDSCAPE_WIDTH 1000.0	// Width of landscape in meters

#define ZOOM_RADIUS 70.0        // Radius of zoomed view (when lander is close to terrain)


class Landscape {

  static float landscapeVerts[];
  int numVerts;			// number of vertices in the landscape model
  GLuint VAO;

 public:

  Landscape() {
    setupVAO();
  }

  void setupVAO();  

  void draw( mat4 &worldToViewTransform );

  float minX() { return 0; }
  float maxX() { return LANDSCAPE_WIDTH; }
  float minY() { return 0; }

  vec3 findClosestPoint( vec3 position, vec3 segTail, vec3 segHead );
  vec3 findClosestPoint( vec3 position );

  float getAltitude(vec3 pos);
};


#endif
