#pragma once 

#include "gtest/gtest.h"

namespace vc::test
{

class vc_fixture : public ::testing::Test
{
protected:
    explicit vc_fixture() { }
    virtual ~vc_fixture() { }
    virtual void SetUp() override { }
    virtual void TearDown() override { }

private:
    template<typename... Args>
    void log(Args&&... args) const
    {
        ((std::cout << std::forward<Args>(args) << ' ') , ...) << std::endl;
    }
};

}
