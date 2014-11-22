#version 100
#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.1415
#define SPEED 0.5
#define SIZE 5.0

#define LOW 0.0
#define HIGH 500.0
#define STEP 45.0
#define THICKNESS 0.01

uniform sampler2D fft;
uniform sampler2D last;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

float snd(float i) {
        return texture2D(fft, vec2(i, 0.0)).r ;
}

float bassRange() {
    float bass = 0.0;
    for (float i = LOW; i <= HIGH; i += STEP){
        bass += snd(i / 1024.0);
    }
    return bass / (( HIGH - LOW ) / STEP) ;
}

//float random() {
  //  float r = snd( mod(gl_FragCoord.y * gl_FragCoord.x, 1024.0) / 1024.0 );
//    r *= 1000.0;
   // r = mod(r, 1.0 ) - 0.5 ;
  //  return r;
//}

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    //float bass = snd(0.0) + snd(0.025) + snd(0.05) + snd(0.075) ;

    //for(float i = 0.0; i <= 10.0; i += 1.0) { 
    //    bass += snd(i / 1024.0); 
    //}

    float bass = bassRange();
    float scale = (SIZE * p.x) * sin(time / 50.0);
    //float scale = SIZE;
    float wave = sin( (scale * 10.0 ) / PI ) ;
    wave += 10.0 * snd(p.x);
    //wave *= 1.0 - 2.0 * distance(p.x, 0.5) ;
    //wave *= random() * 2.0 ;
    //wave *= sin( ((SIZE * p.y) * 10.0 ) / PI );
    wave *= sin((SPEED * time) / 7.0) ;
    //wave *= clamp(20.0 * bass,0.0, 1.0);

    wave *= 0.4;
    wave += 0.5;
    float brightness = distance( wave, p.y );
    //brightness *= pow(1.0 - 2.0*distance(p.y, 0.5), 2.0);

    brightness = 1.0 - abs(brightness);
    //float waveY = sin( ( p.y * 10.0 ) / PI );
    //waveY *= 0.5;
    //waveY += 0.5;
    //float brightness = distance( waveY , p.x );
    brightness = pow( brightness, 1.0 / THICKNESS ) ;
    //brightness *= clamp(bass*10.0,0.0,1.0) ;
    //brightness = random();
    //brightness = sqrt(brightness);

    gl_FragColor = vec4( vec3(1.0,1.0,1.0) * brightness, 0.0 ) ;
}
