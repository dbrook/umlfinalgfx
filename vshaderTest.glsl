attribute  vec4 vPosition;
attribute  vec4 vColor;
attribute  vec3 vNormal;
varying vec4 color;

uniform mat4 model_view;
uniform mat4 projection;

varying vec4 lightDiff;


void main()
{
    vec3 vertNormal = normalize(gl_NormalMatrix * vNormal);
    
    vec3 vLight0Pos = gl_LightSource[0].position.xyz;
	vec3 vLight1Pos = gl_LightSource[1].position.xyz;
	
	vec4 light0Diff = gl_LightSource[0].diffuse * max(dot(vertNormal, vLight0Pos), 0.0);
	vec4 light1Diff = gl_LightSource[1].diffuse * max(dot(vertNormal, vLight1Pos), 0.0);
	lightDiff = light0Diff + light1Diff;

    gl_Position = projection * model_view * vPosition;
    color = vColor;
    
}
