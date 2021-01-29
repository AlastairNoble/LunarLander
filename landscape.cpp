// landscape.cpp


#include "headers.h"
#include "landscape.h"
#include "gpuProgram.h"
#include "ll.h"


// Set up the landscape by creating a VAO and rewriting the landscape
// vertices so that the x values fit in [ 0, LANDSCAPE_WIDTH ].


void Landscape::setupVAO()

{
  // ---- Rewrite the landscape vertices into world coordinates ----

  // Find the bounding box of the landscape

  vec3 min = vec3( landscapeVerts[0], landscapeVerts[1], 0 );
  vec3 max = vec3( landscapeVerts[0], landscapeVerts[1], 0 );

  numVerts = 0;

  for (int i=0; landscapeVerts[i] != -1; i+=2) {

    vec3 v( landscapeVerts[i], landscapeVerts[i+1], 0 );

    if (v.x < min.x) min.x = v.x;
    if (v.x > max.x) max.x = v.x;
    if (v.y < min.y) min.y = v.y;
    if (v.y > max.y) max.y = v.y;

    numVerts++;
  }

  // Translate the landscape so that its lower-left corner is at (0,0)
  // and its width is LANDSCAPE_WIDTH and y increases upward.
  // 
  // Note that y increases downward in the model, so the y axis must
  // be inverted.

  float s = LANDSCAPE_WIDTH / (max.x - min.x);

  mat4 modelToWorldTransform = scale( s, -s, 1 ) * translate( -min.x, -max.y, 0 );

  // Rewrite the model vertices so that they are in the world
  // coordinate system.
  //
  // Also, fix the landscape so that it doesn't have more than one
  // segment and any x position.  Segments that go backward in x
  // should be pushed a bit forward to prevent this.  This makes it
  // much easier to detect a lander/landscape collision.

  float prevX = 0;

  for (int i=0; landscapeVerts[i] != -1; i+=2) {

    vec4 newV = modelToWorldTransform * vec4( landscapeVerts[i], landscapeVerts[i+1], 0, 1 );

    landscapeVerts[i]   = newV.x / newV.w;
    landscapeVerts[i+1] = newV.y / newV.w;

    // prevent the landscape from going backward
    
    if (landscapeVerts[i] < prevX)
      landscapeVerts[i] = prevX;

    prevX = landscapeVerts[i];
  }

  // ---- Create a VAO for this object ----

  glGenVertexArrays( 1, &VAO );
  glBindVertexArray( VAO );

  // Store the vertices

  GLuint VBO;
  glGenBuffers( 1, &VBO );
  glBindBuffer( GL_ARRAY_BUFFER, VBO );
  glBufferData( GL_ARRAY_BUFFER, 2*numVerts*sizeof(float), &landscapeVerts[0], GL_STATIC_DRAW );

  // define the position attribute

  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );
}


// Draw the landscape


void Landscape::draw(  mat4 &worldToViewTransform )

{
  glBindVertexArray( VAO );

  glUniformMatrix4fv( glGetUniformLocation( myGPUProgram->id(), "MVP"), 1, GL_TRUE, &worldToViewTransform[0][0] );

#ifndef MACOS  
  glLineWidth( 2.0 );
#endif

  glDrawArrays( GL_LINE_STRIP, 0, numVerts );
}


// Find the point on a segment ( 'segTail', 'segHead' ) that is
// closest to 'position'.


vec3 Landscape::findClosestPoint( vec3 position, vec3 segTail, vec3 segHead )

{
  // Compute perpendicular projection of 'position' onto the line
  // containing 'segTail' and 'segHead'.  If the projection is outside
  // the range [segTail,segHead], return the closest end of that
  // range.

  // YOUR CODE HERE
	vec3 seg = segTail - segHead;
	//normalize seg. note the z component is 0 so not included in normalizing
	seg = (1 / sqrt(pow(seg.x, 2) + pow(seg.y, 2))) * seg;

	vec3 p = segHead - position;
	// dot product
	float proj = p * seg;

	if (proj < 0) {
		return segHead;
	}
	else if (proj > 1) {
		return segTail;
	}
	else {
		return segHead + proj * seg;
	}

  return vec3(0,0,0);
}


// Find the point on the landscape that is closest to 'position'.
//
// This is very inefficiently done.


vec3 Landscape::findClosestPoint( vec3 position )

{
  vec3  closestPoint;
  float minSquaredDistance = MAXFLOAT;

  for (int i=0; i<numVerts-1; i++) {

    vec3 thisClosestPoint = findClosestPoint( position,
					      vec3( landscapeVerts[2*i], landscapeVerts[2*i+1], 0 ),
					      vec3( landscapeVerts[2*(i+1)], landscapeVerts[2*(i+1)+1], 0 ) );

    float thisSquaredDistance = (thisClosestPoint - position) * (thisClosestPoint - position);

    if (thisSquaredDistance < minSquaredDistance) {
      closestPoint = thisClosestPoint;
      minSquaredDistance = thisSquaredDistance;
    }
  }

  return closestPoint;
}


