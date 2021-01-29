// world.cpp


#include "world.h"
#include "lander.h"
#include "ll.h"
#include "gpuProgram.h"
#include "strokefont.h"

#include <sstream>
#include <iomanip>
float PI = 3.14;
float altitude = 100;
string message = "";

//const float textAspect = 0.7;	// text width-to-height ratio (you can use this for more realistic text on the screen)
float runTime = 0;

void World::updateState(float elapsedTime)

{
	// See if any keys are pressed for thrust
	runTime += elapsedTime;
	if (lander->checkFuel() > 0) {
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) // right arrow
			lander->rotateCW(elapsedTime);

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) // left arrow
			lander->rotateCCW(elapsedTime);

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) // down arrow
			lander->addThrust(elapsedTime);
	}

	// Update the position and velocity

	lander->updatePose(elapsedTime);

	// See if the lander has touched the terrain

	vec3 closestTerrainPoint = landscape->findClosestPoint(lander->centrePosition());
	float closestDistance = (closestTerrainPoint - lander->centrePosition()).length();

	// Find if the view should be zoomed

	zoomView = (closestDistance < ZOOM_RADIUS);

	// Check for landing or collision and let the user know
	//
	// Landing is successful if the vertical speed is less than 1 m/s and
	// the horizontal speed is less than 0.5 m/s.
	//
	// SHOULD ALSO CHECK THAT LANDING SURFACE IS HORIZONAL, BUT THIS IS
	// NOT REQUIRED IN THE ASSIGNMENT.
	//
	// SHOULD ALSO CHECK THAT THE LANDER IS VERTICAL, BUT THIS IS NOT
	// REQUIRED IN THE ASSIGNMENT.

	// YOUR CODE HERE
	altitude = landscape->getAltitude(lander->centrePosition());
	if (altitude <= 0) {
		pauseGame = TRUE;
		if (abs(lander->getVelocity().x) <= .5 && abs(lander->getVelocity().y) < 1) {
			score += 100;
			message = "Successful Landing!";
		}else {
			message = "Crash Landing!";
		}

	}
}

void World::draw()

{
  mat4 worldToViewTransform;

  if (!zoomView) {

    // Find the world-to-view transform that transforms the world
    // to the [-1,1]x[-1,1] viewing coordinate system, with the
    // left edge of the landscape at the left edge of the screen, and
    // the bottom of the landscape BOTTOM_SPACE above the bottom edge
    // of the screen (BOTTOM_SPACE is in viewing coordinates).

    float s = 2.0 / (landscape->maxX() - landscape->minX());

    worldToViewTransform
      = translate( -1, -1 + BOTTOM_SPACE, 0 )
      * scale( s, s, 1 )
      * translate( -landscape->minX(), -landscape->minY(), 0 );

  } else {

    // Find the world-to-view transform that is centred on the lander
    // and is 2*ZOOM_RADIUS wide (in world coordinates).

    // YOUR CODE HERE
	  float s = 1/(2*ZOOM_RADIUS);
	  worldToViewTransform
		    = translate(0, BOTTOM_SPACE, 0)
			* scale(s, s, 1)
			* translate(-lander->centrePosition().x, -lander->centrePosition().y, 0);
  }

  // Draw the landscape and lander, passing in the worldToViewTransform
  // so that they can append their own transforms before passing the
  // complete transform to the vertex shader.

  landscape->draw( worldToViewTransform );
  lander->draw( worldToViewTransform );

  // Draw the heads-up display (i.e. all text).

  stringstream ss;

  drawStrokeString( "LUNAR LANDER", -0.2, 0.85, 0.06, glGetUniformLocation( myGPUProgram->id(), "MVP") );
  drawStrokeString(message, -.5, 0.1, 0.1, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  ss.setf( ios::fixed, ios::floatfield );
  ss.precision(1);

  //ss << ;
  drawStrokeString("SCORE ", -0.95, 0.75, 0.04, glGetUniformLocation( myGPUProgram->id(), "MVP") );
  ss << score;
  drawStrokeString(ss.str(), -0.65, 0.75, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  
  drawStrokeString("TIME", -0.95, 0.65, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  ss.str("");
  ss.precision(0);
  ss << runTime << "s";
  drawStrokeString(ss.str(), -0.65, 0.65, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));


  drawStrokeString("FUEL ", -0.95, 0.55, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  ss.str("");
  ss << lander->checkFuel();
  drawStrokeString(ss.str(), -0.65, 0.55, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  ss.str(std::string());

  drawStrokeString("ALTITUDE", 0.05, 0.75, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  ss.str("");
  ss.precision(1);

  ss << altitude;
  drawStrokeString(ss.str(), 0.55, 0.75, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));

   vec3 vel = lander->getVelocity();
   float vx = vel.x;
   float vy = vel.y;


   drawStrokeString("HORIZONAL SPEED", 0.05, 0.65, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));
   ss.str("");
   ss <<  abs(vx);
  drawStrokeString(ss.str(), 0.55, 0.65, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  
  float dir;
  if (vx < 0) { dir = PI/2; } else { dir = -PI/2; }

  drawArrow(worldToViewTransform, 0.7, 0.67, dir);
  
  drawStrokeString("VERTICAL SPEED ", 0.05, 0.55, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));
  ss.str("");
  ss <<  abs(vy);
  drawStrokeString(ss.str(), 0.55, 0.55, 0.04, glGetUniformLocation(myGPUProgram->id(), "MVP"));

  if (vy < 0) { dir = PI; }
  else { dir = 0; }

  drawArrow(worldToViewTransform, 0.7, 0.57, dir);

}
void World::drawArrow(mat4& worldToViewTransform, float x, float y, float direction)
{
	float s = .03;
	mat4 transform = translate(x, y, 1) * rotate(direction, vec3(0, 0, 1)) * scale(s, s, 1);

	glBindVertexArray(arrowVAO);
	glUniformMatrix4fv(glGetUniformLocation(myGPUProgram->id(), "MVP"), 1, GL_TRUE, &transform[0][0]);
	glDrawArrays(GL_LINES, 0, numArrowVerts);

}

// Create an arrow VAO
//
// This needs 'numArrowVerts' and 'arrowVAO' defined in world.h

void World::setupArrowVAO()

{
  // create an arrow from line segments

  GLfloat arrowVerts[] = {
    0,    1,    0,    -1,
    0,    1,    0.5, 0.25,
    0,    1,   -0.5, 0.25,
    0.5, 0.25, -0.5, 0.25
  };
  numArrowVerts = sizeof(arrowVerts)/sizeof(GLfloat);
  // ---- Create a VAO for this object ----

  glGenVertexArrays( 1, &arrowVAO );
  glBindVertexArray( arrowVAO );

  // Store the vertices

  GLuint VBO;
  
  glGenBuffers( 1, &VBO );
  glBindBuffer( GL_ARRAY_BUFFER, VBO );
  glBufferData( GL_ARRAY_BUFFER, numArrowVerts*sizeof(GLfloat), arrowVerts, GL_STATIC_DRAW );

  // define the position attribute

  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );

  // Done

  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}
