#include "HttpRequest.h"

HttpRequest::HttpRequest()
{
	char buf[256];
	GetCurrentDirectoryA(256, buf);
	HttpRequest(string(buf) + '\\');
}

HttpRequest::HttpRequest(string i_RootPath)
{
	m_RootPath = i_RootPath;
}

int HttpRequest::initFromString(string i_RequestStr)
{
	m_RequestStr = i_RequestStr.c_str();
	size_t bodyStartIndex = getBodyStartIndex();
	size_t sizeParsed = 0;
	HttpRequest parsedRequest = HttpRequest();
	size_t charsBetweenHeadersAndBody = 0;
	char* requstStr = new char[i_RequestStr.length() + 1];
	strcpy(requstStr, i_RequestStr.c_str());
	char* bodyPtr = requstStr + bodyStartIndex;
	try {
		m_Method = strtok(requstStr, " ");
		m_Url = strtok(NULL, " ");
		m_Version = strtok(NULL, " \r\n");
		char* currHeader = strtok(NULL, "\r\n\t");
		while (currHeader != NULL && (bodyPtr == NULL || (!bodyPtr != NULL && currHeader < bodyPtr))) {
			string header = string(currHeader);
			size_t pos = header.find(':');
			string headerName = header.substr(0, pos++);
			while (header[pos] == ' ') {
				pos++;
			}
			string headerValue = header.substr(pos);
			m_Headers.insert(make_pair(headerName, headerValue));
			currHeader = strtok(NULL, "\t\r\n");
		}
		m_Body = string(bodyPtr);
		size_t contentLength = fmin(getContentLength(), m_Body.length());
		if (contentLength > 0) {
			m_Body = m_Body.substr(0, contentLength);
		}
		sizeParsed = contentLength + bodyStartIndex;
	}
	catch (exception e) {
		sizeParsed = -1;
	}
	return sizeParsed;
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

size_t HttpRequest::getBodyStartIndex()
{
	size_t index = -1;
	const char* position = strstr(m_RequestStr, "\n\n");
	if (position != NULL)
		return position - m_RequestStr + 2;

	position = strstr(m_RequestStr, "\r\n\r\n");
	if (position != NULL)
		return position - m_RequestStr + 4;

	return -1;
}

size_t HttpRequest::getContentLength()
{
	size_t contentLength = -1;

	try {
		if (!m_Headers["Content-length"].empty()) {
			contentLength = stoi(m_Headers["Content-length"]);
		}
	}
	catch (exception e) {
		contentLength = -1;
	}

	return contentLength;
}
