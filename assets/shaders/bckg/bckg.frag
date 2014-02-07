#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_texture;
uniform vec2 u_scaling;

varying vec2 v_texCoords;

void main(){
	gl_FragColor = texture2D(u_texture, v_texCoords * u_scaling);
}