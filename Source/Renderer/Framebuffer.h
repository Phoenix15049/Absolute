#pragma once

#include <gl.h>

enum class DepthAttachment
{
	None,
	Renderbuffer,
	ShadowTexture
};

struct FramebufferSpec
{
	int width = 0;
	int height = 0;
	GLenum colorFormat = 0;
	int samples = 0;
	DepthAttachment depth = DepthAttachment::None;

	bool operator==(const FramebufferSpec& other) const
	{
		return width == other.width && height == other.height && colorFormat == other.colorFormat &&
			samples == other.samples && depth == other.depth;
	}
};

class Framebuffer
{

public:

	Framebuffer();
	~Framebuffer();

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;

	bool Create(const FramebufferSpec& spec);
	void Destroy();

	void Bind() const;
	void BlitColorTo(const Framebuffer& target) const;

	bool IsValid() const;
	const FramebufferSpec& GetSpec() const;
	GLuint GetID() const;
	GLuint GetColorTexture() const;
	GLuint GetDepthTexture() const;

private:

	FramebufferSpec m_spec;

	GLuint m_FBO;
	GLuint m_colorTexture;
	GLuint m_colorRenderbuffer;
	GLuint m_depthTexture;
	GLuint m_depthRenderbuffer;

};
