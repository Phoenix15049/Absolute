#include <algorithm>
#include <cstring>
#include <SDL.h>
#include <SDL_image.h>
#include "Core/Log.h"
#include "Renderer/Texture.h"

float Texture::s_anisotropy = 1.0f;

bool ImageData::Load(const std::string& filename, ImageData& image)
{
	SDL_Surface* loaded = IMG_Load(filename.c_str());

	if (!loaded)
	{
		Log::Error("Could not load image " + filename + ": " + IMG_GetError());
		return false;
	}

	SDL_Surface* converted = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(loaded);

	if (!converted)
	{
		Log::Error("Could not convert image " + filename + ": " + SDL_GetError());
		return false;
	}

	image.width = converted->w;
	image.height = converted->h;

	const size_t rowSize = static_cast<size_t>(image.width) * 4;
	image.pixels.resize(rowSize * image.height);

	const unsigned char* source = static_cast<const unsigned char*>(converted->pixels);

	for (int row = 0; row < image.height; row++)
	{
		std::memcpy(&image.pixels[(image.height - 1 - row) * rowSize], source + row * converted->pitch, rowSize);
	}

	SDL_FreeSurface(converted);
	return true;
}

Texture::Texture()
{
	m_ID = 0;
	m_width = 0;
	m_height = 0;
}

Texture::~Texture()
{
	Destroy();
}

Texture::Texture(Texture&& other) noexcept
{
	m_ID = other.m_ID;
	m_width = other.m_width;
	m_height = other.m_height;
	other.m_ID = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
	if (this != &other)
	{
		Destroy();

		m_ID = other.m_ID;
		m_width = other.m_width;
		m_height = other.m_height;
		other.m_ID = 0;
	}

	return *this;
}

bool Texture::Create(const ImageData& image, bool isSRGB)
{
	if (!image.IsValid())
	{
		return false;
	}

	Destroy();

	m_width = image.width;
	m_height = image.height;

	glGenTextures(1, &m_ID);
	glBindTexture(GL_TEXTURE_2D, m_ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8, m_width, m_height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	ApplyAnisotropy();

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool Texture::Load(const std::string& filename, bool isSRGB)
{
	ImageData image;
	return ImageData::Load(filename, image) && Create(image, isSRGB);
}

void Texture::CreateSolid(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	ImageData image;
	image.width = 1;
	image.height = 1;
	image.pixels = { r, g, b, a };

	Create(image, false);
}

void Texture::Destroy()
{
	if (m_ID != 0)
	{
		glDeleteTextures(1, &m_ID);
		m_ID = 0;
	}
}

void Texture::Bind(GLuint unit) const
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::ApplyAnisotropy() const
{
	if (m_ID == 0 || GetMaxAnisotropy() <= 1.0f)
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, m_ID);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, std::clamp(s_anisotropy, 1.0f, GetMaxAnisotropy()));
}

bool Texture::IsLoaded() const
{
	return m_ID != 0;
}

GLuint Texture::GetID() const
{
	return m_ID;
}

int Texture::GetWidth() const
{
	return m_width;
}

int Texture::GetHeight() const
{
	return m_height;
}

void Texture::SetAnisotropy(float level)
{
	s_anisotropy = level;
}

float Texture::GetMaxAnisotropy()
{
	static float maxAnisotropy = -1.0f;

	if (maxAnisotropy < 0.0f)
	{
		maxAnisotropy = 1.0f;

		if (SDL_GL_ExtensionSupported("GL_EXT_texture_filter_anisotropic") ||
			SDL_GL_ExtensionSupported("GL_ARB_texture_filter_anisotropic"))
		{
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
		}
	}

	return maxAnisotropy;
}
