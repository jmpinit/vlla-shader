#version 100
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;

    vec2 pos = vec2(0.5 + 0.5 * sin(time / 4.0), 0.5);
    float dist = 0.4 - distance(p, pos);

    gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0) * dist;
}
