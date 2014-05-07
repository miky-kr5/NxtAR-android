#ifdef GL_ES
precision mediump float;
#endif

// Fragment color received from the vertex shader.
varying vec4 v_color;

void main(){
	gl_FragColor = v_color;
}
