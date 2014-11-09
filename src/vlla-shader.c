#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "kiss_fftr.h"
#include "esUtil.h"

#define WIDTH 60
#define HEIGHT 32
#define NUM_BYTES (WIDTH*HEIGHT)

#define FFT_SIZE NUM_BYTES
kiss_fftr_cfg fft_cfg;
kiss_fft_scalar *fft_in;
kiss_fft_cpx *fft_out;
float log_pwr_fft[FFT_SIZE];

GLubyte fft_tex[60*32*4];

int yzero = 0;
float scale = 0.05f;

typedef struct {
    // Handle to a program object
    GLuint programObject;
} UserData;

int db_to_pixel(float dbfs) {
    return yzero + (int)(-dbfs*scale);
}

void updateFFT() {
    static int pos;

    char buffer[FFT_SIZE];
    int len = read(STDIN_FILENO, buffer, FFT_SIZE);

    int i;
    for(i=0; i < len; i++) {
        int k = (pos+i)%NUM_BYTES;
        fft_tex[k*4] = buffer[i] << 2;
    }
    pos += len;

    /*int i;
    for(i=0; i < FFT_SIZE; i++) {
        fft_in[i] = (float)buffer[i] / 256.f;
    }

    kiss_fftr(fft_cfg, fft_in, fft_out);

    kiss_fft_cpx pt;
    for(i = 0; i < FFT_SIZE; i++) {
        // shift, normalize and convert to dBFS
        if(i < FFT_SIZE / 2) {
            pt.r = fft_out[FFT_SIZE/2+i].r / FFT_SIZE;
            pt.i = fft_out[FFT_SIZE/2+i].i / FFT_SIZE;
        } else {
            pt.r = fft_out[i-FFT_SIZE/2].r / FFT_SIZE;
            pt.i = fft_out[i-FFT_SIZE/2].i / FFT_SIZE;
        }
        float pwr = pt.r * pt.r + pt.i * pt.i;

        log_pwr_fft[i] = 10.f * log10(pwr + 1.0e-20f);
    }

    for(i=0; i < FFT_SIZE; i++) {
        fft_tex[i*4] = db_to_pixel(log_pwr_fft[i]);
    }*/
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

    int timeLoc = glGetUniformLocation(userData->programObject, "time");
    glUniform1f(timeLoc, shaderTime);

    int resolutionLoc = glGetUniformLocation(userData->programObject, "resolution");
    glUniform2f(resolutionLoc, 60.0, 32.0);

    // AUDIO

    GLuint textureID[1];
    glGenTextures(1, textureID);

    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 60, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)fft_tex);

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
    fft_cfg = kiss_fftr_alloc(FFT_SIZE, FALSE, NULL, NULL);
    fft_in = (kiss_fft_scalar*)malloc(FFT_SIZE * sizeof(kiss_fft_scalar));
    fft_out = (kiss_fft_cpx*)malloc(FFT_SIZE / 2 * sizeof(kiss_fft_cpx) + 1);

    /*// debug pattern
    int i;
    for(i=0; i < 60*32; i++) {
        if(i%2==0)
            fft_tex[i*4+1] = 255;
    }*/

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
