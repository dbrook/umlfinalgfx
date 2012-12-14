/*
 * Filename: finalproj.cpp
 * 
 * 12-Dec-2012
 * 
 * Description: Final project code. See README for details.
 */

#include "angel/Angel.h"        // Ed Angel's OpenGL Melting-Pot

#include <iostream>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

// 6 faces/cube * 2 tri/face * 2 vert/tri + 2 triangle for the gnd = 40 vertices/cube
const int NumVertices = 100;

// Updated to be vec4s
// These are the globally-accessible points and colors
vec4 points[NumVertices];
vec4 colors[NumVertices];

// Division factor
// (for slowing down how fast something moves when using the keyboard)
#define DIV_FACT 200.0

// What does this do?
int  Index = 0;

GLuint model_view;  // model-view matrix uniform shader variable location
GLuint projection;  // projection matrix uniform shader variable location

// Displacement options
bool forward = false, backward = false, left = false, right = false;

// Starting cube positions -- DEPRECATED IN FAVOR OF camXYZ
GLfloat xPos = 0.0, yPos = 0.0, zPos = -4.0;

// Starting cube rotation --  DEPRECATED IN FAVOR OF camRot
GLfloat thetaX = 0.0, thetaY = 0.0, thetaZ = 0.0;

// Camera position and rotation
vec4 camXYZ;
vec3 camRot;

// Current transformation matrix
mat4 tran;

// Globals that can be modified to change projection matrices
typedef enum { PERSPECTIVE, ORTHOGRAPHIC } viewerModes;
viewerModes globalViewMode;

// Declare this globally. Trying to minimize the amount of work redisplay has
mat4 new_projection;


//------------------------------------------------------------------------------
/*
 * Vertices of a unit cube centered at origin, sides aligned with axes
 */
point4 vertices[14] = {
        point4( -0.5, -0.5,  0.5, 1.0 ),
        point4( -0.5,  0.5,  0.5, 1.0 ),
        point4(  0.5,  0.5,  0.5, 1.0 ),
        point4(  0.5, -0.5,  0.5, 1.0 ),
        point4( -0.5, -0.5, -0.5, 1.0 ),
        point4( -0.5,  0.5, -0.5, 1.0 ),
        point4(  0.5,  0.5, -0.5, 1.0 ),
        point4(  0.5, -0.5, -0.5, 1.0 ),

        /*
         * Ground coordinates
         */
        point4( -2, -2, -2, 1.0 ),
        point4( -2, -2,  2, 1.0 ),
        point4(  2, -2,  2, 1.0 ),
        point4( -2, -2, -2, 1.0 ),
        point4(  2, -2,  2, 1.0 ),
        point4(  2, -2, -2, 1.0 )
};

/*
 * The unit color cube's RGBA colors list
 */
