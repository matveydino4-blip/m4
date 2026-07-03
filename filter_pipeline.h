#pragma once

#include <memory>
#include <vector>

#include "filter.h"
#include "image.h"

class FilterPipeline {
public:
    void AddFilter(std::unique_ptr<Filter> filter) {
        filters_.push_back(std::move(filter));
    }

    Image Apply(const Image& image) const {
        Image current = image;
        for (const auto& filter : filters_) {
            current = filter->Apply(current);
        }
        return current;
    }

private:
    std::vector<std::unique_ptr<Filter>> filters_;
};