// Landscape model consisting of a path of segments
//
// These are in a ARBITRARY coordinate system and get remapped to the
// world coordinate system when the VAO is set up.

float Landscape::getAltitude(vec3 pos) {
	int i = 0;
	float x0, x1, y0, y1;
	for (; i < numVerts - 1; i++) {
		x0 = landscapeVerts[2 * i];
		x1 = landscapeVerts[2 * (i + 1)];
		if (pos.x > x0 && pos.x < x1) { // if lander lies in segment i
			break;
			
		}
	}
	y0 = landscapeVerts[2 * i + 1];
	y1 = landscapeVerts[2 * (i + 1) + 1];
	
	float m = (y1 - y0) / (x1 - x0);	//slope
	float landHeight = y0 + m * (pos.x - x0);			//y = b + m*x
	float altitude = pos.y - 5.5 - landHeight;

	return altitude;
}


float Landscape::landscapeVerts[] = {
  -463, 866,
  -449, 866,
  -445, 879,
  -433, 880,
  -431, 893,
  -423, 894,
  -422, 927,
  -408, 958,
  -409, 975,
  -402, 996,
  -384, 1010,
  -380, 1030,
  -364, 1050,
  -347, 1060,
  -336, 1040,
  -321, 1020,
  -312, 1010,
  -302, 998,
  -296, 987,
  -281, 976,
  -277, 965,
  -263, 958,
  -251, 942,
  -238, 941,
  -226, 932,
  -213, 932,
  -197, 934,
  -187, 945,
  -185, 956,
  -172, 968,
  -172, 980,
  -160, 992,
  -160, 998,
  -147, 1010,
  -135, 1010,
  -125, 990,
  -114, 985,
  -103, 992,
  -93, 1010,
  -87.3, 1030,
  -64.6, 1040,
  -62.3, 1080,
  -52.1, 1110,
  -55.5, 1120,
  -38.5, 1130,
  -11.3, 1130,
  15.9, 1110,
  21.5, 1100,
  35.1, 1090,
  43.1, 1080,
  57.8, 1070,
  63.5, 1040,
  72.5, 1020,
  82.8, 1010,
  99.8, 999,
  111, 983,
  122, 963,
  130, 934,
  141, 929,
  148, 916,
  152, 903,
  162, 890,
  178, 891,
  190, 881,
  203, 855,
  214, 846,
  220, 820,
  227, 784,
  224, 760,
  229, 733,
  239, 703,
  254, 700,
  258, 687,
  266, 675,
  280, 675,
  282, 686,
  294, 685,
  299, 699,
  306, 699,
  317, 705,
  328, 717,
  331, 743,
  354, 754,
  354, 768,
  366, 793,
  374, 809,
  388, 811,
  399, 823,
  400, 831,
  411, 845,
  414, 856,
  427, 869,
  440, 869,
  442, 906,
  452, 939,
  453, 950,
  464, 950,
  467, 963,
  478, 976,
  485, 995,
  495, 1010,
  506, 1020,
  508, 1050,
  521, 1090,
  520, 1100,
  525, 1110,
  535, 1120,
  546, 1120,
  554, 1150,
  561, 1160,
  575, 1170,
  589, 1180,
  696, 1180,
  702, 1140,
  713, 1120,
  720, 1100,
  728, 1100,
  736, 1070,
  747, 1070,
  759, 1050,
  774, 1050,
  784, 1040,
  804, 1040,
  805, 1050,
  829, 1090,
  829, 1100,
  845, 1110,
  855, 1120,
  864, 1130,
  871, 1150,
  894, 1170,
  951, 1170,
  977, 1180,
  1030, 1180,
  1040, 1150,
  1040, 1110,
  1040, 1090,
  1060, 1090,
  1060, 1070,
  1070, 1060,
  1080, 1050,
  1080, 1030,
  1090, 1020,
  1110, 1020,
  1110, 992,
  1120, 966,
  1130, 962,
  1140, 974,
  1170, 974,
  1180, 963,
  1180, 937,
  1190, 906,
  1210, 902,
  1210, 888,
  1220, 877,
  1230, 865,
  1250, 864,
  1250, 879,
  1270, 878,
  1270, 889,
  1280, 889,
  1280, 927,
  1290, 959,
  1290, 970,
  1300, 991,
  1320, 1000,
  1320, 1030,
  1330, 1040,
  -1
};
