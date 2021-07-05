#pragma once

#include <vector>
#include <memory>
#include <cstdint>

namespace vc
{
struct raw_frame
{
    explicit raw_frame() {}
    ~raw_frame() = default;
    
    std::vector<uint8_t> data;
	double pts = 0.0;
};

}