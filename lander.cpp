// lander.cpp


#include "headers.h"
#include "lander.h"
#include "world.h"
#include "gpuProgram.h"
#include "ll.h"


// Animation of the lander is not physically realistic.  The lander
// should really have a mass (which decreases as fuel is used) and a
// thrust in Newtons, from which acceleration should be calculated.
// We also have rotation without rotational inertia (as in the
// original game).

#define ROTATION_SPEED 0.4	          // upon sidewise thrust, rotation speed in radians/second
#define THRUST_ACCEL 4.0                  // upon main thrust, acceleration in m/s/s
#define GRAVITY vec3( 0, -1.6, 0 )        // gravity acceleration on the moon is 1.6 m/s/s
#define LANDER_WIDTH 10                  // the real lander is about 6.7 m wide, not 10m

#define ROTATIONAL_FUEL_CONSUMPTION   20  // Fuel/second during rotation
#define THRUST_FUEL_CONSUMPTION      100  // Fuel/second during thrust

#define CLAMP(min,max,x) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

// Set up the lander by creating a VAO and rewriting the lander
// vertices so that the lander is centred at (0,0).

void Lander::setupVAO()

{
  // ---- Rewrite the lander vertices ----

  // Find the bounding box of the lander

  vec3 min = vec3( landerVerts[0], landerVerts[1], 0 );
  vec3 max = vec3( landerVerts[0], landerVerts[1], 0 );

  int i;
  for (i=0; landerVerts[i] != -1; i+=2) {
    vec3 v( landerVerts[i], landerVerts[i+1], 0 );
    if (v.x < min.x) min.x = v.x;
    if (v.x > max.x) max.x = v.x;
    if (v.y < min.y) min.y = v.y;
    if (v.y > max.y) max.y = v.y;
  }

  // number of segments in the lander model

  numSegments = i/2;		

  // Rewrite the model vertices so that the lander is centred at (0,0)
  // and has width LANDER_WIDTH.
  //
  // Also, invert the y axis since the original vertices are defined
  // in a coordinate system with y increasing downward.
  
  float s = LANDER_WIDTH / (max.x - min.x); // scale
 
  mat4 modelToOriginTransform = scale( s, -s, 1 ) * translate( -(min.x+max.x)/2, -(min.y+max.y)/2, 0 );

  for (int i=0; landerVerts[i] != -1; i+=2) {
    vec4 newV = modelToOriginTransform * vec4( landerVerts[i], landerVerts[i+1], 0.0, 1.0 );
    landerVerts[i]   = newV.x / newV.w;
    landerVerts[i+1] = newV.y / newV.w;

  }

  // ---- Create a VAO for this object ----

  // YOUR CODE HERE
  

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  GLuint VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, 2 * numSegments * sizeof(float), &landerVerts[0], GL_STATIC_DRAW);

  // define the position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

}

// Draw the lander

void Lander::draw( mat4 &worldToViewTransform )
{
	// YOUR CODE HERE
	float x = position.x;
	float y = position.y;
	worldToViewTransform = worldToViewTransform * translate(x, y, 0)* rotate(orientation, vec3(0,0,1));

	glBindVertexArray(VAO);
	glUniformMatrix4fv(glGetUniformLocation(myGPUProgram->id(), "MVP"), 1, GL_TRUE, &worldToViewTransform[0][0]);

	glDrawArrays(GL_LINES, 0, numSegments);

}


// Update the pose (position and orientation)


void Lander::updatePose( float deltaT )

{
  position    = position    + deltaT * velocity;           // first-order approximations
  orientation = orientation + deltaT * angularVelocity;
  velocity    = velocity    + deltaT * GRAVITY;

  // wrap around screen

  if (position.x > world->maxX() + 10)
    position.x = world->minX() - 10;
  else if (position.x < world->minX() - 10)
    position.x = world->maxX() + 10;
}


// Update the thrust or orientation


void Lander::rotateCW( float deltaT )

{
	orientation -= ROTATION_SPEED * deltaT;

	// YOUR CODE HERE
	fuel -= ROTATIONAL_FUEL_CONSUMPTION * deltaT;
}


void Lander::rotateCCW( float deltaT )

{
	orientation += ROTATION_SPEED * deltaT;

	// YOUR CODE HERE
	fuel -= ROTATIONAL_FUEL_CONSUMPTION * deltaT;

 
}


// Change velocity in direction of thrust


void Lander::addThrust( float deltaT )

{
  // YOUR CODE HERE
	velocity = velocity + deltaT * THRUST_ACCEL * vec3(-sin(orientation),cos
	(orientation),0);

	fuel -= THRUST_FUEL_CONSUMPTION * deltaT;
}


// Lunar lander model consisting of line segments
//
// These are in a ARBITRARY coordinate system and get remapped to the
// world coordinate system (with the centre of the lander at (0,0))
// when the VAO is set up.


float Lander::landerVerts[] = {

  165,859, 157,852,
  157,852, 157,842,
  157,842, 166,834,
  166,834, 183,834,
  183,834, 191,843,
  191,843, 191,852,
  191,852, 182,859,

  157,859, 191,859,
  191,859, 191,873,
  191,873, 157,873,
  157,873, 157,859,

  191,866, 195,880,
  195,880, 185,873,

  172,873, 168,881,
  168,881, 179,881,
  179,881, 175,873,

  157,866, 153,880,
  153,880, 162,873,

  153,880, 153,889,

  147,889, 158,889,

  195,880, 195,889,

  189,889, 201,889,

  168,881, 178,881,

  // Done
  
  -1
};
