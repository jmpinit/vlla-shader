#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include <alsa/asoundlib.h>

#include "kiss_fftr.h"
#include "esUtil.h"

#define BUFFER_SIZE 8000
#define DOWNSAMPLE 4
#define FFT_SIZE (BUFFER_SIZE / DOWNSAMPLE)

#define WIDTH 60
#define HEIGHT 32
#define NUM_PIXELS (WIDTH*HEIGHT)

snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *hw_params;
unsigned int sampleRate;

uint8_t audiobuf[BUFFER_SIZE];
int16_t samplebuf[FFT_SIZE];
kiss_fft_cfg fft_cfg;
kiss_fft_cpx *fft_in;
kiss_fft_cpx *fft_out;
float log_pwr_fft[FFT_SIZE];

float shaderTime = 0.0f;
GLubyte* fft_tex;

char* fragmentShaderFilename;
GLuint fragmentShader;

GLuint fragTextures[2];

int yzero = 60;
float scale = -1.0f;
float outerscale = 1.0f;

typedef struct {
    // Handle to a program object
    GLuint programObject;
} UserData;

pthread_t tid[1];
bool running = true;

void shiftFFT(unsigned int amt) {
    if(amt > FFT_SIZE) printf("\nFFT shifted too far.");

    int i;
    for(i = 0; i < FFT_SIZE - amt; i++) {
        fft_in[i].r = fft_in[i+amt].r;
        fft_in[i].i = fft_in[i+amt].i;
    }
}

void* consumeAudio(void *arg) {
    int i;

    while(running) {
        // record some audio data
        int numFrames = BUFFER_SIZE/2;
        int sampleCount = snd_pcm_readi(capture_handle, audiobuf, numFrames);
        //int sampleCount = read(STDIN_FILENO, audiobuf, BUFFER_SIZE)/2;

        if(sampleCount < 0) {
            fprintf (stderr, "read from audio interface failed (%s)\n", snd_strerror (sampleCount));
        } else {
            // downsample TODO FIR filter
            //printf("\n");
            for(i = 0; i < sampleCount; i += DOWNSAMPLE) {
                samplebuf[i/DOWNSAMPLE] = (audiobuf[i*2]) | (audiobuf[i*2+1] << 8);

                if(i < 16*DOWNSAMPLE && (int)shaderTime % 100 == 0)
                    printf("%d\t", samplebuf[i/DOWNSAMPLE]);
            }

            int downsampleCount = sampleCount / DOWNSAMPLE;

            shiftFFT(downsampleCount);

            for(i=0; i < downsampleCount; i += 2) {
                fft_in[FFT_SIZE-(downsampleCount + i)/2].r = (float)(samplebuf[i]) / 32767.f;
                fft_in[FFT_SIZE-(downsampleCount + i)/2].i = (float)(samplebuf[i+1]) / 32767.f;
            }
            //printf("\n");

            kiss_fft(fft_cfg, fft_in, fft_out);
        }
    }
    return NULL;
}

int db_to_pixel(float dbfs) {
    return yzero + (int)(-dbfs*scale);
}

void updateFFT() {
    int i;

    kiss_fft_cpx pt;
    for(i = 0; i < FFT_SIZE/2; i++) {
        // normalize and convert to dBFS
        pt.r = fft_out[i].r / FFT_SIZE;
        pt.i = fft_out[i].i / FFT_SIZE;
        float pwr = pt.r * pt.r + pt.i * pt.i;

        log_pwr_fft[i] = 10.f * log10(pwr + 1.0e-20f);

        int v = outerscale*db_to_pixel(log_pwr_fft[i]);
        v = (v < 0)?      0       : v;
        v = (v > 0xff)?   0xff    : v;
        //if(i < 16)
        //    printf("%d\t", v);
        fft_tex[4*i] = v;
    }
    /*for(i=0; i < FFT_SIZE/4; i++) {
        if((int)shaderTime / 16 % 2 == 0)
            fft_tex[4*i] = 0xff;
        else
            fft_tex[4*i] = 0x11;
    }*/
}

