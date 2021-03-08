#include <regex>
#include <string>

int CheckPatchFileName(char* fileName)
{
	std::string checkName(fileName);
	std::regex rx("\\d\\d\\d\\d\\dto\\d\\d\\d\\d\\d\\.patch");
	return std::regex_match(checkName.begin(), checkName.end(), rx);
}