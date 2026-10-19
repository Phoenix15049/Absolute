#pragma once

#include <string>
#include <vector>
#include <gl.h>

struct ImageData
{
	int width = 0;
	int height = 0;
	std::vector<unsigned char> pixels;

	bool IsValid() const
	{
		return width > 0 && height > 0 && !pixels.empty();
	}

	static bool Load(const std::string& filename, ImageData& image);
};

class Texture
{

public:

	Texture();
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture&& other) noexcept;

	bool Create(const ImageData& image, bool isSRGB);
	bool Load(const std::string& filename, bool isSRGB);
	void CreateSolid(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void Destroy();

	void Bind(GLuint unit) const;
	void ApplyAnisotropy() const;

	bool IsLoaded() const;
	GLuint GetID() const;
	int GetWidth() const;
	int GetHeight() const;

	static void SetAnisotropy(float level);
	static float GetMaxAnisotropy();

private:

	GLuint m_ID;
	int m_width;
	int m_height;

	static float s_anisotropy;

};
