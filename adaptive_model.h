#pragma once

#include "coder.h"

#include <cstdint>
#include <vector>


class AdaptiveModel {
public:
    AdaptiveModel();

    explicit AdaptiveModel(size_t symbol_count);

    void Reset(size_t symbol_count);

    const std::vector<uint32_t>& cdf() const { return cdf_; }

    void Update(size_t symbol);

private:
    std::vector<uint32_t> symbols_count_;
    std::vector<uint32_t> cdf_;
    size_t total_counter_;
    size_t update_downcounter_;
};

inline AdaptiveModel::AdaptiveModel()
  : total_counter_(0)
  , update_downcounter_(0)
{ }

inline AdaptiveModel::AdaptiveModel(size_t symbol_count)
{
    Reset(symbol_count);
}

inline void AdaptiveModel::Reset(size_t symbol_count)
{
    symbols_count_.assign(symbol_count, 1);
    cdf_.reserve(symbol_count + 1);
    total_counter_ = symbol_count;
    update_downcounter_ = 32;
    range_code::GenerateCdf(symbols_count_, &cdf_);
}

inline void AdaptiveModel::Update(size_t symbol)
{
    symbols_count_.at(symbol) += 16;
    total_counter_ += 16;

    if (total_counter_ >= (1<<24)) {
        total_counter_ = 0;
        for (auto& symbol_count : symbols_count_) {
            symbol_count = (symbol_count + 1) / 2;
            total_counter_ += symbol_count;
        }
    }

    --update_downcounter_;
    if (update_downcounter_ == 0) {
        update_downcounter_ = 64;
        range_code::GenerateCdf(symbols_count_, &cdf_);
    }
}
