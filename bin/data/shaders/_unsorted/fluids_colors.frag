/*
{
  "CATEGORIES": [
    "Automatically Converted"
  ],
  "INPUTS": [
    
  ]
}
*/

#ifdef GL_ES
precision mediump float;
#endif
 
 
// these seem popular nowdays let's try it with a twist --jz
 
// how many curly fries do we need?
#define MAX_ITER 12
 
void main( void ) {
 
	vec2 p = vv_FragNormCoord;
	vec2 i = p;
 
	float c = 0.0;
 
	float d = length(i);
	float r = atan(i.x, i.y);
	for (int n = 0; n < MAX_ITER; n++) {
		i = i + vec2( 
				1.0/float((n+1) * MAX_ITER/(n+1)) * d * sin(r * i.y * (pow(float(n), 2.0 - abs(sin(TIME * 0.022))) * float(MAX_ITER)/float(n+1)) + TIME * 1.49),
				1.0/float((n+1) * MAX_ITER/(n+1)) * d * cos(r * i.x * (pow(float(n), 2.0 - abs(cos(TIME * 0.021))) * float(MAX_ITER)/float(n+1)) + TIME * 1.51)
			);
		
		d = length(i);
	        r = atan(i.x, i.y);
	}
	
	c = (1.0-d);
	c = pow(c, 2.2);
	gl_FragColor = vec4( vec3( c*abs(sin(i.x + 2.0)), c*abs(sin(i.y + 4.0)), c*abs(sin(i.x+i.y + 4.0))) * vec3(1.01,1.15,1.5), 1.0 );
}