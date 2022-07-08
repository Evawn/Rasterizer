/*
 * Written for Cornell CS 5625 (Interactive Computer Graphics).
 * Copyright (c) 2015, Department of Computer Science, Cornell University.
 * 
 * This code repository has been authored collectively by:
 * Ivaylo Boyadzhiev (iib2), John DeCorato (jd537), Asher Dunn (ad488), 
 * Pramook Khungurn (pk395), Sean Ryan (ser99), and Eston Schweickart (ers273)
 */

#version 330

layout (location = 0) in vec3 vert_position;
layout (location = 1) in vec2 vert_texCoord;

out vec2 geom_texCoord;

void main()
{
	gl_Position = vec4(vert_position, 1.0);
	geom_texCoord = vert_texCoord;
}
