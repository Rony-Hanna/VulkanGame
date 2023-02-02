#pragma once

#include <vector>
#include <string>

class Utilities
{
public:
	static std::vector<char> ReadBinaryFile(const std::string& _fileName);
	static unsigned char* LoadTextureFile(const std::string& _fileName, int* _width, int* _height, int* _desiredChannels);
	static void FreeImage(unsigned char* _imageData);
};