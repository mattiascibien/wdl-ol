#pragma once

/*
Copyright (C) 2016 and later, Youlean

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. This notice may not be removed or altered from any source distribution.

*/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "IPlugStructs.h"

using namespace std;

class IPlugConfigFile
{
public:
	void UseEncryption(bool value);
	void SetEncriptionKey(unsigned numberOfValues, ...);

	void SetFilePath(string _filePath);

	void ReadFile();
	void WriteFile();

	template <typename ValueType>
	void WriteValue(string groupName, string valueName, ValueType value, string comment = "");

	template <typename ValueType>
	ValueType ReadValue(string groupName, string valueName, ValueType defaultValue);
	
private:
	struct GroupProps
	{
		string groupName;
		size_t groupStart;
		size_t groupEnd;
	};

	vector<GroupProps> groupPropsVector;
	string outputString;
	string key;
	bool useEncryption = false;
	string filePath;

	// Convert T, which should be a primitive, to a std::string.
	template <typename T>
	static std::string T_to_string(T const &val);

	// Convert a std::string to T.	
	template <typename T>
	static T string_to_T(std::string const &val);

	template <>
	static std::string string_to_T(std::string const &val);

	void EncryptDecryptString(string *workingString);
	void WriteString(string groupName, string valueName, string value, string comment = "");
	string ReadString(string groupName, string valueName, string defaultValue);
	void UpdateValue(size_t groupPropsPos, string valueName, string value, string comment = "");
	string GetValue(size_t groupPropsPos, string valueName, string defaultValue);
	void UpdateGroupPos(size_t fromGroup, size_t move);
	size_t FindGroupPropsPos(string groupName);
	bool GetGroupProps();
	size_t GetGroupLabelStartPosition(size_t openBracePos, size_t closedBracePos);
	void ReadFromPath(string filePath);
	void WriteToPath(string filePath);
	void RemoveComment(string *line);
	void CreateGroup(string groupName);
};
