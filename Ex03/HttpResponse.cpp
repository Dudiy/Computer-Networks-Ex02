#include "HttpResponse.h"

HttpResponse::HttpResponse(string i_RootPath)
{
	m_RootPath.append("D:\\School\\Year3\\Root");
}

string HttpResponse::toString()
{
	string responseStr = string(m_Protocol + ' ' + m_StatusCode + ' ' + m_StatusPhrase + '\n');
	for (auto itr = m_Headers.begin(); itr != m_Headers.end(); ++itr) {
		responseStr.append(itr->first + ": " + itr->second + '\n');
	}
	responseStr.append(m_Data);

	return responseStr;
}

void HttpResponse::setStatusLine(size_t i_StatusCode, string i_StatusPhrase, string i_Protocol)
{
	string statusCodeStr = to_string(i_StatusCode);
	m_FullResponseLength += i_Protocol.length() + 1 + statusCodeStr.length() + 1 + i_StatusPhrase.length() + 1;
	m_Protocol = i_Protocol;
	m_StatusCode = statusCodeStr;
	m_StatusPhrase = i_StatusPhrase;
}

void HttpResponse::setOkStatusLine()
{
	setStatusLine(200, "OK");
}

void HttpResponse::addHeaderLine(string i_Name, string i_Value)
{
	m_Headers.insert(pair<string, string>(i_Name, i_Value));
	m_FullResponseLength += i_Name.length() + 2 + i_Value.length() + 1;
}

size_t HttpResponse::setData(string i_Data)
{
	m_Data = i_Data;
	m_FullResponseLength += i_Data.length() + 2;
	return m_Data.length();
}

size_t HttpResponse::getContentLength()
{
	return m_Data.length();
}
