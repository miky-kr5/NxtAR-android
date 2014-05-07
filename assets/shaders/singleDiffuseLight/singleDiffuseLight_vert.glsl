// Model-view matrix.
uniform mat4 u_projTrans;

// Vertex position in world coordinates.
attribute vec4 a_position;

// Vertex color.
attribute vec4 a_color;

// Vertex color to pass to the fragment shader.
varying vec4 v_color;

void main(){
	v_color = a_color;
	gl_Position = u_projTrans * a_position;
}