color4 vertex_colors[8] = {
        color4( 0.0, 0.0, 0.0, 1.0 ),  // black
        color4( 1.0, 0.0, 0.0, 1.0 ),  // red
        color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
        color4( 0.0, 1.0, 0.0, 1.0 ),  // green
        color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
        color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
        color4( 1.0, 1.0, 1.0, 1.0 ),  // white
        color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

//-----------------------------------------------------------------------------
/*
 * Populate points and colors for one face of the cube
 */
void quad( int a, int b, int c, int d )
{
        colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
        colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; Index++;
        colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
        colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
        colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
        colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; Index++;
}

/* 
 * Sets up each face of the color cube w/ the quad() helper function
 */
void colorcube()
{
        quad( 1, 0, 3, 2 );
        quad( 2, 3, 7, 6 );
        quad( 3, 0, 4, 7 );
        quad( 6, 5, 1, 2 );
        quad( 4, 5, 6, 7 );
        quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------
/*
 * Sets up a black ground area.
 */
void makeGround()
{
        colors[Index] = vertex_colors[0]; points[Index] = vertices[8]; Index++;
        colors[Index] = vertex_colors[0]; points[Index] = vertices[9]; Index++;
        colors[Index] = vertex_colors[0]; points[Index] = vertices[10];Index++;
        colors[Index] = vertex_colors[0]; points[Index] = vertices[11];Index++;
        colors[Index] = vertex_colors[0]; points[Index] = vertices[12];Index++;
        colors[Index] = vertex_colors[0]; points[Index] = vertices[13];Index++;
}

//----------------------------------------------------------------------------
/*
 * OpenGL Initialization Function
 */
void init()
{
        colorcube();        // Fill up the unit color cube
        makeGround();       // Show the ground somewhere
        
        // Create a vertex array object
        GLuint vao;
        glGenVertexArrays( 1, &vao );
        glBindVertexArray( vao );
        
        // Create and initialize a buffer object
        GLuint buffer;
        glGenBuffers( 1, &buffer );
        glBindBuffer( GL_ARRAY_BUFFER, buffer );
        glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW );
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
        glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
        
        // Load shaders and use the resulting shader program
        GLuint program = InitShader( "vshaderTest.glsl", "fshaderTest.glsl" );
        glUseProgram( program );
        
        // set up vertex arrays
        GLuint vPosition = glGetAttribLocation( program, "vPosition" );
        glEnableVertexAttribArray( vPosition );
        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
        
        GLuint vColor = glGetAttribLocation( program, "vColor" ); 
        glEnableVertexAttribArray( vColor );
        glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
                               BUFFER_OFFSET(sizeof(points)) );
        
        model_view = glGetUniformLocation( program, "model_view" );
        projection = glGetUniformLocation( program, "projection" );
        
        glEnable( GL_DEPTH_TEST );

        // Get a new projection
        // Projection for Homework 2 uses some handy globals so the controlling keys
        // can change matrices.
        GLfloat aspect = GLfloat( glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT) );

        if (globalViewMode == PERSPECTIVE)
                new_projection = Perspective( 45.0, aspect, 0.5, 15.0 );
        else if (globalViewMode == ORTHOGRAPHIC)
                new_projection = Ortho( -1.5, 1.5, -1.5, 1.5, -10.0, 10.0 );

        // Make the transformation matrix
        //tran = *(new mat4());

        // Initialize the position and rotation of the camera
        camXYZ.x =  0.0;
        camXYZ.y =  0.0;
        camXYZ.z = -5.0;
        camRot.x =  0.0;
        camRot.y =  0.0;
        camRot.z =  0.0;

        // See if we can use multisampling to smooth things out!

        // A grayish background
        glClearColor( 0.25, 0.25, 0.25, 1.0 ); 
}

//----------------------------------------------------------------------------
/*
 * The display function
 */
