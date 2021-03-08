#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstddef>
#include <cstring>

#include "Error.h"
#include "Checksum.h"
#include "PatchFile.h"

int WritePatch(char* patchFile, char* outMessage, int outMessageLength)
{
	std::string notice("NXPatcher Lite v2.0 - Written by Fiel (toyobayashi Fork) - http://www.southperry.net/");
	std::string outputFile;
	int patchCRC = 0xF2F7FBF3;
	std::ofstream o;
	std::vector<unsigned char> v;
	std::ifstream currentEXE;
	std::ifstream fPatch;
	size_t lengthBase = 0;
	std::vector<unsigned char> base;
	unsigned int zlibCRC;
	Checksum crc;
	int pos;
	int lengthPatch;
	int blockSize = 5000000;
	std::ostringstream cmdline;
	size_t lengthCurrentEXE;
	char fileName[MAX_PATH] = {0};
	int lengthZlibBlock;
	
	size_t lengthSize = 4;

	//Read in the base file stored in the current executable file
	GetModuleFileNameA(NULL, fileName, MAX_PATH);
	currentEXE.open(fileName, std::ios::binary | std::ios::in);
	currentEXE.seekg(0, currentEXE.end);
	lengthCurrentEXE = (int) currentEXE.tellg();
	currentEXE.seekg(lengthCurrentEXE - lengthSize, currentEXE.beg);
	currentEXE.read(reinterpret_cast<char*>(&lengthBase), lengthSize);
	base.resize(lengthBase);
	currentEXE.seekg(lengthCurrentEXE - lengthSize - lengthBase, currentEXE.beg);
	currentEXE.read(reinterpret_cast<char*>(&base[0]), lengthBase);
	currentEXE.close();

	//Begin the process for creating the patch file.
	outputFile = patchFile;
	outputFile.replace(outputFile.find(".patch"), strlen(".patch"), ".exe");
	o.open(outputFile, std::ios::binary | std::ios::out);
	o.write(reinterpret_cast<char*>(&base[0]), base.size());

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
	lengthPatch = (int) fPatch.tellg();
	lengthZlibBlock = lengthPatch - 16;
	fPatch.seekg(16, fPatch.beg);
	while(pos < lengthZlibBlock)
	{
		if(pos + blockSize > lengthZlibBlock)
		{
			blockSize = lengthZlibBlock - pos;
		}
		fPatch.read(reinterpret_cast<char*>(&v[0]), blockSize);
		o.write(reinterpret_cast<char*>(&v[0]), blockSize);
		crc.Update(v, blockSize);
		pos += blockSize;
	}

	//Output the tail of the patch file
	int noticeLength = notice.length();
	o.write(reinterpret_cast<char*>(&notice[0]), notice.length());
	o.write(reinterpret_cast<char*>(&lengthPatch), sizeof(lengthPatch));
	o.write(reinterpret_cast<char*>(&noticeLength), sizeof(noticeLength));
	o.write(reinterpret_cast<char*>(&patchCRC), sizeof(patchCRC));
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
		memset(outMessage, 0, outMessageLength);
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

int PreArgHandler(char* fileName, char* outMessage, int outMessageLength)
{
	if(CheckHeader(fileName) == 0) {
		std::string msg = ErrorHandler(NXError::BAD_HEADER);
		if (outMessage != NULL) {
			strncpy(outMessage, msg.c_str(), outMessageLength - 1);
			*(outMessage + outMessageLength - 1) = '\0';
		}
		return 0;
	} else {
		return WritePatch(fileName, outMessage, outMessageLength);
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