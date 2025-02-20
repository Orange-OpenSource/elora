/*
 * Copyright (c) 2025 University of Bologna
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@unibo.it>
 */

#include "rest-api-helper.h"

#include "ns3/assert.h"
#include "ns3/log.h"

#include <sstream>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("RestApiHelper");

RestApiHelper::RestApiHelper()
    : m_baseUrl(""),
      m_header(nullptr)
{
    /* Init curl */
    curl_global_init(CURL_GLOBAL_NOTHING);
    /* Init handle */
    if (m_curl = curl_easy_init(); !m_curl)
    {
        NS_FATAL_ERROR("curl_easy_init() failed.");
    }
}

RestApiHelper::~RestApiHelper()
{
    NS_LOG_FUNCTION_NOARGS();
    /* Cleanup handle */
    curl_easy_cleanup(m_curl);
    /* Cleanup curl */
    curl_global_cleanup();
}

int
RestApiHelper::InitConnection(const str address, uint16_t port, const str token)
{
    NS_LOG_FUNCTION(this << address << (unsigned)port);

    /* Setup base URL string with IP and port */
    m_baseUrl = "http://" + address + ":" + std::to_string(port);
    NS_LOG_INFO("REST API URL base set to: " << m_baseUrl);

    /* Initialize HTTP header fields */
    NS_ASSERT_MSG(!token.empty(), "Empty API token.");
    curl_slist_free_all(m_header); /* free the header list if previously set */
    m_header = curl_slist_append(m_header, ("Authorization: Bearer " + token).c_str());

    /* Set HTTP header in handles */
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_header);
    /* Set reply write callback */
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, StringWriteCallback);

    return DoConnect();
}

int
RestApiHelper::GET(const str& path, str& out) const
{
    /* Set the request type to GET */
    curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1);
    /* Reset any custom request type */
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, NULL);

    /* Perform the request */
    NS_LOG_INFO("Sending request to " << m_baseUrl << path);
    return ExecuteRequest(m_curl, m_baseUrl + path, out);
}

int
RestApiHelper::POST(const str& path, const str& body, str& out) const
{
    /* Set request type to POST and add body */
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
    /* Reset any custom request type */
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, NULL);

    /* Perform the request */
    NS_LOG_INFO("Sending request to " << m_baseUrl << path << ", with body: " << body);
    return ExecuteRequest(m_curl, m_baseUrl + path, out);
}

int
RestApiHelper::PUT(const str& path, const str& body, str& out) const
{
    /* Set request type to POST and add body */
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
    /* Set PUT as custom request type */
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "PUT");

    /* Perform the request */
    NS_LOG_INFO("Sending request to " << m_baseUrl << path << ", with body: " << body);
    return ExecuteRequest(m_curl, m_baseUrl + path, out);
}

int
RestApiHelper::DELETE(const str& path, str& out) const
{
    /* Set the request type to GET */
    curl_easy_setopt(m_curl, CURLOPT_HTTPGET, 1);
    /* Set DELETE as custom request type */
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    /* Perform the request */
    NS_LOG_INFO("Sending request to " << m_baseUrl << path);
    return ExecuteRequest(m_curl, m_baseUrl + path, out);
}

int
RestApiHelper::ExecuteRequest(CURL* handle, const str& url, str& out)
{
    long response_code;

    /* Set the destination URL of our request. */
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    /* Set reply stringstream */
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&out);

    /* Perform the request */
    if (auto res = curl_easy_perform(handle); res != CURLE_OK)
    {
        NS_LOG_ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res) << ".");
        return EXIT_FAILURE;
    }

    /* Check HTTP response code */
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200)
    {
        NS_LOG_ERROR("Expected response code 200, but got " << response_code << ".");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

size_t
RestApiHelper::StringWriteCallback(char* buffer, size_t size, size_t nmemb, void* string)
{
    size_t realsize = size * nmemb;
    if (auto s = (str*)(string); s)
    {
        s->assign(buffer, realsize);
        return realsize;
    }
    return 0;
}

} // namespace lorawan
} // namespace ns3
