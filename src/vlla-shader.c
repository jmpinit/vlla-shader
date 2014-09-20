#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "esUtil.h"
#include "portaudio.h"

#define SAMPLE_RATE (44100)

typedef struct
{
   // Handle to a program object
   GLuint programObject;

} UserData;

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = malloc (sizeof(char) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         esLogMessage ( "Error compiling shader:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

char* loadfile(char* filename) {
    char* buffer = 0;
    long length;
    FILE* f = fopen (filename, "rb");

    if(f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc(length);
        if(buffer) {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }

    if(buffer) {
        return buffer;
    } else {
        return 0;
    }
}

///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
   esContext->userData = malloc(sizeof(UserData));

   UserData *userData = esContext->userData;

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, loadfile("vert.glsl") );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, loadfile("frag.glsl") );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Bind vPosition to attribute 0   
   glBindAttribLocation ( programObject, 0, "vPosition" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = malloc (sizeof(char) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         esLogMessage ( "Error linking program:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return GL_FALSE;
   }

   // Store the program object
   userData->programObject = programObject;

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//

typedef struct {
    float sample;
} shaderAudioData;

PaStream *stream;
PaError err;
static shaderAudioData data;

float shaderTime = 0.0f;

void Draw ( ESContext *esContext ) {
   UserData *userData = esContext->userData;
   GLfloat vVertices[] = {  1.0f,  1.0f, 0.0f,
                           -1.0f,  1.0f, 0.0f,
                            1.0f, -1.0f, 0.0f,
                           -1.0f, -1.0f, 0.0f };

   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   int timeLoc = glGetUniformLocation(userData->programObject, "iGlobalTime");
   int resLoc = glGetUniformLocation(userData->programObject, "iResolution");
   int channel0Loc = glGetUniformLocation(userData->programObject, "iChannel0");
   glUniform1f(timeLoc, shaderTime);
   glUniform2f(resLoc, 60.0f, 32.0f);
   glUniform1f(channel0Loc, sample);

   // Use the program object
   glUseProgram ( userData->programObject );

   shaderTime += 0.5;

   // Load the vertex data
   glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
   glEnableVertexAttribArray ( 0 );

   glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 );
}

static int audioCallback( const void *inputBuffer, void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData ) {
    /* Cast data passed through stream to our structure. */
    shaderAudioData *data = (shaderAudioData*)userData; 
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */

    float total = 0;
    for( i=0; i<framesPerBuffer; i++ ) {
        total += out[i];
    }
    data->sample = total / framesPerBuffer;

    return 0;
}

void cleanup() {
    err = Pa_StopStream( stream );
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );

    err = Pa_Terminate();
    if( err != paNoError )
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
}

int main ( int argc, char *argv[] )
{
    err = Pa_Initialize();
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        exit(1);
    }
    atexit(cleanup);

     /* Open an audio I/O stream. */
     err = Pa_OpenDefaultStream( &stream,
             1,          /* input channels */
             0,          /* no output */
             paFloat32,  /* 32 bit floating point output */
             SAMPLE_RATE,
             256,        /* frames per buffer, i.e. the number
                            of sample frames that PortAudio will
                            request from the callback. Many apps
                            may want to use
                            paFramesPerBufferUnspecified, which
                            tells PortAudio to pick the best,
                            possibly changing, buffer size.*/
             audioCallback, /* this is your callback function */
             &data ); /*This is a pointer that will be passed to
                        your callback*/
    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        exit(1);
    }

    err = Pa_StartStream(stream);

    if( err != paNoError ) {
        printf(  "PortAudio error: %s\n", Pa_GetErrorText( err ) );
        exit(1);
    }

   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   esCreateWindow ( &esContext, "Hello Triangle", 60, 32, ES_WINDOW_RGB );

   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );

   esMainLoop ( &esContext );
}
