#include "Quad.h"
#include "Shader.h"

Quad::Quad() {

	GLfloat vertices[] = { -0.5f,0.5f,0.0f,//TRIANGLE 1
						   0.5f,0.5f,0.0f,
						  -0.5f,-0.5f,0.0f,

						  -0.5f,-0.5f,0.0f,//TRIANGLE 2
						   0.5f,0.5f,0.0f,
						   0.5f,-0.5f,0.0f
	};


	GLfloat colors[] = { 1.0f,0.0f,0.0f,//TRIANGLE 1
						   0.0f,0.0f,1.0f,
						   0.0f,1.0f,1.0f,

						   0.0f,1.0f,1.0f,//TRIANGLE 2
						   0.0f,0.0f,1.0f,
						   0.0f,1.0f,0.0f
	};

	
	m_buffer.CreateBuffer(6); 
	
	m_buffer.FillVBO(Buffer::VERTEX_BUFFER, vertices, sizeof(vertices), Buffer::SINGLE);
	m_buffer.FillVBO(Buffer::COLOR_BUFFER, colors, sizeof(colors), Buffer::SINGLE);
	
	m_buffer.LinkBuffer("vertexIn", Buffer::VERTEX_BUFFER, Buffer::XYZ, Buffer::FLOAT);
	m_buffer.LinkBuffer("colorIn", Buffer::COLOR_BUFFER, Buffer::RGB, Buffer::FLOAT);

}

Quad::~Quad() {
	m_buffer.DestroyBuffer();
}

void Quad::Render() {
	m_buffer.Render(Buffer::TRIANGLES);
}