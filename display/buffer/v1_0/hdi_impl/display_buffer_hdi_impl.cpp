/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "display_buffer_hdi_impl.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Buffer {
namespace V1_0 {
constexpr int32_t MAX_WAIT_COUNT = 500;
IDisplayBuffer *IDisplayBuffer::Get()
{
    HdfTrace trace("DisplayBufferHdiImplV1_0", "HDI:DISP:IMPL:");
    sptr<IAllocator> allocator;
    int32_t count = MAX_WAIT_COUNT;
    while ((allocator = IAllocator::Get(false)) == nullptr) {
        // Waiting for allocator service ready
        usleep(HdiDisplayBufferImpl::WAIT_TIME_INTERVAL);
        if (--count < 0) {
            HDF_LOGE("IAllocator::Get over 5s");
            break;
        }
    }
    sptr<IMapper> mapper = IMapper::Get(true);

    IDisplayBuffer *instance = new (std::nothrow) HdiDisplayBufferImpl(allocator, mapper);
    if (instance == nullptr) {
        return nullptr;
    }
    return instance;
}

} // namespace V1_0
} // namespace Buffer
} // namespace Display
} // namespace HDI
} // namespace OHOS
