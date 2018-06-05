#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <map>
#include <cctype>
#include <math.h>
#include <Windows.h>
using namespace std;

class HttpRequest
{
public:
	HttpRequest();
	HttpRequest(string i_RootPath);
	int initFromString(string i_RequestStr);
	string getMethod();
	string getURL();
	string getVersion();
	map<string, string> getHeaders();
	string getBody();
	size_t getRequestLength();
private:
	const char* m_RequestStr;
	string m_RootPath;
	string m_Method = "";
	string m_Url = "";
	string m_Version = "";
	map<string, string> m_Headers;
	string m_Body = "";
	size_t m_FullRequestLength = 0;
	size_t getBodyStartIndex();
	size_t getContentLength();
};