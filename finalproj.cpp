/*
 * Filename: finalproj.cpp
 * 
 * 12-Dec-2012
 * 
 * Description: Final project code. See README for details.
 */

#include "angel/Angel.h"        // Ed Angel's OpenGL Helper Extensions

#include <iostream>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

/*
 * Initialization of space for the objects in the scene to live
 */
const int NumVertices = 100;
vec4 points[NumVertices];         // Each point
vec4 colors[NumVertices];         // Corresponding colors at points
int  Index = 0;

/*
 * Division factor
 * (for slowing down how fast something moves when using the keyboard)
 *
 * Some good values for this:
 * nVidia fast workstation card             ~ 500.0
 * run-of-the-mill integrated laptop chip   ~ 200.0
 */
#define DIV_FACT 200.0

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
bool forward = false, backward = false, left = false, right = false;

/*
 * Current camera position and rotation vectors
 */
vec4 camXYZ;
vec3 camRot;

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
        glClearColor( 0.15, 0.15, 0.15, 1.0 );
}

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
         *
         * WARNING: the call to Angel::identity() kept spewing errors so
         * mat.h has been modified to take this "feature" out.
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

/*
 * Keybindings:
 *   W - move forward in camera's looking vector
 *   S - reverse opposite of camera's looking vector
 *   A - strafe left relative to camera
 *   D - strafe right relative to camera
 *
 *   R - resets the camera positioning and angle at next redraw
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

        }

        glutPostRedisplay();
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


int main( int argc, char **argv )
{
        /*
         * Bunch of init code... it's all from Angel's examples.
         */
        glutInit( &argc, argv );
        glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH );
        glutInitWindowSize( 512, 512 );
        glutCreateWindow( "final" );
        glewInit();
        init();

        /*
         * Initialize the GLUT callbacks
         */
        glutDisplayFunc( display );
        glutMouseFunc( mouse );
        glutKeyboardUpFunc( keyUp );
        glutKeyboardFunc( keyboard );
        glutIdleFunc( idle );

        /*
         * Enter the GLUT event loop for input processing
         */
        glutMainLoop();
        return 0;
}