void display( void )
{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        // Get a new projection
        // Projection for Homework 2 uses some handy globals so the controlling keys
        // can change matrices.
        GLfloat aspect = GLfloat( glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT) );

        if (globalViewMode == PERSPECTIVE)
                new_projection = Perspective( 45.0, aspect, 0.5, 15.0 );
        else if (globalViewMode == ORTHOGRAPHIC)
                new_projection = Ortho( -1.5, 1.5, -1.5, 1.5, -10.0, 10.0 );

        /*
         * Make one big matrix to send to the shader with the current state
         * variable values.
         */
        tran = Angel::identity();
        tran = tran * RotateY(-camRot.y) * RotateX(-camRot.x);
        //camXYZ = camXYZ + tran;

        tran = Angel::identity();
        tran = tran * RotateX(camRot.x) * RotateY(camRot.y);
        tran = tran * Translate(camXYZ.x, camXYZ.y, camXYZ.z);

        /* 
         * Update the model view matrix with the translation.
         * Then update the projection matrix.
         */
        glUniformMatrix4fv( model_view, 1, GL_TRUE, tran );
        glUniformMatrix4fv( projection, 1, GL_TRUE, new_projection );
        glDrawArrays( GL_TRIANGLES, 0, NumVertices );
        glutSwapBuffers();
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y )
{
        /*
         * Keybindings:
         *   W - move forward in camera's looking vector
         *   S - reverse opposite of camera's looking vector
         *   A - strafe left relative to camera
         *   D - strafe right relative to camera
         *
         *   P - perspective view with some "easy to look at" defaults
         *   O - orthographic view
         *
         *   X - Exit (so does ESC and Q)
         */
        switch (key) {
        case 'p':
                globalViewMode = PERSPECTIVE;
                break;
        case 'o':
                globalViewMode = ORTHOGRAPHIC;
                break;
        case 'w':
                forward = true;
                break;
        case 's':
                backward = true;
                break;
        case 'a':
                left = true;
                break;
        case 'd':
                right = true;
                break;


        case 'r':
                // Reset field of view!
                camXYZ.y = camXYZ.x = 0.0;
                camXYZ.z = -5.0;
                camRot.x = camRot.y = camRot.z = 0.0;
                break;

                /* EXIT BLOCK */
        case 033:
        case 'q':
        case 'x':
                exit( EXIT_SUCCESS );
                break;
        }

        // Get a new projection
        // Projection for Homework 2 uses some handy globals so the controlling keys
        // can change matrices.
        GLfloat aspect = GLfloat( glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT) );

        if (globalViewMode == PERSPECTIVE)
                new_projection = Perspective( 45.0, aspect, 0.5, 9.0 );
        else if (globalViewMode == ORTHOGRAPHIC)
                new_projection = Ortho( -1.5, 1.5, -1.5, 1.5, -10.0, 10.0 );

        // Update drawing
        glutPostRedisplay();
}

/* 
 * Keyboard up callback
 */
void keyUp( unsigned char key, int x, int y )
{
        switch (key) {
        case 'w':
                forward = false;
                break;
        case 's':
                backward = false;
                break;
        case 'a':
                left = false;
                break;
        case 'd':
                right = false;
                break;
        }
        glutPostRedisplay();
}

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y)
{
        int hmiddle, wmiddle;

        // Figure out where the center of the frame buffer is
        wmiddle = glutGet(GLUT_WINDOW_WIDTH) / 2;
        hmiddle = glutGet(GLUT_WINDOW_HEIGHT) / 2;

        // Make the center look straight onto the cube
        x -= wmiddle;
        y -= hmiddle;

        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
                thetaY = -x;
                thetaX = y;
        }

        glutPostRedisplay();
}


// Helper function to compute radians from degrees
GLfloat toRadians( GLfloat degrees ) { return degrees * (M_PI / 180.0); }

/*
 * Idle callback
 */
void idle(void)
{
        if (forward) {
                camXYZ.x -= sin(camRot.y) / DIV_FACT;
                camXYZ.z += cos(camRot.y) / DIV_FACT;
                camXYZ.y += sin(camRot.x) / DIV_FACT;
        }

        if (backward) {
                camXYZ.x += sin(camRot.y) / DIV_FACT;
                camXYZ.z -= cos(camRot.y) / DIV_FACT;
                camXYZ.y -= sin(camRot.x) / DIV_FACT;
        }

        if (left) {
                camXYZ.x += cos(camRot.y) / DIV_FACT;
                camXYZ.z += sin(camRot.y) / DIV_FACT;
        }

        if (right) {
                camXYZ.x -= cos(camRot.y) / DIV_FACT;
                camXYZ.z -= sin(camRot.y) / DIV_FACT;
        }

        // Need to update the frame buffer
        glutPostRedisplay();

}

//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
        glutInit( &argc, argv );
        glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH );
        glutInitWindowSize( 512, 512 );
        glutCreateWindow( "Color Cube Projections" );

        glewInit();
	
        init();

        glutDisplayFunc( display );
	glutMouseFunc( mouse );
        
        glutKeyboardUpFunc( keyUp );
        glutKeyboardFunc( keyboard );

        glutIdleFunc( idle );

        glutMainLoop();

        return 0;
}