// create a shader object, load the shader source, and
// compile the shader.
GLuint LoadShader(GLenum type, const char *shaderSrc) {
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);

    if(shader == 0)
        return 0;

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1) {
            char* infoLog = malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            esLogMessage("Error compiling shader:\n%s\n", infoLog);            

            free(infoLog);
        }

        glDeleteShader(shader);
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
int Init(ESContext *esContext) {
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
      "  gl_FragColor = vec4(0.0, 0.0, 1.0, gl_FragCoord.xf);\n"
      "}                                            \n";*/

    GLuint programObject;
    GLint linked;

    // Load the vertex shader
    GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, loadfile("vert.glsl"));

    // Create the program object
    programObject = glCreateProgram();

    if(programObject == 0)
        return 0;

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Bind vPosition to attribute 0   
    glBindAttribLocation(programObject, 0, "vPosition");

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if(!linked) {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1) {
            char* infoLog = malloc (sizeof(char) * infoLen);

            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            esLogMessage("Error linking program:\n%s\n", infoLog);            

            free(infoLog);
        }

        glDeleteProgram(programObject);
        return GL_FALSE;
    }

    // Store the program object
    userData->programObject = programObject;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    return GL_TRUE;
}

void checkError(const char* msg) {
    int err = glGetError();
    switch(err) {
        case GL_INVALID_ENUM:
            printf("%s\ninvalid enum\n", msg);
            break;
        case GL_INVALID_VALUE:
            printf("%s\ninvalid value\n", msg);
            break;
        case GL_INVALID_OPERATION:
            printf("%s\ninvalid op\n", msg);
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            printf("%s\ninvalid framebuffer op\n", msg);
            break;
        case GL_OUT_OF_MEMORY:
            printf("%s\nout of memory\n", msg);
            break;
    }
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(ESContext *esContext) {
    UserData *userData = esContext->userData;

    // TIME

    int timeLoc = glGetUniformLocation(userData->programObject, "time");
    glUniform1f(timeLoc, shaderTime);
    shaderTime += 1.0;

    // RESOLUTION

    int resolutionLoc = glGetUniformLocation(userData->programObject, "resolution");
    glUniform2f(resolutionLoc, 60.0, 32.0);

    // MOUSE

    int mouseLoc = glGetUniformLocation(userData->programObject, "mouse");
    glUniform2f(mouseLoc, 0.f, 0.f);

    // AUDIO

    updateFFT();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FFT_SIZE / 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)fft_tex);

    GLint baseImageLoc = glGetUniformLocation(userData->programObject, "fft");
    glUniform1i(baseImageLoc, 0);

    glActiveTexture(GL_TEXTURE0);

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex data
    GLfloat vVertices[] = { 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, -1.0f, 0.0f };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void cleanup() {
    running = false;
    snd_pcm_close(capture_handle);
}

void consume_parameters(int argc, char *argv[]) {
    if(argc != 2) {
        printf("usage: vlla-shader <fragment shader source file>\n");
        exit(1);
    }

    fragmentShaderFilename = argv[1];

    if(access(fragmentShaderFilename, F_OK) == -1) {
        fprintf(stderr, "Fragment shader source file does not exist.\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    int i;
    int err;

    consume_parameters(argc, argv);
    
    // fft
    fft_cfg = kiss_fft_alloc(FFT_SIZE, FALSE, NULL, NULL);
    fft_in = (kiss_fft_cpx*)malloc(FFT_SIZE * sizeof(kiss_fft_cpx));
    fft_out = (kiss_fft_cpx*)malloc(FFT_SIZE * sizeof(kiss_fft_cpx));

    fft_tex = (GLubyte*)malloc(4 * FFT_SIZE / 2 * sizeof(GLubyte));

    if(fft_in == NULL || fft_out == NULL || fft_tex == NULL) {
        printf("Not enough memory.\n");
        exit(1);
    }

    // audio
    char* device = "mic";
    if ((err = snd_pcm_open (&capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n", 
                device,
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    sampleRate = 44100;
    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &sampleRate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                snd_strerror (err));
        exit (1);
    }
    printf("sample rate is %d Hz.\n", sampleRate);

    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    err = pthread_create(&(tid[0]), NULL, &consumeAudio, NULL);
    if (err != 0)
        printf("can't create thread :[%s]\n", strerror(err));
    else
        printf("audio thread created successfully\n");

    printf("created recording stream.\n");
    printf("FFT_SIZE=%d\n", FFT_SIZE);

    // debug pattern
    for(i=0; i < 4 * FFT_SIZE / 2; i++) {
        fft_tex[i] = 0;
        //if(i%4==2)
        //    fft_tex[i] = 255;
    }

    ESContext esContext;
    UserData  userData;

    esInitContext(&esContext);
    esContext.userData = &userData;

    esCreateWindow(&esContext, "VLLA", 60, 32, ES_WINDOW_RGB);

    if(!Init(&esContext))
        return 0;

    esRegisterDrawFunc(&esContext, Draw);

    // shader setup
    glGenTextures(2, fragTextures);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, loadfile(fragmentShaderFilename));

    esMainLoop(&esContext);
}
