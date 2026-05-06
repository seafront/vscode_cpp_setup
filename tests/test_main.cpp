#include <gtest/gtest.h>
#include "src/nvme_device.h"

TEST(NvmeDeviceTest, CanConstruct) {
    NvmeDevice dev;
    SUCCEED();
}
