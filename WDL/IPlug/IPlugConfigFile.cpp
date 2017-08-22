#include "IPlugConfigFile.h"

IPlugConfigFile::IPlugConfigFile()
{
	key.reserve(2048);
}

void IPlugConfigFile::UseEncryption(bool value)
{
	useEncryption = value;

	if (useEncryption)
	{
		if (key.size() == 0)
		{
			// Set default values
			key.push_back(7);
			key.push_back(6);
			key.push_back(5);
			key.push_back(1);
			key.push_back(3);
		}
	}
}

void IPlugConfigFile::SetEncriptionKey(unsigned numberOfValues, ...)
{
	key.resize(0);

	va_list vl;
	va_start(vl, numberOfValues);
	for (int i = 0; i < numberOfValues; i++)
	{
		unsigned tmp = va_arg(vl, unsigned);

		for (int s = 0; s < 31; s++)
		{
			unsigned char tmpChar = (unsigned char)(tmp >> s);

			while (tmpChar < 128)
			{
				tmpChar += (s + 1);
			}

			key.push_back(tmpChar);
		}
	}
	va_end(vl);
}

void IPlugConfigFile::SetFilePath(string _filePath)
{
	filePath = _filePath;
}

void IPlugConfigFile::ReadFile()
{
	ReadFromPath(filePath);

	if (useEncryption) EncryptDecryptString(&outputString);

	// If file is not valid, rewrite the file
	if (!GetGroupProps())
	{
		outputString.resize(0);
		WriteFile();
	}
}

void IPlugConfigFile::WriteFile()
{
	if (useEncryption) EncryptDecryptString(&outputString);

	WriteToPath(filePath);

	if (useEncryption) EncryptDecryptString(&outputString);
}

void IPlugConfigFile::EncryptDecryptString(string * workingString)
{
	if (key.size() > 0)
	{
		int keyPos = 0;
		for (int i = 0; i < workingString->size(); i++)
		{
			(*workingString)[i] = (*workingString)[i] ^ (i + key[keyPos]);

			keyPos++;
			if (keyPos >= key.size()) keyPos = 0;
		}
	}
}

void IPlugConfigFile::WriteString(string groupName, string valueName, string value, string comment)
{
	size_t groupPropsPos = FindGroupPropsPos(groupName);

	if (groupPropsPos == -1)
	{
		CreateGroup(groupName);
		groupPropsPos = groupPropsVector.size() - 1;
	}

	UpdateValue(groupPropsPos, valueName, value, comment);
}

string IPlugConfigFile::ReadString(string groupName, string valueName, string defaultValue)
{
	size_t groupPropsPos = FindGroupPropsPos(groupName);

	if (groupPropsPos == -1) return defaultValue;

	return GetValue(groupPropsPos, valueName, defaultValue);
}

void IPlugConfigFile::UpdateValue(size_t groupPropsPos, string valueName, string value, string comment)
{
	bool valueExist = true;

	size_t groupStartPos = groupPropsVector[groupPropsPos].groupStart;
	size_t groupEndPos = groupPropsVector[groupPropsPos].groupEnd;

	size_t valueStartPos = groupStartPos;
	size_t firstEqualPos = 0;
	size_t firstNewLinePos = 0;

	while (valueStartPos < groupEndPos)
	{
		valueStartPos = outputString.find(valueName, valueStartPos + 1);
		if (valueStartPos > groupEndPos)
		{
			valueExist = false;
			continue;
		}

		firstEqualPos = outputString.find('=', valueStartPos);
		firstNewLinePos = outputString.find('\n', valueStartPos);

		if (firstEqualPos > firstNewLinePos)
		{
			valueExist = false;
			continue;
		}

		if (valueExist) break;
	}

	string valueString;
	valueString.append(valueName);
	valueString.append(" = ");
	valueString.append(value);
	valueString.append(" ");

	if (comment.size() > 0)
	{
		valueString.append("// ");
		valueString.append(comment);
	}

	if (valueExist)
	{
		size_t replaceSize = firstNewLinePos - valueStartPos;
		outputString.replace(valueStartPos, replaceSize, valueString);

		UpdateGroupPos(groupPropsPos, valueString.size() - replaceSize);
	}
	else
	{
		valueString.append("\n");
		outputString.insert(groupEndPos, valueString);

		UpdateGroupPos(groupPropsPos, valueString.size());
	}

}

