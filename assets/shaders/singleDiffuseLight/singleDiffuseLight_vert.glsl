// Model-view matrix.
uniform mat4 u_projTrans;

// Vertex position in world coordinates.
attribute vec4 a_position;

void main(){
	gl_Position = u_projTrans * a_position;
}
