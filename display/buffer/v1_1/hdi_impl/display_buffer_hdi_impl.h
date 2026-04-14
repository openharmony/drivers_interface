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

#ifndef OHOS_HDI_DISPLAY_V1_1_DISPLAY_BUFFER_HDI_IMPL_H
#define OHOS_HDI_DISPLAY_V1_1_DISPLAY_BUFFER_HDI_IMPL_H

#include <iproxy_broker.h>
#include <unistd.h>
#include <mutex>
#include "hdf_log.h"
#include "hilog/log.h"
#include "v1_0/hdi_impl/display_buffer_hdi_impl.h"
#include "v1_1/imetadata.h"
#include "v1_1/include/idisplay_buffer.h"

#undef LOG_TAG
#define LOG_TAG "DISP_HDI_BUFF"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002515

namespace OHOS {
namespace HDI {
namespace Display {
namespace Buffer {
namespace V1_1 {
template<typename Interface>
class DisplayBufferHdiImpl : public V1_0::DisplayBufferHdiImpl<Interface> {
public:
    explicit DisplayBufferHdiImpl(sptr<V1_0::IAllocator> allocator, sptr<V1_0::IMapper> mapper,
        sptr<IMetadata> metadata)
        : BaseType1_0(allocator, mapper), metadata_(metadata)
    {}
    virtual ~DisplayBufferHdiImpl() {};

    void CheckMetadata() const
    {
        if (metadata_ != nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        if (metadata_ == nullptr) {
            metadata_ = IMetadata::Get(true);
        }
    }

    int32_t RegisterBuffer(const BufferHandle& handle) override
    {
        CheckMetadata();
        CHECK_NULLPOINTER_RETURN_VALUE(metadata_, HDF_FAILURE);
        sptr<NativeBuffer> hdiBuffer = new NativeBuffer();
        CHECK_NULLPOINTER_RETURN_VALUE(hdiBuffer, HDF_FAILURE);
        hdiBuffer->SetBufferHandle(const_cast<BufferHandle*>(&handle));
        int32_t ret = metadata_->RegisterBuffer(hdiBuffer);
        return ret;
    }

    int32_t SetMetadata(const BufferHandle& handle, uint32_t key, const std::vector<uint8_t>& value) override
    {
        CheckMetadata();
        CHECK_NULLPOINTER_RETURN_VALUE(metadata_, HDF_FAILURE);
        sptr<NativeBuffer> hdiBuffer = new NativeBuffer();
        CHECK_NULLPOINTER_RETURN_VALUE(hdiBuffer, HDF_FAILURE);
        hdiBuffer->SetBufferHandle(const_cast<BufferHandle*>(&handle));
        int32_t ret = metadata_->SetMetadata(hdiBuffer, key, value);
        return ret;
    }
    
    int32_t GetMetadata(const BufferHandle& handle, uint32_t key, std::vector<uint8_t>& value) override
    {
        CheckMetadata();
        CHECK_NULLPOINTER_RETURN_VALUE(metadata_, HDF_FAILURE);
        sptr<NativeBuffer> hdiBuffer = new NativeBuffer();
        CHECK_NULLPOINTER_RETURN_VALUE(hdiBuffer, HDF_FAILURE);
        hdiBuffer->SetBufferHandle(const_cast<BufferHandle*>(&handle));
        int32_t ret = metadata_->GetMetadata(hdiBuffer, key, value);
        return ret;
    }

    int32_t ListMetadataKeys(const BufferHandle& handle, std::vector<uint32_t>& keys) override
    {
        CheckMetadata();
        CHECK_NULLPOINTER_RETURN_VALUE(metadata_, HDF_FAILURE);
        sptr<NativeBuffer> hdiBuffer = new NativeBuffer();
        CHECK_NULLPOINTER_RETURN_VALUE(hdiBuffer, HDF_FAILURE);
        hdiBuffer->SetBufferHandle(const_cast<BufferHandle*>(&handle));
        int32_t ret = metadata_->ListMetadataKeys(hdiBuffer, keys);
        return ret;
    }
    
    int32_t EraseMetadataKey(const BufferHandle& handle, uint32_t key) override
    {
        CheckMetadata();
        CHECK_NULLPOINTER_RETURN_VALUE(metadata_, HDF_FAILURE);
        sptr<NativeBuffer> hdiBuffer = new NativeBuffer();
        CHECK_NULLPOINTER_RETURN_VALUE(hdiBuffer, HDF_FAILURE);
        hdiBuffer->SetBufferHandle(const_cast<BufferHandle*>(&handle));
        int32_t ret = metadata_->EraseMetadataKey(hdiBuffer, key);
        return ret;
    }
    using V1_0::DisplayBufferHdiImpl<Interface>::WAIT_TIME_INTERVAL;
private:
    using BaseType1_0 = V1_0::DisplayBufferHdiImpl<Interface>;
protected:
    mutable std::mutex mutex_;
    mutable sptr<IMetadata> metadata_;
};
using HdiDisplayBufferImpl = DisplayBufferHdiImpl<V1_1::IDisplayBuffer>;
} // namespace V1_1
} // namespace Buffer
} // namespace Display
} // namespace HDI
} // namespace OHOS

#endif // OHOS_HDI_DISPLAY_V1_1_DISPLAY_BUFFER_HDI_IMPL_H
