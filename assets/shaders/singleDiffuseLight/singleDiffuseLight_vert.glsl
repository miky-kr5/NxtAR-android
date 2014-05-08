// Model-view matrix.
uniform mat4 u_projTrans;

// Normal matrix.
uniform mat4 u_normalMat;

// Vertex position in world coordinates.
attribute vec4 a_position;

// Vertex normal.
attribute vec4 a_normal;

// Vertex color.
attribute vec4 a_color;

// Vertex position to pass to the fragment shader.
varying vec4 v_position;

// Vertex normal to pass to the fragment shader.
varying vec3 v_normal;

// Vertex color to pass to the fragment shader.
varying vec4 v_color;

void main(){
	v_position = u_projTrans * a_position;
	v_color = a_color;
	v_normal = vec3(u_normalMat * a_normal);

	gl_Position = v_position;
}
