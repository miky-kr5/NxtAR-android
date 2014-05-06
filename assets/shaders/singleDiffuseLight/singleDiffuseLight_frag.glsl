#ifdef GL_ES
precision mediump float;
#endif

// Light color.
uniform vec3 u_lightColor;
// Diffuse surface color.
uniform vec3 u_diffColor;
// Specular color.
uniform vec3 u_specColor;
// Ambient light color.
uniform vec3 u_ambColor;

void main(){
	// TODO: Implement per pixel diffuse lighting.
	gl_FragColor = vec4(u_diffColor, 1.0);
}
