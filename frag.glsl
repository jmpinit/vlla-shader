precision mediump float;

uniform float t;

float myFun(float coord){
    if(int(coord)==50){return 1.0;}
    return 0.0;
}

void main(void){
    vec2 uv = gl_FragCoord.xy;
    gl_FragColor = vec4(sin(gl_FragCoord.y/5.)+1.+sin(t+180.),.02,sin(gl_FragCoord.x/5.)+1.+sin(t),1.0);
}
