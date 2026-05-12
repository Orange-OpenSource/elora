/*
 * Copyright (c) 2026 University of Bologna
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@unibo.it>
 */

#ifndef OTLP_HTTP_HELPER_H
#define OTLP_HTTP_HELPER_H

#include <cstdint>
#include <curl/curl.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to send live simulation metrics to an Opentelemetry HTTP Protocol
 * endpoint. Currently, only gauge and sum (counter) metrics are supported. Also, data-point
 * attributes may only be strings, and control over nesting metrics by scope or resource is not
 * exposed, but statically managed inside the class.
 *
 * \warning Requires libcurl-dev installed.
 */
class OtlpHttpHelper
{
    using str = std::string;

  public:
    using KeyValueMap = std::unordered_map<std::string, std::string>;

    struct NumberDataPoint
    {
        enum
        {
            DOUBLE,
            INT
        } value;

        KeyValueMap attributes;
        uint64_t startTimeUnixNano;
        uint64_t timeUnixNano;

        union {
            double asDouble;
            int64_t asInt;
        };

        // exemplars: not impl
        // flags: not impl
    };

    enum AggregationTemporality
    {
        AGGREGATION_TEMPORALITY_UNSPECIFIED,
        AGGREGATION_TEMPORALITY_DELTA,
        AGGREGATION_TEMPORALITY_CUMULATIVE
    };

    using DataPointVec = std::vector<NumberDataPoint>;

    struct Gauge
    {
        DataPointVec& dataPoints;
    };

    struct Sum
    {
        DataPointVec& dataPoints;
        AggregationTemporality aggregationTemporality = AGGREGATION_TEMPORALITY_UNSPECIFIED;
        bool isMonotonic;
    };

    struct Metric

    {
        enum
        {
            GAUGE,
            SUM
        } data;

        std::string name;
        std::string description;
        std::string unit;

        union {
            Gauge gauge;
            Sum sum;
        };

        KeyValueMap metadata;
    };

    using MetricVec = std::vector<Metric>;

    OtlpHttpHelper();

    ~OtlpHttpHelper();

    int PostMetrics(const std::string& url, const MetricVec& metrics);

  private:
    str BuildJson(const MetricVec& input);

    static void* MetricToJson(const Metric& metric);

    static void* NumberDataPointToJson(const NumberDataPoint& numberDataPoint);

    static size_t StringWriteCallback(char* buffer, size_t size, size_t nmemb, void* string);

    char m_hostname[HOST_NAME_MAX + 1];

    struct curl_slist* m_headers;
    CURL* m_handle;
};

} // namespace lorawan
} // namespace ns3

#endif /* OTLP_HTTP_HELPER_H */
