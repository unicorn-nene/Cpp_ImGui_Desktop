
#include <string>

class Texture
{
public:
	Texture();
	~Texture();

	bool Load(const std::string &fileName);
	void Unload();
	void CreateFromSurface(struct SDL_Surface *surface);

	void SetActive();

	int GetWidth() const { return mWidth; }
	int GetHeight() const { return mHeight; }
private:
	unsigned int mTextureID;
	int mWidth;
	int mHeight;
};
