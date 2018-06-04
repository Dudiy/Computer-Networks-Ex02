#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <map>
using namespace std;

class HttpRequest
{
public:
	HttpRequest();
	bool initFromString(string i_RequestStr);
	string getMethod();
	string getURL();
	string getVersion();
	map<string, string> getHeaders();
	string getBody();
private:
	string m_Method;
	string m_Url;
	string m_Version;
	map<string, string> m_Headers;
	string m_Body;
};