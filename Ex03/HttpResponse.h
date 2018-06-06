#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <map>
#include <cctype>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <Windows.h>
using namespace std;

class HttpResponse
{
public:
	HttpResponse(string i_RootPath);
	string toString();
	void setStatusLine(size_t i_StatusCode, string i_StatusPhrase, string i_Protocol = "HTTP/1.1");	
	void setOkStatusLine();
	void addHeaderLine(string i_Name, string i_Value);
	size_t setData(string i_Data);		
	size_t getContentLength();
private:	
	string m_RootPath = string();	
	string m_Protocol = "";
	string m_StatusCode = "";
	string m_StatusPhrase = "";
	map<string, string> m_Headers;
	string m_Data = "";
	size_t m_FullResponseLength = 0;	
};