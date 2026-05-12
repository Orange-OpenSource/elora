/*
 * Copyright (c) 2026 University of Bologna
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@unibo.it>
 */

#include "otlp-http-helper.h"

#include "ns3/log.h"
#include "ns3/parson.h"

#include <sstream>
#include <unistd.h>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("OtlpHttpHelper");

OtlpHttpHelper::OtlpHttpHelper()
    : m_headers(nullptr)
{
    gethostname(m_hostname, HOST_NAME_MAX + 1);

    m_headers = curl_slist_append(m_headers, "Content-Type: application/json");

    m_handle = curl_easy_init();
    /* Set HTTP header in handles */
    curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, m_headers);
    /* Set reply write callback */
    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, StringWriteCallback);
}

OtlpHttpHelper::~OtlpHttpHelper()
{
    NS_LOG_FUNCTION_NOARGS();
    curl_easy_cleanup(m_handle);
    m_handle = nullptr;
    curl_slist_free_all(m_headers);
    m_headers = nullptr;
}

int
OtlpHttpHelper::PostMetrics(const std::string& url, const MetricVec& metrics)
{
    NS_LOG_FUNCTION(this << url << &metrics);

    CURLcode result;
    long response_code;
    str reply;

    str body = BuildJson(metrics); // construct payload
    NS_LOG_DEBUG("OTLP JSON payload: " << body);

    /* Set request URL with metrics endpoint path */
    curl_easy_setopt(m_handle, CURLOPT_URL, (url + "/v1/metrics").c_str());
    /* Set request body */
    curl_easy_setopt(m_handle, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(m_handle, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)body.size());
    /* Set reply stringstream */
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, (void*)&reply);

    /* Perform the request */
    if (result = curl_easy_perform(m_handle); result != CURLE_OK)
    {
        NS_LOG_ERROR("curl_easy_perform() failed: " << curl_easy_strerror(result) << ".");
        return EXIT_FAILURE;
    }

    /* Check HTTP response code */
    curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200)
    {
        NS_LOG_ERROR("Expected response code 200, but got " << response_code << ": " << reply);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

std::string
OtlpHttpHelper::BuildJson(const MetricVec& metrics)
{
    // construct json object
    auto jsonMetricsData = json_value_init_object();
    // resource_metrics
    auto jsonResourceMetricsArray = json_value_init_array();
    auto jsonResourceMetrics = json_value_init_object();
    {
        // resource
        {
            // attributes
            auto jsonAttributes = json_value_init_array();
            auto jsonKeyValue = json_value_init_object();
            json_object_set_string(json_object(jsonKeyValue), "key", "hostname");
            json_object_dotset_string(json_object(jsonKeyValue), "value.stringValue", m_hostname);
            json_array_append_value(json_array(jsonAttributes), jsonKeyValue);
            json_object_dotset_value(json_object(jsonResourceMetrics),
                                     "resource.attributes",
                                     jsonAttributes);
            // dropped_attributes_count: not impl
            // entity_refs: not impl
        }
        // scope_metrics
        auto jsonScopeMetricsArray = json_value_init_array();
        auto jsonScopeMetrics = json_value_init_object();
        {
            // scope
            {
                json_object_dotset_string(json_object(jsonScopeMetrics), "scope.name", "ns-3");
                /// TODO: add ns-3 version here, see ns3/version.h (requires --enable-build-version,
                /// git, tags present in .git, ...)
                // version: not impl
                // attributes: not impl
                // dropped_attributes_count: not impl
            }
            // metrics
            auto jsonMetricsArray = json_value_init_array();
            for (auto& metric : metrics)
            {
                auto jsonMetric = (JSON_Value*)MetricToJson(metric);
                json_array_append_value(json_array(jsonMetricsArray), jsonMetric);
            }
            json_object_set_value(json_object(jsonScopeMetrics), "metrics", jsonMetricsArray);
            // schema_url: not impl
        }
        json_array_append_value(json_array(jsonScopeMetricsArray), jsonScopeMetrics);
        json_object_set_value(json_object(jsonResourceMetrics),
                              "scopeMetrics",
                              jsonScopeMetricsArray);
        // schema_url: not impl
        json_array_append_value(json_array(jsonResourceMetricsArray), jsonResourceMetrics);
        json_object_set_value(json_object(jsonMetricsData),
                              "resourceMetrics",
                              jsonResourceMetricsArray);
    }
    // serialize to string
    auto json = std::string(json_serialize_to_string(jsonMetricsData));
    // free json helper
    json_value_free(jsonMetricsData);
    return json;
}

void*
OtlpHttpHelper::MetricToJson(const Metric& metric)
{
    auto jsonMetric = json_value_init_object();
    // name
    json_object_set_string(json_object(jsonMetric), "name", metric.name.c_str());
    // description
    json_object_set_string(json_object(jsonMetric), "description", metric.description.c_str());
    // unit
    json_object_set_string(json_object(jsonMetric), "unit", metric.unit.c_str());
    // data
    switch (metric.data)
    {
        // gauge
    case Metric::GAUGE: {
        {
            // data_points
            if (!metric.gauge.dataPoints.empty())
            {
                auto jsonDataPoints = json_value_init_array();
                for (auto& numberDataPoint : metric.gauge.dataPoints)
                {
                    auto jsonNumberDataPoint = (JSON_Value*)NumberDataPointToJson(numberDataPoint);
                    json_array_append_value(json_array(jsonDataPoints), jsonNumberDataPoint);
                }
                json_object_dotset_value(json_object(jsonMetric),
                                         "gauge.dataPoints",
                                         jsonDataPoints);
            }
        }
    }
    break;
        // sum
    case Metric::SUM: {
        {
            // data_points
            if (!metric.sum.dataPoints.empty())
            {
                auto jsonDataPoints = json_value_init_array();
                for (auto& numberDataPoint : metric.sum.dataPoints)
                {
                    auto jsonNumberDataPoint = (JSON_Value*)NumberDataPointToJson(numberDataPoint);
                    json_array_append_value(json_array(jsonDataPoints), jsonNumberDataPoint);
                }
                json_object_dotset_value(json_object(jsonMetric), "sum.dataPoints", jsonDataPoints);
                // aggregation_temporality
                json_object_dotset_number(json_object(jsonMetric),
                                          "sum.aggregationTemporality",
                                          metric.sum.aggregationTemporality);
                // is_monotonic
                json_object_dotset_boolean(json_object(jsonMetric),
                                           "sum.isMonotonic",
                                           metric.sum.isMonotonic);
            }
        }
    }
    break;
        // histogram: not impl
        // exponential_histogram: not impl
        // summary: not impl
    }
    // metadata
    if (!metric.metadata.empty())
    {
        auto jsonMetadata = json_value_init_array();
        for (auto [key, value] : metric.metadata)
        {
            auto jsonKeyValue = json_value_init_object();
            json_object_set_string(json_object(jsonKeyValue), "key", key.c_str());
            json_object_dotset_string(json_object(jsonKeyValue),
                                      "value.stringValue",
                                      value.c_str());
            json_array_append_value(json_array(jsonMetadata), jsonKeyValue);
        }
        json_object_set_value(json_object(jsonMetric), "metadata", jsonMetadata);
    }
    return jsonMetric;
}

void*
OtlpHttpHelper::NumberDataPointToJson(const NumberDataPoint& numberDataPoint)
{
    auto jsonNumberDataPoint = json_value_init_object();
    // attributes
    if (!numberDataPoint.attributes.empty())
    {
        auto jsonAttributes = json_value_init_array();
        for (auto [key, value] : numberDataPoint.attributes)
        {
            auto jsonKeyValue = json_value_init_object();
            json_object_set_string(json_object(jsonKeyValue), "key", key.c_str());
            json_object_dotset_string(json_object(jsonKeyValue),
                                      "value.stringValue",
                                      value.c_str());
            json_array_append_value(json_array(jsonAttributes), jsonKeyValue);
        }
        json_object_set_value(json_object(jsonNumberDataPoint), "attributes", jsonAttributes);
    }
    // start_time_unix_nano
    auto startTimeUnixNano = std::to_string(numberDataPoint.startTimeUnixNano);
    json_object_set_string(json_object(jsonNumberDataPoint),
                           "startTimeUnixNano",
                           startTimeUnixNano.c_str());
    // time_unix_nano
    auto timeUnixNano = std::to_string(numberDataPoint.timeUnixNano);
    json_object_set_string(json_object(jsonNumberDataPoint), "timeUnixNano", timeUnixNano.c_str());
    // value
    switch (numberDataPoint.value)
    {
        // as_double
    case NumberDataPoint::DOUBLE: {
        // sstream ensures better representation with scientific notation
        // see https://en.cppreference.com/cpp/string/basic_string/to_string
        std::ostringstream sstream;
        sstream << numberDataPoint.asDouble;
        auto asDouble = sstream.str();
        json_object_set_string(json_object(jsonNumberDataPoint), "asDouble", asDouble.c_str());
    }
    break;
        // as_int
    case NumberDataPoint::INT: {
        // 64 bit ints must be serialized to string
        // see https://protobuf.dev/programming-guides/json/#field-representation
        auto asInt = std::to_string(numberDataPoint.asInt);
        json_object_set_string(json_object(jsonNumberDataPoint), "asInt", asInt.c_str());
    }
    break;
    }
    // exemplars: not impl
    // flags: not impl
    return jsonNumberDataPoint;
}

size_t
OtlpHttpHelper::StringWriteCallback(char* buffer, size_t size, size_t nmemb, void* string)
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
