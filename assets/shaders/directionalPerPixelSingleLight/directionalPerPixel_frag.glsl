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

#ifdef GL_ES
precision mediump float;
#endif

// Ambient light color.
uniform vec4 u_ambient;

// Specular light color.
uniform vec4 u_specular;

// Shininess.
uniform float u_shiny;

// Fragment normal.
varying vec3 v_normal;

// Fragment shaded diffuse color.
varying vec4 v_diffuse;

// Vector from the fragment to the light source.
varying vec3 v_lightVector;

// Vector from the fragment to the camera.
varying vec3 v_eyeVector;

// The light vector reflected around the fragment normal.
varying vec3 v_reflectedVector;

void main(){
	// Normalize the input varyings.
	vec3 normal = normalize(v_normal);
	vec3 lightVector = normalize(v_lightVector);
	vec3 eyeVector = normalize(v_eyeVector);
	vec3 reflectedVector = normalize(v_reflectedVector);

	// Specular Term:
	vec4 specular = u_specular * pow(max(dot(reflectedVector, eyeVector), 0.0), u_shiny);

	// Aggregate light color.
	vec4 finalColor = clamp(vec4(u_ambient.rgb + v_diffuse.rgb + specular.rgb, 1.0), 0.0, 1.0);

	// Final color.
	gl_FragColor = finalColor;
}
