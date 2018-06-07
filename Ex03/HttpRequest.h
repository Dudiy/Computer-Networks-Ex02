#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <map>
using namespace std;

class HttpRequest
{
public:
	HttpRequest(const string i_RootPath);		
	string getMethod() const { return m_Method; }
	string getURL() const { return m_Url; }
	string getVersion() const { return m_Version; }
	map<string, string> getHeaders() const { return m_Headers; }
	string getBody() const { return m_Body; }
	size_t getRequestLength() const { return m_RequestStr.length(); }
	string toString() const { return string(m_RequestStr); }
	int initFromString(string i_RequestStr);
private:
	string m_RequestStr;
	string m_RootPath;
	string m_Method = "";
	string m_Url = "";
	string m_Version = "";
	map<string, string> m_Headers;
	string m_Body = "";
	size_t getBodyStartIndex() const;
	size_t getContentLength();
};
