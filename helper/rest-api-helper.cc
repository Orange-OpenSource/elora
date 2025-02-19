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
    NS_LOG_FUNCTION_NOARGS();
    /* Init curl */
    curl_global_init(CURL_GLOBAL_NOTHING);
    /* Init handles */
    m_curl_handle_get = curl_easy_init();
    m_curl_handle_post = curl_easy_init();
    m_curl_handle_put = curl_easy_init();
    m_curl_handle_delete = curl_easy_init();
}

RestApiHelper::~RestApiHelper()
{
    NS_LOG_FUNCTION_NOARGS();
    /* Cleanup handles */
    curl_easy_cleanup(m_curl_handle_get);
    curl_easy_cleanup(m_curl_handle_post);
    curl_easy_cleanup(m_curl_handle_put);
    curl_easy_cleanup(m_curl_handle_delete);
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
    m_header = curl_slist_append(m_header, "Accept: application/json");
    m_header = curl_slist_append(m_header, "Content-Type: application/json");

    /* Set HTTP header in handles */
    curl_easy_setopt(m_curl_handle_get, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(m_curl_handle_post, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(m_curl_handle_put, CURLOPT_HTTPHEADER, m_header);
    curl_easy_setopt(m_curl_handle_delete, CURLOPT_HTTPHEADER, m_header);

    /* Set reply write callback */
    curl_easy_setopt(m_curl_handle_get, CURLOPT_WRITEFUNCTION, StringWriteCallback);
    curl_easy_setopt(m_curl_handle_post, CURLOPT_WRITEFUNCTION, StringWriteCallback);
    curl_easy_setopt(m_curl_handle_put, CURLOPT_WRITEFUNCTION, StringWriteCallback);
    curl_easy_setopt(m_curl_handle_delete, CURLOPT_WRITEFUNCTION, StringWriteCallback);

    return DoConnect();
}

int
RestApiHelper::GET(const str& path, str& out) const
{
    /* Check handle */
    if (!m_curl_handle_get)
    {
        NS_LOG_ERROR("curl_easy_init() failed\n");
        return EXIT_FAILURE;
    }

    /* Perform the request */
    NS_LOG_INFO("Sending GET request to " << m_baseUrl << path);
    int status = ExecuteRequest(m_curl_handle_get, m_baseUrl + path, out);
    NS_LOG_INFO("Received GET reply: " << out);

    return status;
}

int
RestApiHelper::POST(const str& path, const str& body, str& out) const
{
    /* Check handle */
    if (!m_curl_handle_post)
    {
        NS_LOG_ERROR("curl_easy_init() failed\n");
        return EXIT_FAILURE;
    }

    /* Add request body */
    curl_easy_setopt(m_curl_handle_post, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(m_curl_handle_post, CURLOPT_POSTFIELDSIZE, (long)body.size());

    /* Perform the request */
    NS_LOG_INFO("Sending POST request to " << m_baseUrl << path << ", with body: " << body);
    int status = ExecuteRequest(m_curl_handle_post, m_baseUrl + path, out);
    NS_LOG_INFO("Received POST reply: " << out);

    return status;
}

int
RestApiHelper::PUT(const str& path, const str& body, str& out) const
{
    /* Check handle */
    if (!m_curl_handle_put)
    {
        NS_LOG_ERROR("curl_easy_init() failed\n");
        return EXIT_FAILURE;
    }

    /* Set the request type to PUT */
    curl_easy_setopt(m_curl_handle_put, CURLOPT_CUSTOMREQUEST, "PUT");
    /* Add request body */
    curl_easy_setopt(m_curl_handle_put, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(m_curl_handle_put, CURLOPT_POSTFIELDSIZE, (long)body.size());

    /* Perform the request */
    NS_LOG_INFO("Sending PUT request to " << m_baseUrl << path << ", with body: " << body);
    int status = ExecuteRequest(m_curl_handle_put, m_baseUrl + path, out);
    NS_LOG_INFO("Received PUT reply: " << out);

    return status;
}

int
RestApiHelper::DELETE(const str& path, str& out) const
{
    /* Check handle */
    if (!m_curl_handle_delete)
    {
        NS_LOG_ERROR("curl_easy_init() failed\n");
        return EXIT_FAILURE;
    }

    /* Set the request type to DELETE */
    curl_easy_setopt(m_curl_handle_delete, CURLOPT_CUSTOMREQUEST, "DELETE");

    /* Perform the request */
    NS_LOG_INFO("Sending DELETE request to " << m_baseUrl << path);
    int status = ExecuteRequest(m_curl_handle_delete, m_baseUrl + path, out);
    NS_LOG_INFO("Received DELETE reply: " << out);

    return status;
}

int
RestApiHelper::ExecuteRequest(CURL* handle, const str& url, str& out)
{
    /* Check handle */
    if (!handle)
    {
        NS_LOG_ERROR("curl_easy_init() failed\n");
        return EXIT_FAILURE;
    }

    long response_code;

    /* Set the destination URL of our request. */
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    /* Set reply stringstream */
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&out);

    /* Perform the request */
    if (auto res = curl_easy_perform(handle); res != CURLE_OK)
    {
        NS_LOG_ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n");
        return EXIT_FAILURE;
    }

    /* Check HTTP response code */
    if (auto res = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
        res != CURLE_OK)
    {
        NS_LOG_ERROR("curl_easy_getinfo() failed: " << curl_easy_strerror(res) << "\n");
        return EXIT_FAILURE;
    }
    else if (response_code != 200)
    {
        NS_LOG_ERROR("Unexpected response code: " << response_code << "\n");
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
