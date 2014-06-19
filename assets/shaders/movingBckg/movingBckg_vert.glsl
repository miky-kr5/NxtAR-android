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
uniform mat4 u_projTrans;
uniform float u_displacement;

attribute vec4 a_position;
attribute vec2 a_texCoord0;

varying vec2 v_texCoords;

void main(){
	v_texCoords = a_texCoord0 + u_displacement;
	gl_Position = u_projTrans * a_position;
}