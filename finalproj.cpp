/*
 * Filename: finalproj.cpp
 * 
 * 12-Dec-2012
 * 
 * Description: Final project code. See README for details.
 */

#include "angel/Angel.h"        // Ed Angel's OpenGL Helper Extensions
#include "asset.hpp"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <iostream>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

/*
 * Initialization of space for the objects in the scene to live such
 * that the shaders are able to get at the information
 */
const int NumVertices = 100;
vec4 points[NumVertices];      // Each point
vec4 colors[NumVertices];      // Corresponding colors at points
int  Index = 0;

/*
 * Division factor and Speed factor
 * (for slowing down how fast something moves when using the keyboard)
 *
 * Some good values for this:
 * nVidia fast workstation card             ~ 500.0
 * run-of-the-mill integrated laptop chip   ~ 200.0
 */
#define DIV_FACT 200.0

#define SPD_FACT 0.03

#define MOUSE_SENSITIVITY 0.1

/*
 * The model_view matrix will be used by the shader to draw points,
 * the projection matrix will be used to set which projection the scene has.
 * (The projection will basically be one of two, depending on user preference
 * ... orthographic or perspective). There's a global flag that controls this.
 */
GLuint model_view;  // model-view matrix uniform shader variable location
GLuint projection;  // projection matrix uniform shader variable location
typedef enum { PERSPECTIVE, ORTHOGRAPHIC } viewerModes;
viewerModes globalViewMode;

/*
 * Displacement flags
 *
 * These are kept global because their status is needed in several
 * functions that cannot take arguments. As long as any of them are set
 * to TRUE, the idle function will "move the camera" in their respective
 * direction. On a keyUp event, the flag is set FALSE.
 */
bool cam_forward = false, cam_backward = false, cam_left = false, cam_right = false;
bool lookLeft = false, lookRight = false, lookUp = false, lookDown = false;

/*
 * Current camera position and rotation vectors
 */
vec4 camXYZ;
vec3 camRot;
vec4 camVel;

// Object thing
Asset3ds *scene;

/*
 * Current transformation matrix AND the matrix that will be sent to the
 * shader when an updated projection is requested by the user.
 * (This was done to try and minimize the amount of work the redisplay
 * has to do because of flickering. It didn't work, naturally, so I've
 * just left it here.)
 */
mat4 tran;
mat4 new_projection;

/*
 * Populate the vertices and colors for the scene
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

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                END OF EVIL GLOBALLY-SCOPED PARAMETERS                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

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
 * Sets up the ground area (all green)
 */
void makeGround()
{
        colors[Index] = vertex_colors[3]; points[Index] = vertices[8]; Index++;
        colors[Index] = vertex_colors[3]; points[Index] = vertices[9]; Index++;
        colors[Index] = vertex_colors[3]; points[Index] = vertices[10];Index++;
        colors[Index] = vertex_colors[3]; points[Index] = vertices[11];Index++;
        colors[Index] = vertex_colors[3]; points[Index] = vertices[12];Index++;
        colors[Index] = vertex_colors[3]; points[Index] = vertices[13];Index++;
}

//----------------------------------------------------------------------------
/*
 * OpenGL Initialization Function
 */
void init()
{
/*
        // A grayish background
        glClearColor( 1.0, 1.0, 1.0, 1.0 );

        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);
        GLfloat pos[] = { 0.0, 4.0, 4.0 };
        glLightfv(GL_LIGHT0, GL_POSITION, pos);

        // Create the vertex buffer object from the loaded scene / 3DS file
        scene->CreateVBO();

*/

        colorcube();        // Fill up the unit color cube
        makeGround();       // Show the ground somewhere


        // Create a vertex array object
        GLuint vao;
#ifdef __APPLE__
        glGenVertexArraysAPPLE( 1, &vao );
        glBindVertexArrayAPPLE( vao );
#else
        glGenVertexArrays( 1, &vao );
        glBindVertexArray( vao );
#endif
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
        
        glEnable(GL_DEPTH_TEST);

        // Get a new projection
        // Projection for Homework 2 uses some handy globals so the controlling keys
        // can change matrices.
        GLfloat aspect = GLfloat( glutGet(GLUT_WINDOW_WIDTH) / glutGet(GLUT_WINDOW_HEIGHT) );

        if (globalViewMode == PERSPECTIVE)
                new_projection = Perspective( 45.0, aspect, 0.5, 50.0 );
        else if (globalViewMode == ORTHOGRAPHIC)
                new_projection = Ortho( -1.5, 1.5, -1.5, 1.5, -50.0, 50.0 );

        // Make the transformation matrix
        tran = *(new mat4());

        // Initialize the position and rotation of the camera
        camXYZ.x =  0.0;
        camXYZ.y =  0.0;
        camXYZ.z = -5.0;
        camRot.x =  0.0;
        camRot.y =  0.0;
        camRot.z =  0.0;

        // See if we can use multisampling to smooth things out!

        // A grayish background
        glClearColor( 0.15, 0.15, 0.15, 1.0 );

}

/*
 * The display function
 */
