#include "Core/Log.h"
#include "Renderer/Framebuffer.h"

Framebuffer::Framebuffer()
{
	m_FBO = 0;
	m_colorTexture = 0;
	m_colorRenderbuffer = 0;
	m_depthTexture = 0;
	m_depthRenderbuffer = 0;
}

Framebuffer::~Framebuffer()
{
	Destroy();
}

bool Framebuffer::Create(const FramebufferSpec& spec)
{
	Destroy();

	m_spec = spec;

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	if (spec.colorFormat != 0)
	{
		if (spec.samples > 0)
		{
			glGenRenderbuffers(1, &m_colorRenderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, spec.samples, spec.colorFormat, spec.width, spec.height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorRenderbuffer);
		}

		else
		{
			glGenTextures(1, &m_colorTexture);
			glBindTexture(GL_TEXTURE_2D, m_colorTexture);
			const GLenum format = (spec.colorFormat == GL_R11F_G11F_B10F || spec.colorFormat == GL_RGB16F) ? GL_RGB : GL_RGBA;
			glTexImage2D(GL_TEXTURE_2D, 0, spec.colorFormat, spec.width, spec.height, 0, format, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
		}
	}

	else
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	if (spec.depth == DepthAttachment::Renderbuffer)
	{
		glGenRenderbuffers(1, &m_depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);

		if (spec.samples > 0)
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, spec.samples, GL_DEPTH24_STENCIL8, spec.width, spec.height);
		}

		else
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, spec.width, spec.height);
		}

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
	}

	else if (spec.depth == DepthAttachment::ShadowTexture)
	{
		const float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glGenTextures(1, &m_depthTexture);
		glBindTexture(GL_TEXTURE_2D, m_depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, spec.width, spec.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
	}

	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		Log::Error("Framebuffer is incomplete (status " + std::to_string(status) + ").");
		Destroy();
		return false;
	}

	return true;
}

void Framebuffer::Destroy()
{
	if (m_colorTexture != 0)
	{
		glDeleteTextures(1, &m_colorTexture);
		m_colorTexture = 0;
	}

	if (m_depthTexture != 0)
	{
		glDeleteTextures(1, &m_depthTexture);
		m_depthTexture = 0;
	}

	if (m_colorRenderbuffer != 0)
	{
		glDeleteRenderbuffers(1, &m_colorRenderbuffer);
		m_colorRenderbuffer = 0;
	}

	if (m_depthRenderbuffer != 0)
	{
		glDeleteRenderbuffers(1, &m_depthRenderbuffer);
		m_depthRenderbuffer = 0;
	}

	if (m_FBO != 0)
	{
		glDeleteFramebuffers(1, &m_FBO);
		m_FBO = 0;
	}

	m_spec = FramebufferSpec();
}

void Framebuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glViewport(0, 0, m_spec.width, m_spec.height);
}

void Framebuffer::BlitColorTo(const Framebuffer& target) const
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.m_FBO);
	glBlitFramebuffer(0, 0, m_spec.width, m_spec.height, 0, 0, target.m_spec.width, target.m_spec.height,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::IsValid() const
{
	return m_FBO != 0;
}

const FramebufferSpec& Framebuffer::GetSpec() const
{
	return m_spec;
}

GLuint Framebuffer::GetID() const
{
	return m_FBO;
}

GLuint Framebuffer::GetColorTexture() const
{
	return m_colorTexture;
}

GLuint Framebuffer::GetDepthTexture() const
{
	return m_depthTexture;
}
