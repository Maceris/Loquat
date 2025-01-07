// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <cstdio>
#include <limits>
#include <string>

#include "main/loquat.h"

namespace loquat
{
    class StatsAccumulator;
    class PixelStatsAccumulator;

    class StatRegisterer
    {
    public:
        using AccumFunc = void (*)(StatsAccumulator&);
        using PixelAccumFunc = void (*)(Point2i p, int counterIndex,
            PixelStatsAccumulator&);
        StatRegisterer(AccumFunc func, PixelAccumFunc = {});

        static void call_callbacks(StatsAccumulator& accum);
        static void call_pixel_callbacks(Point2i p,
            PixelStatsAccumulator& accum);
    };

    void stats_enable_pixel_stats(const AABB2i& b, const std::string& baseName);
    void stats_report_pixel_stats(Point2i p);
    void stats_report_pixel_end(Point2i p);

    void print_stats(FILE* dest);
    void stats_write_pixel_images();
    bool print_check_rare(FILE* dest);
    void clear_stats();
    void report_thread_stats();

    class StatsAccumulator
    {
    public:
        StatsAccumulator();

        void report_counter(const char* name, int64_t val);
        void report_memory_counter(const char* name, int64_t val);
        void report_percentage(const char* name, int64_t num, int64_t denom);
        void report_ratio(const char* name, int64_t num, int64_t denom);
        void report_rare_check(const char* condition, Float maxFrequency,
            int64_t numTrue, int64_t total);

        void report_int_distribution(const char* name, int64_t sum,
            int64_t count, int64_t min, int64_t max);
        void report_float_distribution(const char* name, double sum,
            int64_t count, double min, double max);

        void accumulate_pixel_stats(const PixelStatsAccumulator& accum);
        void write_pixel_images() const;

        void print(FILE* file);
        bool print_check_rare(FILE* dest);
        void clear();

    private:
        struct Stats;
        Stats* stats = nullptr;
    };

    class PixelStatsAccumulator
    {
    public:
        PixelStatsAccumulator();

        void report_pixel_ms(Point2i p, float ms);
        void report_counter(Point2i p, int counterIndex, const char* name, int64_t val);
        void report_ratio(Point2i p, int counterIndex, const char* name, int64_t num,
            int64_t denom);

    private:
        friend class StatsAccumulator;
        struct PixelStats;
        PixelStats* stats = nullptr;
    };

#define STAT_COUNTER(title, var)                                       \
    static thread_local int64_t var;                                   \
    static StatRegisterer STATS_REG##var([](StatsAccumulator &accum) { \
        accum.report_counter(title, var);                               \
        var = 0;                                                       \
    });

#define STAT_PIXEL_COUNTER(title, var)                                        \
    static thread_local int64_t var, var##Sum;                                \
    static StatRegisterer STATS_REG##var(                                     \
        [](StatsAccumulator &accum) {                                         \
            /* report sum, since if disabled, it all just goes into var... */ \
            accum.report_counter(title, var + var##Sum);                       \
            var##Sum = 0;                                                     \
            var = 0;                                                          \
        },                                                                    \
        [](Point2i p, int counterIndex, PixelStatsAccumulator &accum) {       \
            accum.report_counter(p, counterIndex, title, var);                 \
            var##Sum += var;                                                  \
            var = 0;                                                          \
        });

#define STAT_MEMORY_COUNTER(title, var)                                \
    static thread_local int64_t var;                                   \
    static StatRegisterer STATS_REG##var([](StatsAccumulator &accum) { \
        accum.report_memory_counter(title, var);                         \
        var = 0;                                                       \
    });

    struct StatIntDistribution {
        int64_t sum = 0, count = 0;
        int64_t min = std::numeric_limits<int64_t>::max();
        int64_t max = std::numeric_limits<int64_t>::lowest();
        void operator<<(int64_t value) {
            sum += value;
            count += 1;
            min = (value < min) ? value : min;
            max = (value > max) ? value : max;
        }
    };

#define STAT_INT_DISTRIBUTION(title, var)                                         \
    static thread_local StatIntDistribution var;                                  \
    static StatRegisterer STATS_REG##var([](StatsAccumulator &accum) {            \
        accum.report_int_distribution(title, var.sum, var.count, var.min, var.max); \
        var.sum = 0;                                                              \
        var.count = 0;                                                            \
        var.min = int64_t(std::numeric_limits<int64_t>::max());                   \
        var.max = int64_t(std::numeric_limits<int64_t>::lowest());                \
    });

#define STAT_FLOAT_DISTRIBUTION(title, var)                                             \
    static thread_local StatCounter<double> var##sum;                                   \
    static thread_local int64_t var##count;                                             \
    static thread_local StatCounter<double> var##min(                                   \
        std::numeric_limits<double>::max());                                            \
    static thread_local StatCounter<double> var##max(                                   \
        std::numeric_limits<double>::lowest());                                         \
    static StatRegisterer STATS_REG##var([](StatsAccumulator &accum) {                  \
        accum.report_float_distribution(title, var##sum, var##count, var##min, var##max); \
        var##sum = 0;                                                                   \
        var##count = 0;                                                                 \
        var##min = StatCounter<double>(std::numeric_limits<double>::max());             \
        var##max = StatCounter<double>(std::numeric_limits<double>::lowest());          \
    });

#define STAT_PERCENT(title, numVar, denomVar)                             \
    static thread_local int64_t numVar, denomVar;                         \
    static StatRegisterer STATS_REG##numVar([](StatsAccumulator &accum) { \
        accum.report_percentage(title, numVar, denomVar);                  \
        numVar = 0;                                                       \
        denomVar = 0;                                                     \
    });

#define STAT_RATIO(title, numVar, denomVar)                               \
    static thread_local int64_t numVar, denomVar;                         \
    static StatRegisterer STATS_REG##numVar([](StatsAccumulator &accum) { \
        accum.report_ratio(title, numVar, denomVar);                       \
        numVar = 0;                                                       \
        denomVar = 0;                                                     \
    });

#define STAT_PIXEL_RATIO(title, numVar, denomVar)                                     \
    static thread_local int64_t numVar, numVar##Sum, denomVar, denomVar##Sum;         \
    static StatRegisterer STATS_REG##numVar##denomVar(                                \
        [](StatsAccumulator &accum) {                                                 \
            /* report sum, since if disabled, it all just goes into var... */         \
            accum.report_ratio(title, numVar + numVar##Sum, denomVar + denomVar##Sum); \
            numVar = 0;                                                               \
            numVar##Sum = 0;                                                          \
            denomVar = 0;                                                             \
            denomVar##Sum = 0;                                                        \
        },                                                                            \
        [](Point2i p, int counterIndex, PixelStatsAccumulator &accum) {               \
            accum.report_ratio(p, counterIndex, title, numVar, denomVar);              \
            numVar##Sum += numVar;                                                    \
            denomVar##Sum += denomVar;                                                \
            numVar = 0;                                                               \
            denomVar = 0;                                                             \
        });
}
