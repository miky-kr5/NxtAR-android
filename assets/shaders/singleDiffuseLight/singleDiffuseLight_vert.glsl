/*
 * Copyright (C) 2014 Miguel Angel Astor Romero
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Model-view matrix.
uniform mat4 u_projTrans;

// The world space geometric transformation to apply to this vertex.
uniform mat4 u_geomTrans;

// Light source position
uniform vec4 u_lightPos;

// Diffuse light color.
uniform vec4 u_lightDiffuse;

// Camera world space position.
uniform vec3 u_cameraPos;

// Vertex position in world coordinates.
attribute vec4 a_position;

// Vertex normal.
attribute vec4 a_normal;

// Vertex color.
attribute vec4 a_color;

// Vertex color to pass to the fragment shader.
varying vec4 v_color;

// Diffuse shaded color to pass to the fragment shader.
varying vec4 v_diffuse;

// The vector from the vertex to the light source.
varying vec3 v_lightVector;

// The vector from the vertex to the camera.
varying vec3 v_eyeVector;

// The light vector reflected around the vertex normal.
varying vec3 v_reflectedVector;

void main(){
	// Apply the geometric transformation to the original position of the vertex.
	vec4 transformedPosition = u_geomTrans * a_position;

	// Set the varyings.
	v_lightVector = normalize(u_lightPos.xyz - transformedPosition.xyz);
	v_eyeVector = normalize(u_cameraPos.xyz - transformedPosition.xyz);
	v_reflectedVector = normalize(-reflect(v_lightVector, a_normal.xyz));
	v_color = a_color;

	// Diffuse Term.
	v_diffuse = u_lightDiffuse * max(dot(a_normal.xyz, v_lightVector), 0.0);

	gl_Position = u_projTrans * transformedPosition;
}
