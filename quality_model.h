#pragma once

#include "adaptive_model.h"

#include <array>
#include <cstdint>
#include <vector>


class QualityModel {
public:
    explicit QualityModel(size_t alphabet_size)
      : alphabet_size_(alphabet_size)
      , models_(1, AdaptiveModel(alphabet_size))
      , model_(&models_.front())
    {
        c2_ = 0;
        c1_ = 0;
        position_ = 0;
    }

    void Update(uint16_t symbol)
    {
        if (symbol) {
            c2_ = c1_;
            c1_ = symbol;
            ++position_;
        } else {
            c2_ = 0;
            c1_ = 0;
            position_ = 0;
        }
        model_->Update(symbol);

        const size_t index = (position_ * alphabet_size_ + c1_) * alphabet_size_ + c2_;
        if (index >= models_.size()) {
            models_.resize(index + 1);
        }
        model_ = &models_[index];
        if (model_->cdf().empty()) {
            model_->Reset(alphabet_size_);
        }
    }

    const std::vector<uint32_t>& cdf() const { return model_->cdf(); }

private:
    size_t alphabet_size_;

    unsigned char c1_;
    unsigned char c2_;
    size_t position_;

    std::vector<AdaptiveModel> models_;
    AdaptiveModel *model_;
};
