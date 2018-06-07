#include "HttpRequest.h"

HttpRequest::HttpRequest(const string i_RootPath)
	: m_RootPath(i_RootPath)
{
}

int HttpRequest::initFromString(string i_RequestStr)
{
	m_RequestStr = i_RequestStr;
	const size_t bodyStartIndex = getBodyStartIndex();
	size_t sizeParsed;
	char* requstStr = new char[i_RequestStr.length() + 1];
	strcpy(requstStr, i_RequestStr.c_str());
	char* bodyPtr = requstStr + bodyStartIndex;
	try
	{
		m_Method = strtok(requstStr, " ");
		m_Url = strtok(nullptr, " ");
		m_Version = strtok(nullptr, " \r\n");
		char* currHeader = strtok(nullptr, "\r\n\t");
		while (currHeader != nullptr && (bodyPtr == nullptr || (!bodyPtr != NULL && currHeader < bodyPtr)))
		{
			string header = string(currHeader);
			size_t pos = header.find(':');
			string headerName = header.substr(0, pos++);
			while (header[pos] == ' ')
			{
				pos++;
			}
			string headerValue = header.substr(pos);
			m_Headers.insert(make_pair(headerName, headerValue));
			currHeader = strtok(nullptr, "\t\r\n");
		}
		size_t contentLength = 0;
		if (i_RequestStr.length() != bodyStartIndex)
		{
			m_Body = string(bodyPtr);
			contentLength = fmin(getContentLength(), m_Body.length());
			if (contentLength > 0)
			{
				m_Body = m_Body.substr(0, contentLength);
			}
		}
		sizeParsed = contentLength + bodyStartIndex;
	}
	catch (exception e)
	{
		sizeParsed = -1;
	}
	return sizeParsed;
}

size_t HttpRequest::getBodyStartIndex() const
{
	const char* requestStr = m_RequestStr.c_str();
	size_t position = m_RequestStr.find("\n\n");
	if (position != string::npos)
		return position + 2;

	position = m_RequestStr.find("\r\n\r\n");
	if (position != string::npos)
		return position + 4;

	return -1;
}

size_t HttpRequest::getContentLength()
{
	size_t contentLength = -1;

	try
	{
		if (!m_Headers["Content-length"].empty())
		{
			contentLength = stoi(m_Headers["Content-length"]);
		}
	}
	catch (exception e)
	{
		contentLength = -1;
	}

	return contentLength;
}
