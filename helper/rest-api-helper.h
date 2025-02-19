/*
 * Copyright (c) 2025 University of Bologna
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@unibo.it>
 */

#ifndef REST_API_HELPER_H
#define REST_API_HELPER_H

#include <curl/curl.h>
#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * This class implements base functionality for a REST HTTP API with token auth.
 *
 * \warning Requires libcurl-dev installed.
 */
class RestApiHelper
{
    using str = std::string;

  public:
    int InitConnection(const str address, uint16_t port, const str token);

  protected:
    RestApiHelper();

    ~RestApiHelper();

    int GET(const str& path, str& out) const;

    int POST(const str& path, const str& body, str& out) const;

    int PUT(const str& path, const str& body, str& out) const;

    int DELETE(const str& path, str& out) const;

  private:
    virtual int DoConnect() = 0;

    virtual void CloseConnection(int signal) = 0;

    static int ExecuteRequest(CURL* handle, const str& url, str& out);

    static size_t StringWriteCallback(char* buffer, size_t size, size_t nmemb, void* string);

    str m_baseUrl;

    struct curl_slist* m_header;

    CURL* m_curl_handle_get;
    CURL* m_curl_handle_post;
    CURL* m_curl_handle_put;
    CURL* m_curl_handle_delete;
};

} // namespace lorawan
} // namespace ns3

#endif /* REST_API_HELPER_H */