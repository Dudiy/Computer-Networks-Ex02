#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <map>
using namespace std;

class HttpResponse
{
public:
	HttpResponse(string i_RootPath) { m_RootPath.append("D:\\School\\Year3\\Root"); }
	size_t getContentLength() const { return m_Data.length(); }
	string getStatusCode() const { return m_StatusCode; };
	void addHeaderLine(string i_Name, string i_Value);
	void setOkStatusLine() { setStatusLine(200, "OK"); }
	string toString();
	void setStatusLine(size_t i_StatusCode, string i_StatusPhrase, string i_Protocol = "HTTP/1.1");	
	size_t setData(string i_Data);		
private:	
	string m_RootPath = string();	
	string m_Protocol = "";
	string m_StatusCode = "";
	string m_StatusPhrase = "";
	map<string, string> m_Headers;
	string m_Data = "";
	size_t m_FullResponseLength = 0;	
};