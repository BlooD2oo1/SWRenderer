#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma pack(push, 1)
struct SPCXHeader
{
	uint8_t  m_byManufacturer; // Always 0x0A (ZSoft PCX)
	uint8_t  m_byVersion;      // 5 = 256-color palette support
	uint8_t  m_byEncoding;     // 1 = RLE compression
	uint8_t  m_byBitsPerPixel; // 8 bits per pixel
	uint16_t m_wXMin;
	uint16_t m_wYMin;
	uint16_t m_wXMax;
	uint16_t m_wYMax;
	uint16_t m_wHDPI;
	uint16_t m_wVDPI;
	uint8_t  m_aColorMap[48];  // 16-color EGA palette (ignored for 256-color images)
	uint8_t  m_byReserved;
	uint8_t  m_byNPlanes;      // 1 (for single-plane paletted images)
	uint16_t m_wBytesPerLine;  // Number of bytes per scanline (pitch)
	uint16_t m_wPaletteInfo;
	uint16_t m_wHScreenSize;
	uint16_t m_wVScreenSize;
	uint8_t  m_aFiller[54];
};
#pragma pack(pop)

/**
* Loads a single-plane, 256-color PCX file into the provided SPCXData structure.
* 
* @param szFilename Path to the PCX file
* @param outPcx Output target structure reference
* @return true if loading succeeded, false otherwise
*/
inline bool PCX_LoadFromFile(const char* szFilename, STextureIndexed& outPcx)
{
	outPcx.Clear();

	FILE* pFile = nullptr;
#if defined(_MSC_VER) && _MSC_VER >= 1400
	fopen_s(&pFile, szFilename, "rb");
#else
	pFile = fopen(szFilename, "rb");
#endif

	if (!pFile) return false;

	// Check file size
	fseek(pFile, 0, SEEK_END);
	long lFileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	// Verify minimum valid file size: Header + min RLE payload + 256-color palette (769 bytes)
	if (lFileSize < static_cast<long>(sizeof(SPCXHeader) + 769))
	{
		fclose(pFile);
		return false;
	}

	// Read PCX header
	SPCXHeader header;
	if (fread(&header, sizeof(SPCXHeader), 1, pFile) != 1)
	{
		fclose(pFile);
		return false;
	}

	// Format validation (must be 8-bit, 1-plane, RLE encoded)
	if (header.m_byManufacturer != 0x0A ||
		header.m_byBitsPerPixel != 8 ||
		header.m_byNPlanes != 1 ||
		header.m_byEncoding != 1)
	{
		fclose(pFile);
		return false;
	}

	int32_t iWidth  = header.m_wXMax - header.m_wXMin + 1;
	int32_t iHeight = header.m_wYMax - header.m_wYMin + 1;

	if (iWidth <= 0 || iHeight <= 0)
	{
		fclose(pFile);
		return false;
	}

	// Read 256-color palette from the tail of the file (last 769 bytes)
	fseek(pFile, -769, SEEK_END);
	uint8_t byPaletteMarker = 0;
	if (fread(&byPaletteMarker, 1, 1, pFile) != 1 || byPaletteMarker != 0x0C)
	{
		fclose(pFile);
		return false; // Missing 0x0C palette marker byte
	}

	uint8_t* pPalette = new uint8_t[768]; // 256 * RGB
	if (fread(pPalette, 1, 768, pFile) != 768)
	{
		delete[] pPalette;
		fclose(pFile);
		return false;
	}

	// Seek back to the image data section (right after the 128-byte header)
	fseek(pFile, sizeof(SPCXHeader), SEEK_SET);

	uint8_t* pPixelData = new uint8_t[iWidth * iHeight];
	uint32_t uBytesPerLine = header.m_wBytesPerLine;

	// RLE decompression line by line
	for (int32_t y = 0; y < iHeight; ++y)
	{
		uint32_t uBytesDecoded = 0;
		uint8_t* pLineDst = pPixelData + (y * iWidth);

		while (uBytesDecoded < uBytesPerLine)
		{
			int c = fgetc(pFile);
			if (c == EOF)
			{
				delete[] pPixelData;
				delete[] pPalette;
				fclose(pFile);
				return false;
			}

			uint8_t uCount = 1;
			uint8_t uValue = static_cast<uint8_t>(c);

			// If the top 2 bits are set (>= 0xC0), this byte represents a run-length count
			if ((uValue & 0xC0) == 0xC0)
			{
				uCount = uValue & 0x3F; // Lower 6 bits store the repetition count
				c = fgetc(pFile);
				if (c == EOF)
				{
					delete[] pPixelData;
					delete[] pPalette;
					fclose(pFile);
					return false;
				}
				uValue = static_cast<uint8_t>(c);
			}

			// Write bytes to the target buffer, dropping line padding beyond image width
			for (uint8_t k = 0; k < uCount; ++k)
			{
				if (uBytesDecoded < static_cast<uint32_t>(iWidth))
				{
					pLineDst[uBytesDecoded] = uValue;
				}
				uBytesDecoded++;
			}
		}
	}

	fclose(pFile);

	// Assign buffers and dimensions to target structure on success
	outPcx.m_iWidth   = iWidth;
	outPcx.m_iHeight  = iHeight;
	outPcx.m_pData    = pPixelData;
	outPcx.m_pPalette = pPalette;

	return true;
}
