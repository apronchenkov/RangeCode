#pragma once

#include "adaptive_model.h"

#include <cstdint>
#include <vector>


class QualityModel {
public:
    explicit QualityModel(size_t alphabet_size)
      : default_(alphabet_size)
      , models_(1, default_)
      , model_ptr_(&models_[0])
      , position_(0)
    { }

    void Update(unsigned char symbol)
    {
        model_ptr_->Update<14>(symbol);

        if (symbol) {
            const size_t index = (++position_ << 8) | symbol;
            if (index >= models_.size()) {
                models_.resize(index + 1);
            }
            model_ptr_ = &models_[index];
            if (model_ptr_->cdf().empty()) {
                *model_ptr_ = default_;
            }

        } else {
            model_ptr_ = &models_[0];
            position_ = 0;
        }
    }

    const std::vector<uint32_t>& cdf() const { return model_ptr_->cdf(); }

private:
    AdaptiveModel default_;
    std::vector<AdaptiveModel> models_;
    AdaptiveModel* model_ptr_;
    size_t position_;

    QualityModel(const QualityModel&) = delete;
    QualityModel& operator= (const QualityModel&) = delete;
};
