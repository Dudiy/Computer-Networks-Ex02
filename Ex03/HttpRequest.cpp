#include "HttpRequest.h"

HttpRequest::HttpRequest()
{
}

bool HttpRequest::initFromString(string i_RequestStr)
{
	HttpRequest parsedRequest = HttpRequest();
	bool parseSuccess = true;
	char* requstStr = new char[i_RequestStr.length() + 1];
	strcpy(requstStr, i_RequestStr.c_str());

	try {		
		m_Method = strtok(requstStr, " ");
		m_Url = strtok(NULL, " ");
		m_Version = strtok(NULL, " \r\n");
		char* currHeader = strtok(NULL, "\t\r\n");
		while (currHeader != NULL) {
			string header = string(currHeader);
			size_t pos = header.find(':');
			string headerName = header.substr(0,pos++);						
			while (header[pos] == ' ') {
				pos++;
			}				
			string headerValue = header.substr(pos);
			m_Headers.insert(make_pair(headerName, headerValue));
			currHeader = strtok(NULL, "\t\r\n");
		}
		// TODO get request body
	}
	catch (exception e) {
		parseSuccess = false;
	}
	return parseSuccess;
}

string HttpRequest::getMethod()
{
	return m_Method;
}

string HttpRequest::getURL()
{
	return m_Url;
}

string HttpRequest::getVersion()
{
	return m_Version;
}

map<string, string> HttpRequest::getHeaders()
{
	return m_Headers;
}

string HttpRequest::getBody()
{
	return m_Body;
}
