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

// The inverse transpose of the geometric transformation matrix.
uniform mat4 u_normalMatrix;

// Light source position
uniform vec3 u_lightPos;

// Diffuse light color.
uniform vec4 u_lightDiffuse;

// Camera world space position.
uniform vec3 u_cameraPos;

// Vertex color.
uniform vec4 u_materialDiffuse;

// Vertex position in world coordinates.
attribute vec4 a_position;

// Vertex normal.
attribute vec4 a_normal;

// Fragment normal.
varying vec3 v_normal;

// Diffuse shaded color.
varying vec4 v_diffuse;

// The vector from the vertex to the camera.
varying vec3 v_eyeVector;

// The light vector reflected around the vertex normal.
varying vec3 v_reflectedVector;

// The clamped dot product between the normal and the light vector.
varying float v_nDotL;

void main(){
	vec4 transformedPosition = u_geomTrans * a_position;
	vec3 lightVector         = normalize(u_lightPos.xyz);
	vec3 invLightVector      = normalize(-u_lightPos.xyz);

	// Set the varyings.
	v_normal          = normalize(vec4(u_normalMatrix * a_normal).xyz);
	v_eyeVector       = normalize(u_cameraPos.xyz - transformedPosition.xyz);
	v_reflectedVector = normalize(reflect(-lightVector, v_normal));

	// Diffuse Term.
	float invNDotL = max(dot(v_normal.xyz, invLightVector), 0.0);
	v_nDotL   = max(dot(v_normal.xyz, lightVector), 0.0);
	v_diffuse = (u_lightDiffuse * u_materialDiffuse * v_nDotL) + (vec4(0.1, 0.1, 0.2, 1.0) * u_materialDiffuse * invNDotL);

	gl_Position = u_projTrans * transformedPosition;
}
