#version 100
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D fft;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main() {
    float x = 0.5*sin(time / 5.0) + 0.5;
    float y = 0.5;
    float xpos = gl_FragCoord.x / resolution.x;
    float ypos = gl_FragCoord.y / resolution.y;
    float d = sqrt(2.0) - 2.0 * sqrt(pow(y-ypos, 2.0) + pow(x-xpos, 2.0));
    
    float freq = texture2D(fft, vec2(d / sqrt(2.0), 0.0)).r;
    float c = d * freq;

    //float snd = texture2D(fft, vec2(xpos, 0.0)).r;
    //gl_FragColor = vec4(snd, snd, snd, 0.0);//vec4(c, c, c, 0.0);
    gl_FragColor = vec4(c, c, c, 0.0);
}
