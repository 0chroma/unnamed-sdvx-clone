#include "stdafx.h"
#include "Path.hpp"
#include "Log.hpp"
#include "Shellapi.h"

/*
	Windows version
*/
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

char Path::sep = '\\';

bool Path::CreateDir(const String& path)
{
	return CreateDirectoryA(*path, nullptr) == TRUE;
}
bool Path::Delete(const String& path)
{
	return DeleteFileA(*path) == TRUE;
}
bool Path::DeleteDir(const String& path)
{
	if(!ClearDir(path))
		return false;
	return RemoveDirectoryA(*path) == TRUE;
}
bool Path::Rename(const String& srcFile, const String& dstFile, bool overwrite)
{
	if(PathFileExistsA(*dstFile) == TRUE)
	{
		if(!overwrite)
			return false;
		if(DeleteFileA(*dstFile) == FALSE)
		{
			Logf("Failed to rename file, overwrite was true but the destination could not be removed", Logger::Warning);
			return false;
		}
	}
	return MoveFileA(*srcFile, *dstFile) == TRUE;
}
bool Path::Copy(const String& srcFile, const String& dstFile, bool overwrite)
{
	return CopyFileA(*srcFile, *dstFile, overwrite) == TRUE;
}
String Path::GetCurrentPath()
{
	char currDir[MAX_PATH];
	GetCurrentDirectoryA(sizeof(currDir), currDir);
	return currDir;
}
String Path::GetExecutablePath()
{
	char filename[MAX_PATH];
	GetModuleFileNameA(GetModuleHandle(0), filename, sizeof(filename));
	return filename;
}
String Path::GetTemporaryPath()
{
	char path[MAX_PATH];
	::GetTempPathA(sizeof(path), path);
	return path;
}
String Path::GetTemporaryFileName(const String& path, const String& prefix)
{
	char out[MAX_PATH];
	BOOL r = ::GetTempFileNameA(*path, *prefix, 0, out);
	assert(r == TRUE);
	return out;
}
bool Path::IsDirectory(const String& path)
{
	DWORD attribs = GetFileAttributesA(*path);
	return (attribs != INVALID_FILE_ATTRIBUTES) && (attribs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}
bool Path::FileExists(const String& path)
{
	return PathFileExistsA(*path) == TRUE;
}
String Path::Normalize(const String& path)
{
	char out[MAX_PATH];
	PathCanonicalizeA(out, *path);
	for(uint32 i = 0; i < MAX_PATH; i++)
	{
		if(out[i] == '/')
			out[i] = sep;
	}
	return out;
}
bool Path::IsAbsolute(const String& path)
{
	if(path.length() > 2 && path[1] == ':')
		return true;
	return false;
}

Vector<String> Path::GetSubDirs(const String& path)
{
	Vector<String> res;
	String searchPath = path + "\\*";
	WIN32_FIND_DATAA findData;
	HANDLE searchHandle = FindFirstFileA(*searchPath, &findData);
	if (searchHandle == INVALID_HANDLE_VALUE)
		return res;

	do
	{
		String filename = findData.cFileName;
		if (filename == ".")
			continue;
		if (filename == "..")
			continue;
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			res.Add(filename);
	} while (FindNextFileA(searchHandle, &findData));
	FindClose(searchHandle);

	return res;
}

bool Path::ShowInFileBrowser(const String& path)
{
	//Opens the directory, if a file path is sent then the file will be opened with the default program for that file type.
	long res = (long)ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	if (res > 32)
	{
		return true;
	}
	else
	{
		switch (res)
		{
		case ERROR_FILE_NOT_FOUND:
			Logf("Failed to show file \"%s\" in the system default explorer: File not found.", Logger::Error, path);
			break;
		case ERROR_PATH_NOT_FOUND:
			Logf("Failed to show file \"%s\" in the system default explorer: Path not found.", Logger::Error, path);
			break;
		default:
			Logf("Failed to show file \"%s\" in the system default explorer: error %p", Logger::Error, path, res);
			break;
		}
		return false;
	}
}

bool Path::Run(const String& programPath, const String& parameters)
{
	STARTUPINFOA info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;
	String command = Utility::Sprintf("%s %s", programPath.GetData(), parameters.GetData());

	if (!Path::FileExists(programPath))
	{
		Logf("Failed to open editor: invalid path \"%s\"", Logger::Error, programPath);
		return false;
	}

	if (CreateProcessA(NULL, command.GetData(), NULL, NULL, false, CREATE_NEW_CONSOLE, NULL, NULL, &info, &processInfo))
	{
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
	else
	{
		Logf("Failed to open editor: error %d", Logger::Error, GetLastError());
		return false;
	}
	return true;
}