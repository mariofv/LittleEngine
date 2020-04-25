#ifndef _TEXTUREMANAGER_H_
#define _TEXTUREMANAGER_H_

#include "Filesystem/File.h"

#include <memory>
#include <vector>
#include <string>

namespace DDS
{
	struct DDS_PIXELFORMAT {
		uint32_t dwSize;
		uint32_t dwFlags;
		uint32_t dwFourCC;
		uint32_t dwRGBBitCount;
		uint32_t dwRBitMask;
		uint32_t dwGBitMask;
		uint32_t dwBBitMask;
		uint32_t dwABitMask;
	};

	typedef struct {
		uint32_t           dwSize;
		uint32_t           dwFlags;
		uint32_t           dwHeight;
		uint32_t           dwWidth;
		uint32_t           dwPitchOrLinearSize;
		uint32_t           dwDepth;
		uint32_t           dwMipMapCount;
		uint32_t           dwReserved1[11];
		DDS_PIXELFORMAT ddspf;
		uint32_t           dwCaps;
		uint32_t           dwCaps2;
		uint32_t           dwCaps3;
		uint32_t           dwCaps4;
		uint32_t           dwReserved2;
	} DDS_HEADER;
	const uint32_t magic_number = 4;

}

class Metafile;
class Texture;

class TextureManager
{
public:
	TextureManager() = default;
	~TextureManager()= default;

	static std::shared_ptr<Texture> Load(uint32_t uuid, const FileData& resource_data);

private:
	static std::vector<char> LoadCompressedDDS(const FileData& resource_data, DDS::DDS_HEADER & dds_header);
	static std::vector<char> LoadImageData(const FileData& resource_data,const std::string& file_path, int & width, int & height);

};

#endif //_TEXTUREMANAGER_H_
