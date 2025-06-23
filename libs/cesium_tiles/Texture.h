#pragma once

namespace CesiumTiles {

class Texture
{
  public:
	Texture(const unsigned char* buffer, int height, int width);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind();

	inline int GetHeight() const { return m_Height; }
	inline int GetWidth() const { return m_Width; }

  private:
	unsigned int m_Id;
	const unsigned char* m_LocalBuffer;
	int m_Width, m_Height;
};

} // namespace maptiles