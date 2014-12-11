#version 100
#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.1415
#define SPEED 7.0
#define SPEEDS 1.0
#define SIZE 2.0

#define LOW 0.0
#define HIGH 100.0
#define STEP 10.0
#define THICKNESS 0.005

uniform sampler2D fft;
uniform sampler2D last;

uniform float time;
uniform float _bass;
uniform float _midrange;
uniform float _treble;
uniform vec2 mouse;
uniform vec2 resolution;

float snd(float i) {
        return texture2D(fft, vec2(i, 0.0)).r ;
}

float freqRange() {
    float total = 0.0;
    for (float i = LOW; i <= HIGH; i += STEP){
        total += snd(i / 1024.0);
    }
    return total / (( HIGH - LOW ) / STEP) ;
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 color(float n) {
    float h = 3.1415*cos(n);
    float v = sqrt(n/10.0);
    float s = 1.0 - v;

    vec3 rgb = (hsv2rgb(vec3(h,s,v)));

    return vec4(rgb,0.0);
}

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;

    float bass = freqRange();
    float scale = SIZE * p.x ; 
    //float scale *= sin(SPEEDS*time / 50.0);
    
    float wave = sin( (scale * 10.0 ) / PI ) ;
    wave *= 1.0 + 10.0*snd(p.x);
    //wave *= 1.0 - 2.0 * distance(p.x, 0.5) ;
    //wave *= random() * 2.0 ;
    //wave *= sin( ((SIZE * p.y) * 10.0 ) / PI );
    wave *= sin((bass * SPEED * time) / 7.0) ;
    wave *= clamp(20.0 * bass,0.0, 1.0);

    wave *= 0.4;
    wave += 0.5;
    float brightness = distance( wave, p.y );
    //brightness *= pow(1.0 - 2.0*distance(p.y, 0.5), 2.0);

    brightness = 1.0 - abs(brightness);

    //float brightness = distance( waveY , p.x );
    brightness = pow( brightness, 1.0 / THICKNESS ) ;
    brightness *= clamp(bass*20.0,0.0,1.0);

    vec4 colorV = color( time / 100.0 );
    gl_FragColor = colorV * brightness;
    //gl_FragColor = vec4( vec3(distance(p.y,0.5),p.x,1.0 ) * brightness, 0.0 );
}
