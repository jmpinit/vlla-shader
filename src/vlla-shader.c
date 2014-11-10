#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "kiss_fftr.h"
#include "esUtil.h"

#define BUFFER_SIZE 2048

#define WIDTH 60
#define HEIGHT 32
#define NUM_PIXELS (WIDTH*HEIGHT)

#define FFT_SIZE 2048
kiss_fft_cfg fft_cfg;
kiss_fft_cpx *fft_in;
kiss_fft_cpx *fft_out;
float log_pwr_fft[FFT_SIZE];

float shaderTime = 0.0f;
GLubyte* fft_tex;

int yzero = 80;
float scale = -1.0f;
float outerscale = 1.0f;

typedef struct {
    // Handle to a program object
    GLuint programObject;
} UserData;

int db_to_pixel(float dbfs) {
    return yzero + (int)(-dbfs*scale);
}

void shiftFFT(unsigned int amt) {
    if(amt > FFT_SIZE) printf("\nFFT shifted too far.");

    int i;
    for(i = 0; i < FFT_SIZE - amt; i++) {
        fft_in[i].r = fft_in[i+amt].r;
        fft_in[i].i = fft_in[i+amt].i;
    }
}

void updateFFT() {
    char buffer[BUFFER_SIZE];
    int len = read(STDIN_FILENO, buffer, BUFFER_SIZE);

    shiftFFT(len);

    int i;
    for(i=0; i < len; i += 2) {
        fft_in[FFT_SIZE-len/2 + i/2].r = (float)(buffer[i]<<1) / 255.f;
        fft_in[FFT_SIZE-len/2 + i/2].i = (float)(buffer[i+1]<<1) / 255.f;
    }

    kiss_fft(fft_cfg, fft_in, fft_out);

    kiss_fft_cpx pt;
    for(i = 0; i < FFT_SIZE/2; i++) {
        // normalize and convert to dBFS
        pt.r = fft_out[i].r / FFT_SIZE;
        pt.i = fft_out[i].i / FFT_SIZE;
        float pwr = pt.r * pt.r + pt.i * pt.i;

        log_pwr_fft[i] = 10.f * log10(pwr + 1.0e-20f);

        int v = outerscale*db_to_pixel(log_pwr_fft[i]);
        //if(i < 16)
        //    printf("%d\t", v);
        v = (v < 0)?      0       : v;
        v = (v > 0xff)?   0xff    : v;
        fft_tex[(i+4*60)*4] = v;
    }
}

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
    /*GLbyte vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";

      GLbyte fShaderStr[] =  
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 0.0, 0.0, 1.0, gl_FragCoord.xf );\n"
      "}                                            \n";*/

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

    int timeLoc = glGetUniformLocation(userData->programObject, "time");
    glUniform1f(timeLoc, shaderTime);

    int resolutionLoc = glGetUniformLocation(userData->programObject, "resolution");
    glUniform2f(resolutionLoc, 60.0, 32.0);

    // AUDIO

    GLuint textureID[1];
    glGenTextures(1, textureID);

    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FFT_SIZE, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)fft_tex);

    GLint baseImageLoc = glGetUniformLocation(userData->programObject, "fft");
    glUniform1i(baseImageLoc, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);

    updateFFT();

    int mouseLoc = glGetUniformLocation(userData->programObject, "mouse");
    glUniform2f(mouseLoc, 0.f, 0.f);

    // Use the program object
    glUseProgram(userData->programObject);

    shaderTime += 1.0;

    // Load the vertex data
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
    glEnableVertexAttribArray ( 0 );

    glDrawArrays ( GL_TRIANGLE_STRIP, 0, 4 );
}

int main ( int argc, char *argv[] ) {
    fft_cfg = kiss_fft_alloc(FFT_SIZE, FALSE, NULL, NULL);
    fft_in = (kiss_fft_cpx*)malloc(FFT_SIZE * sizeof(kiss_fft_cpx));
    fft_out = (kiss_fft_cpx*)malloc(FFT_SIZE / 2 * sizeof(kiss_fft_cpx) + 1);

    fft_tex = (GLubyte*)malloc(4 * FFT_SIZE / 2 * sizeof(GLubyte));

    // debug pattern
    int i;
    for(i=0; i < 4 * FFT_SIZE / 2; i++)
        fft_tex[i] = 0;

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
