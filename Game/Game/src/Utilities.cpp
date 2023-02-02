#include "Utilities.h"
#include "Vendors/stb_image.h"
#include <fstream>

std::vector<char> Utilities::ReadBinaryFile(const std::string& _fileName)
{
	std::ifstream inputFile(_fileName, std::ios::binary | std::ios::ate);

	if (!inputFile.is_open())
	{
		throw std::runtime_error("ERROR: Failed to open binary file\n");
	}

	size_t fileSize = static_cast<size_t>(inputFile.tellg());
	std::vector<char> fileBuffer(fileSize);

	inputFile.seekg(0);
	inputFile.read(fileBuffer.data(), fileSize);
	inputFile.close();

	return fileBuffer;
}

unsigned char* Utilities::LoadTextureFile(const std::string& _fileName, int* _width, int* _height, int* _desiredChannels)
{
	int nChannels = 0;
	*_desiredChannels = STBI_rgb_alpha;

	return stbi_load(("res/textures/" + _fileName).c_str(), _width, _height, &nChannels, *_desiredChannels);
}

void Utilities::FreeImage(unsigned char* _imageData)
{
	stbi_image_free(_imageData);
}
