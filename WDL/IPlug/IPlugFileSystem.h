#pragma once

#include "IGraphics.h"

#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>

// for windows mkdir
#ifdef _WIN32
#include <direct.h>
#endif

#include "UTF_Helper.h"

using namespace std;

class IPlugFileSystem : public Windows_UTF_Converter
{
public:

	bool FileCopy(const char* src_path, const char* dst_path)
	{
		utf8::ifstream src(src_path, ios::binary);
		utf8::ofstream dest(dst_path, ios::binary);
		dest << src.rdbuf();
		return src && dest;
	}

	bool FileExist(const char* path) 
	{
		utf8::ifstream file(path);
		return file.good();
	}

	bool FileDelete(const char* path)
	{
		int nError = 0;
#if defined(_WIN32)
		nError = _wremove(utf8_to_utf16(path).c_str());
#else 
		nError = remove(sPath.c_str());
#endif

		if (nError == 0) return true;

		return false;
	}

	bool DirExist(const char* path)
	{
#if defined(_WIN32)
		struct _stat st;
		_wstat(utf8_to_utf16(path).c_str(), &st);
		return st.st_mode & S_IFDIR;
#else 
		struct stat st;
		stat(path, &st);
		return st.st_mode & S_IFDIR;
#endif
	}

	bool DirCreate(const char* path)
	{
		int nError = 0;
#if defined(_WIN32)
		nError = _wmkdir(utf8_to_utf16(path).c_str());
#else 
		mode_t nMode = 0733; // UNIX style permissions
		nError = mkdir(sPath.c_str(), nMode);
#endif

		if (nError == 0) return true;

		return false;
	}

	bool DirDelete(const char* path)
	{
		int nError = 0;
#if defined(_WIN32)
		nError = _wrmdir(utf8_to_utf16(path).c_str());
#else 
		nError = rmdir(sPath.c_str());
#endif

		if (nError == 0) return true;

		return false;
	}

private:

};