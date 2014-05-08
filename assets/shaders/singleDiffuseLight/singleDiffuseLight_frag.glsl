#ifdef GL_ES
precision mediump float;
#endif

// Camera world space position.
uniform vec3 u_cameraPos;

// Light source position
uniform vec4 u_lightPos;

// Diffuse light color.
uniform vec4 u_lightDiffuse;

// Ambient light color.
uniform vec4 u_ambient;

// Shininess.
uniform float u_shiny;

// Fragment position.
varying vec4 v_position;

// Fragment normal.
varying vec3 v_normal;

// Fragment color received from the vertex shader.
varying vec4 v_color;

void main(){
	vec3 normal = normalize(v_normal);
	vec3 lightVector = normalize(u_lightPos.xyz - v_position.xyz);
	vec3 eyeVector = normalize(u_cameraPos.xyz - v_position.xyz);
	vec3 reflectedVector = normalize(-reflect(lightVector, normal));

	// Ambient Term.
	vec4 ambient = u_ambient;

	// Diffuse Term.
	vec4 diffuse = u_lightDiffuse * max(dot(normal, lightVector), 0.0);
	diffuse = clamp(diffuse, 0.0, 1.0);

	// Specular Term:
	vec4 specular = vec4(1.0) * pow(max(dot(reflectedVector, eyeVector), 0.0), 0.3 * u_shiny);
	specular = clamp(specular, 0.0, 1.0);

	// Aggregate light color.
	vec4 lightColor = vec4(ambient.rgb + diffuse.rgb + specular.rgb, 1.0);

	// Final color.
	gl_FragColor = clamp(lightColor * v_color, 0.0, 1.0);
}
