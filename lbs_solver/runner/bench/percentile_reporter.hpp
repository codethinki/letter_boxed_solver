#pragma once
#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <ostream>
#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lbs::bench {

/**
 * summarizes benchmarks named `<family>/<group>/<input>` over their inputs
 * @details prints count, min, p50, p90, p99, max and mean of the per input times of every family and group
 */
class percentile_reporter : public benchmark::BenchmarkReporter {
public:
    bool ReportContext(Context const& context) override {
        PrintBasicContext(&GetErrorStream(), context);
        return true;
    }

    void ReportRuns(std::vector<Run> const& runs) override {
        for(auto const& run : runs) {
            if(run.run_type != Run::RT_Iteration || run.skipped)
                continue;

            auto const name = run.benchmark_name();
            samples_of(name.substr(0, name.rfind('/'))).push_back(run.GetAdjustedRealTime());
        }
    }

    void Finalize() override {
        auto& out = GetOutputStream();
        std::string_view prevGroup{};

        std::println(out, "{:<38} {:>5} {:>9} {:>9} {:>9} {:>9} {:>9} {:>9}", "[us]", "n", "min", "p50", "p90", "p99", "max", "mean");

        for(auto& [name, samples] : _rows) {
            auto const group = std::string_view{name}.substr(name.rfind('/') + 1);
            if(group != prevGroup)
                std::println(out);
            prevGroup = group;

            std::ranges::sort(samples);

            auto const percentile = [&](double p) {
                return samples[static_cast<size_t>(p * static_cast<double>(samples.size() - 1))];
            };
            auto const mean = std::reduce(samples.begin(), samples.end()) / static_cast<double>(samples.size());

            std::println(
                out,
                "{:<38} {:>5} {:>9.1f} {:>9.1f} {:>9.1f} {:>9.1f} {:>9.1f} {:>9.1f}",
                name,
                samples.size(),
                samples.front(),
                percentile(0.5),
                percentile(0.9),
                percentile(0.99),
                samples.back(),
                mean
            );
        }
    }

private:
    std::vector<double>& samples_of(std::string const& name) {
        auto const it = std::ranges::find(_rows, name, &std::pair<std::string, std::vector<double>>::first);
        if(it != _rows.end())
            return it->second;
        return _rows.emplace_back(name, std::vector<double>{}).second;
    }

    std::vector<std::pair<std::string, std::vector<double>>> _rows{};
};

}
