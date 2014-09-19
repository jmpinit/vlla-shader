precision mediump float;

uniform float t;

void main(void){
    vec2 uv = gl_FragCoord.xy;
    gl_FragColor = vec4(sin(gl_FragCoord.y/2.)+1.+sin(t+180.),.02,sin(gl_FragCoord.x/2.)+1.+sin(t),1.0);
}
