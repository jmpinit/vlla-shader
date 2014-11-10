#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

const float PI=3.14159265359;
const int steps=30;

//derived from http://upload.wikimedia.org/wikipedia/commons/5/5d/HSV-RGB-comparison.svg
vec3 hsbToRGB(float h,float s,float b){
    return b*(1.0-s)+(b-b*(1.0-s))*clamp(abs(abs(6.0*(mod(h,1.0)-vec3(0,1,2)/3.0))-3.0)-1.0,0.0,1.0);
}

vec3 rotateY(in vec3 v, in float a) {
    return vec3(cos(a)*v.x + sin(a)*v.z, v.y,-sin(a)*v.x + cos(a)*v.z);
}

vec3 rotateX(in vec3 v, in float a) {
    return vec3(v.x,cos(a)*v.y + sin(a)*v.z,-sin(a)*v.y + cos(a)*v.z);
}


float dCylinder(float radius,float height,vec3 pos){
    vec2 d = abs(vec2(length(pos.xz),pos.y)) - vec2(radius,height);
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));}


    void main( void ) {
        vec2 screenPos=gl_FragCoord.xy/resolution-0.5;
        vec3 rayDir=normalize(vec3(screenPos.x*1.5,screenPos.y-0.2,0.5));
        rayDir=rotateX(rayDir,2.0*(mouse.y)-2.0);
        rayDir=rotateY(rayDir,8.0*(mouse.x-0.5));

        vec3 cameraPos =vec3(0.0,1.0,-4.0);
        cameraPos=rotateX(cameraPos,2.0*(mouse.y)-2.0);
        cameraPos=rotateY(cameraPos,8.0*(mouse.x-0.5));

        float iter=0.0;

        for(int i=0;i<steps;i++){
            float d=dCylinder(2.0,0.7,cameraPos);
            if(d<=0.01){
                break;
            }
            cameraPos+=d*rayDir;
            iter++;
        }

        gl_FragColor = vec4( (1.0-iter/float(steps))*vec3(hsbToRGB(atan(cameraPos.x,cameraPos.z)/(2.0*PI),0.5*length(cameraPos.xz),(cameraPos.y+0.7)/1.4)),1 );
    }