string IPlugConfigFile::GetValue(size_t groupPropsPos, string valueName, string defaultValue)
{
	bool valueExist = true;

	size_t groupStartPos = groupPropsVector[groupPropsPos].groupStart;
	size_t groupEndPos = groupPropsVector[groupPropsPos].groupEnd;

	size_t valueStartPos = groupStartPos;
	size_t firstEqualPos = 0;
	size_t firstCommentPos = 0;
	size_t firstNewLinePos = 0;

	while (valueStartPos < groupEndPos)
	{
		valueStartPos = outputString.find(valueName, valueStartPos + 1);
		if (valueStartPos > groupEndPos)
		{
			valueExist = false;
			continue;
		}

		firstEqualPos = outputString.find('=', valueStartPos);
		firstCommentPos = outputString.find('/', valueStartPos);
		firstNewLinePos = outputString.find('\n', valueStartPos);

		if (firstEqualPos > firstNewLinePos)
		{
			valueExist = false;
			continue;
		}

		if (valueExist) break;
	}

	if (valueExist)
	{
		size_t valueStart = firstEqualPos + 2;
		size_t valueEnd = IPMIN(firstNewLinePos, firstCommentPos) - 1;

		string outValue(outputString.begin() + valueStart, outputString.begin() + valueEnd);

		return outValue;
	}

	return defaultValue;
}

void IPlugConfigFile::UpdateGroupPos(size_t fromGroup, size_t move)
{
	groupPropsVector[fromGroup].groupEnd += move;

	for (size_t i = fromGroup + 1; i < groupPropsVector.size(); i++)
	{
		groupPropsVector[i].groupStart += move;
		groupPropsVector[i].groupEnd += move;
	}
}

size_t IPlugConfigFile::FindGroupPropsPos(string groupName)
{
	for (size_t i = 0; i < groupPropsVector.size(); i++)
	{
		if (groupPropsVector[i].groupName == groupName) return i;
	}

	return -1;
}

bool IPlugConfigFile::GetGroupProps()
{
	groupPropsVector.resize(0);

	// Get open and closed brace
	size_t openBracePosition = 0;
	size_t nextOpenBracePosition = 0;
	size_t closedBracePosition = 0;

	while (true)
	{
		openBracePosition = outputString.find('{', openBracePosition + 1);
		if (openBracePosition != string::npos) nextOpenBracePosition = outputString.find('{', openBracePosition + 1);
		closedBracePosition = outputString.find('}', closedBracePosition + 1);

		// Check if braces are valid
		if (openBracePosition > closedBracePosition) return false;
		if (closedBracePosition > nextOpenBracePosition) return false;
		if (openBracePosition == string::npos && closedBracePosition != string::npos) return false;
		if (openBracePosition != string::npos && closedBracePosition == string::npos) return false;

		if (openBracePosition != string::npos && closedBracePosition != string::npos)
		{
			groupPropsVector.resize(groupPropsVector.size() + 1);

			groupPropsVector.back().groupStart = openBracePosition;
			groupPropsVector.back().groupEnd = closedBracePosition;
		}
		else break;
	}

	// Get group names
	for (size_t i = 0; i < groupPropsVector.size(); i++)
	{
		size_t groupStartPos = GetGroupLabelStartPosition(groupPropsVector[i].groupStart, groupPropsVector[i].groupEnd);
		size_t groupEndPos = groupPropsVector[i].groupStart - 1;

		groupPropsVector[i].groupName = string(outputString.begin() + groupStartPos, outputString.begin() + groupEndPos);
	}

	return true;
}

size_t IPlugConfigFile::GetGroupLabelStartPosition(size_t openBracePos, size_t closedBracePos)
{
	size_t position = openBracePos - 2;

	while (position > 0 && position < closedBracePos)
	{
		if (outputString[position] == '\n') return position + 1;
		else position--;
	}

	return 0;
}

void IPlugConfigFile::ReadFromPath(string filePath)
{
	outputString.resize(0);

	ifstream myfile(filePath);

	if (myfile.is_open())
	{
		myfile.seekg(0, std::ios::end);
		outputString.reserve((unsigned int)myfile.tellg());
		myfile.seekg(0, std::ios::beg);

		outputString.assign((std::istreambuf_iterator<char>(myfile)),
			std::istreambuf_iterator<char>());
		myfile.close();
	}
}

void IPlugConfigFile::WriteToPath(string filePath)
{
	ofstream myfile;
	myfile.open(filePath);
	if (myfile.is_open())
	{
		myfile << outputString;
		myfile.close();
	}
}

void IPlugConfigFile::RemoveComment(string * line)
{
	if (line->find("//") != line->npos)
		line->erase(line->find("//"));
}

void IPlugConfigFile::CreateGroup(string groupName)
{
	groupPropsVector.resize(groupPropsVector.size() + 1);

	groupPropsVector.back().groupName = groupName;

	if (outputString.size() != 0)
	{
		outputString.append("\n");
	}

	outputString.append(groupName);

	groupPropsVector.back().groupStart = outputString.size();
	outputString.append("\n{\n");

	groupPropsVector.back().groupEnd = outputString.size();
	outputString.append("}\n");
}

#ifdef __APPLE__
template <>
std::string IPlugConfigFile::string_to_T<std::string>(std::string const &val)
{
	return val;
}
#endif // __APPLE__