void display( void )
{

//        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//
//        // Reset the viewport
//        glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
//        // Reset the projection and modelview matrix
//        glMatrixMode(GL_PROJECTION);
//        glLoadIdentity();
//        // 10 x 10 x 10 viewing volume
//        glOrtho(-5.0, 5.0, -5.0, 5.0, -5.0, 5.0);
//        glMatrixMode(GL_MODELVIEW);
//        glLoadIdentity();
//
//
//        scene->Draw();

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
         *
         * WARNING: the call to Angel::identity() kept spewing errors so
         * mat.h has been modified to take this "feature" out.
         */
        tran = Angel::identity();
        tran = tran * RotateY(-camRot.y) * RotateX(-camRot.x);

        // Multiplied by the camVel vec4 just to make the syntax error go away.
        camXYZ = camXYZ + tran * camVel;

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

/*
 * Keybindings:
 *   W - move forward in camera's looking vector
 *   S - reverse opposite of camera's looking vector
 *   A - strafe left relative to camera
 *   D - strafe right relative to camera
 *
 *   R - resets the camera positioning and angle at next redraw
 *
 *   P - perspective view with some "easy to look at" defaults
 *   O - orthographic view
 *
 *   X - Exit (so does ESC and Q)
 */
void keyboard( unsigned char key, int x, int y )
{
        switch (key) {
        case 'p':
                globalViewMode = PERSPECTIVE;
                break;
        case 'o':
                globalViewMode = ORTHOGRAPHIC;
                break;
        case 'w':
                cam_forward = true;
                break;
        case 's':
                cam_backward = true;
                break;
        case 'a':
                cam_left = true;
                break;
        case 'd':
                cam_right = true;
                break;

        case 'i':
                lookUp = true;
                break;
        case 'k':
                lookDown = true;
                break;
        case 'j':
                lookLeft = true;
                break;
        case 'l':
                lookRight = true;
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
                cam_forward = false;
                break;
        case 's':
                cam_backward = false;
                break;
        case 'a':
                cam_left = false;
                break;
        case 'd':
                cam_right = false;
                break;
        case 'i':
                lookUp = false;
                break;
        case 'k':
                lookDown = false;
                break;
        case 'j':
                lookLeft = false;
                break;
        case 'l':
                lookRight = false;
                break;
        }
        glutPostRedisplay();
}


void lookAt( int x, int y )
{
        // Thanks to the Simeandroids group for this. Solid mouse
        // handling work there, folks!
        // make sure we're not in the center
        if (x != 255 || y != 255)
        {
            camRot.y -= (255 - x) * MOUSE_SENSITIVITY;

            // lock the up and down look to no more than 90 degrees
            if (camRot.x - (255 - y) * MOUSE_SENSITIVITY <= 90 &&
                    camRot.x - (255 - y) * MOUSE_SENSITIVITY >= -90)
            {
                camRot.x -= (255 - y) * MOUSE_SENSITIVITY;
            }
            
            glutWarpPointer(255, 255);

        }
}

/*
 * Helper function to compute radians from degrees
 * I forced-inlined it for efficiency purposes.
 */
inline GLfloat toRadians( GLfloat degrees ) { return degrees * (M_PI / 180.0); }

/*
 * Idle callback
 */
void idle(void)
{
        if (cam_forward)
                camVel.z = SPD_FACT;
        else if (cam_backward)
                camVel.z = -SPD_FACT;
        else
                camVel.z = 0.0;

        if (cam_left)
                camVel.x = SPD_FACT;
        else if (cam_right)
                camVel.x = -SPD_FACT;
        else
                camVel.x = 0.0;

        if (lookUp)
                camRot.x -= 0.2;
        if (lookDown)
                camRot.x += 0.2;
        if (lookLeft)
                camRot.y -= 0.2;
        if (lookRight)
                camRot.y += 0.2;

        // Need to update the frame buffer
        glutPostRedisplay();
}

/*****************************************************************************
 * Main
 *****************************************************************************/
int main( int argc, char **argv )
{
        /*
         * Bunch of init code... it's all from Angel's examples.
         */
        glutInit( &argc, argv );
        glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH );
        glutInitWindowSize( 512, 512 );
        glutCreateWindow( "final" );

#ifdef __APPLE__
        CGSetLocalEventsSuppressionInterval(0.0);
        //CGAssociateMouseAndMouseCursorPosition(false);
#else
        glewInit();
#endif

        // Try to load a 3DS model for the scene
        scene = new Asset3ds("../l3ds_tut/monkey.3ds");

        init();

        /*
         * Initialize the GLUT callbacks
         */
        glutDisplayFunc( display );
        glutKeyboardUpFunc( keyUp );
        glutKeyboardFunc( keyboard );
        glutIdleFunc( idle );
        glutPassiveMotionFunc(lookAt);

        // Make the cursor disappear and then center it to capture motion.
        glutSetCursor(GLUT_CURSOR_NONE);
        glutWarpPointer(255, 255);



        /*
         * Enter the GLUT event loop for input processing
         */
        glutMainLoop();
        return 0;
}
