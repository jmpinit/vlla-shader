#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D fft;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

//varying lowp vec2 TexCoordOut;

void main() {
    gl_FragColor = texture2D(fft, vec2(gl_FragCoord.x / resolution.x, gl_FragCoord.y / resolution.y));
}
