#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstddef>
#include <cstring>
#include <cstdint>

#include "Error.h"
#include "Checksum.h"
#include "PatchFile.h"

int WritePatch(char* patchFile, char* baseFullPath, int type, char* outMessage, int outMessageLength)
{
	std::string notice("https://github.com/toyobayashi/JMSUpdater/");
	std::string outputFile;
	uint32_t patchCRC = 0xF2F7FBF3;
	std::ofstream o;
	std::vector<uint8_t> v;
	std::ifstream currentEXE;
	std::ifstream fPatch;
	size_t lengthBase = 0;
	std::vector<uint8_t> base;
	uint32_t zlibCRC;
	Checksum crc;
	uint32_t pos;
	uint32_t lengthPatch;
	size_t blockSize = 0x500000;
	// std::ostringstream cmdline;
	size_t lengthCurrentEXE;
	uint32_t lengthZlibBlock;
	size_t lengthSize = 4;

	//Read in the base file stored in the current executable file
	if (strcmp(baseFullPath, "") == 0) {
		char fileName[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, fileName, MAX_PATH);
		currentEXE.open(fileName, std::ios::binary | std::ios::in);
		currentEXE.seekg(0, currentEXE.end);
		lengthCurrentEXE = (size_t) currentEXE.tellg();
		currentEXE.seekg(lengthCurrentEXE - lengthSize, currentEXE.beg);
		currentEXE.read(reinterpret_cast<char*>(&lengthBase), lengthSize);
		base.resize(lengthBase);
		currentEXE.seekg(lengthCurrentEXE - lengthSize - lengthBase, currentEXE.beg);
		currentEXE.read(reinterpret_cast<char*>(&base[0]), lengthBase);
		currentEXE.close();
	} else {
		std::ifstream basePatcher;
		basePatcher.open(baseFullPath, std::ios::binary | std::ios::in);
		basePatcher.seekg(0, basePatcher.end);
		lengthBase = (size_t) basePatcher.tellg();
		base.resize(lengthBase);
		basePatcher.seekg(0, basePatcher.beg);
		basePatcher.read(reinterpret_cast<char*>(&base[0]), lengthBase);
		basePatcher.close();
	}

	//Begin the process for creating the patch file.
	outputFile = patchFile;
	outputFile.replace(outputFile.find(".patch"), strlen(".patch"), ".exe");
	o.open(outputFile, std::ios::binary | std::ios::out);
	o.write(reinterpret_cast<char*>(&base[0]), base.size());
	base.clear();

	//Begin processing the patch file
	v.resize(blockSize);
	fPatch.open(patchFile, std::ios::binary | std::ios::in);
	fPatch.read(reinterpret_cast<char*>(&v[0]), 12);
	fPatch.read(reinterpret_cast<char*>(&zlibCRC), sizeof(zlibCRC));
	o.write(reinterpret_cast<char*>(&v[0]), 12);
	o.write(reinterpret_cast<char*>(&zlibCRC), sizeof(zlibCRC));

	//Calculate the CRC of the zlib block while writing to the new patch file
	pos = 0;
	fPatch.seekg(0, fPatch.end);
	lengthPatch = (uint32_t) fPatch.tellg();
	lengthZlibBlock = lengthPatch - 16;
	fPatch.seekg(16, fPatch.beg);
	while (pos < lengthZlibBlock) {
		if (pos + blockSize > lengthZlibBlock) {
      blockSize = lengthZlibBlock - pos;
		}
		fPatch.read(reinterpret_cast<char*>(&v[0]), blockSize);
		o.write(reinterpret_cast<char*>(&v[0]), blockSize);
		crc.Update(v, blockSize);
		pos += blockSize;
	}

	//Output the tail of the patch file
	uint32_t noticeLength = notice.length();
	o.write(reinterpret_cast<char*>(&notice[0]), notice.length());
	if (type == 2) {
		uint64_t lengthPatchULong = lengthPatch;
		uint64_t noticeLengthULong = noticeLength;
		uint64_t patchCRCULong = patchCRC;
		o.write(reinterpret_cast<char*>(&lengthPatchULong), sizeof(lengthPatchULong));
		o.write(reinterpret_cast<char*>(&noticeLengthULong), sizeof(noticeLengthULong));
		o.write(reinterpret_cast<char*>(&patchCRCULong), sizeof(patchCRCULong));
	} else {
		// type == 1
		o.write(reinterpret_cast<char*>(&lengthPatch), sizeof(lengthPatch));
		o.write(reinterpret_cast<char*>(&noticeLength), sizeof(noticeLength));
		o.write(reinterpret_cast<char*>(&patchCRC), sizeof(patchCRC));
	}
	o.close();

	if(zlibCRC != crc.GetResult())
	{
		std::string msg = ErrorHandler(NXError::BAD_CRC);
		DeleteFileA(outputFile.c_str());
		if (outMessage != NULL) {
			strncpy(outMessage, msg.c_str(), outMessageLength - 1);
			*(outMessage + outMessageLength - 1) = '\0';
		}
		return 0;
	}

	/* int result = MessageBox(NULL, "The pre-patcher is now created.\r\nDo you want to run the pre-patcher now?", "NXPatcher Lite", MB_YESNO | MB_ICONINFORMATION);
	if(result == IDYES)
	{
		ShellExecute(NULL, "open", outputFile.c_str(), NULL, NULL, SW_SHOW);
	} */

	if (outMessage != NULL) {
		strncpy(outMessage, outputFile.c_str(), outMessageLength - 1);
		*(outMessage + outMessageLength - 1) = '\0';
	}
	return 1;
}



/* void NoArgHandler(void)
{
	OPENFILENAME ofn;
	std::string szFile;
	std::string currentDirectory;
	std::string szFileTitle;
	int result;

	szFileTitle.resize(MAX_PATH);
	szFile.resize(MAX_PATH);
	currentDirectory.resize(MAX_PATH);

	GetCurrentDirectory(currentDirectory.length(), &currentDirectory[0]);

	result = 1;
	do {
		ZeroMemory(&ofn , sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = &szFile[0];
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = szFile.length();
		ofn.lpstrFilter = "Nexon Patch (*.patch)\0*.patch";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = &szFileTitle[0];
		ofn.nMaxFileTitle = szFileTitle.length();
		ofn.lpstrInitialDir = currentDirectory.c_str();
		ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
		
		result = GetOpenFileName(&ofn);

		if(result)
		{
			if(CheckHeader(ofn.lpstrFile) == 0)
			{
				ErrorHandler(NXError::BAD_HEADER, MB_OKCANCEL | MB_ICONEXCLAMATION);
				continue;
			}			
			WritePatch(ofn.lpstrFile);
			ExitProcess(EXIT_SUCCESS);
		}
	}while(result);
} */

int PreArgHandler(char* fileName, char* baseFullPath, int type, char* outMessage, int outMessageLength)
{
	if(CheckHeader(fileName) == 0) {
		std::string msg = ErrorHandler(NXError::BAD_HEADER);
		if (outMessage != NULL) {
			strncpy(outMessage, msg.c_str(), outMessageLength - 1);
			*(outMessage + outMessageLength - 1) = '\0';
		}
		return 0;
	} else {
		return WritePatch(fileName, baseFullPath, type, outMessage, outMessageLength);
	}
}


/* int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if(__argc == 2)
	{
		PreArgHandler(__argv[1]);
	}
	else
	{
		NoArgHandler();
	}
} */