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

#include "device_cache.h"

#include "buffer_cache_utils.h"
#include "common/include/display_interface_utils.h"
#include "hdf_base.h"
#include "hdf_log.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {

DeviceCache* DeviceCache::Create(uint32_t id, DeviceType type)
{
    DeviceCache* device = new DeviceCache(id, type);
    DISPLAY_CHK_RETURN(device == nullptr, nullptr, HDF_LOGE("%{public}s: create device cache failed", __func__));

    int32_t ret = device->Init();
    if (ret != HDF_SUCCESS) {
        delete device;
        device = nullptr;
        HDF_LOGE("%{public}s: device cache init failed", __func__);
    }

    return device;
}

DeviceCache::DeviceCache(uint32_t id, DeviceType type) : deviceId_(id), cacheType_(type)
{
}

DeviceCache::~DeviceCache()
{
}

int32_t DeviceCache::Init()
{
    layerCaches_.reset(new CacheManager<uint32_t, LayerCache>());
    DISPLAY_CHK_RETURN(layerCaches_ == nullptr, HDF_FAILURE,
        HDF_LOGE("%{public}s: create layer caches failed", __func__));

    clientBufferCaches_.reset(new CacheManager<uint32_t, NativeBuffer>());
    DISPLAY_CHK_RETURN(clientBufferCaches_ == nullptr, HDF_FAILURE,
        HDF_LOGE("%{public}s: create client buffer caches failed", __func__));

    clientBufferCaches_->SetInitFunc(LayerCache::NativeBufferInit);
    clientBufferCaches_->SetCleanUpFunc(LayerCache::NativeBufferCleanUp);

    outputBufferCaches_.reset(new CacheManager<uint32_t, NativeBuffer>());
    DISPLAY_CHK_RETURN(outputBufferCaches_ == nullptr, HDF_FAILURE,
        HDF_LOGE("%{public}s: create output buffer caches failed", __func__));

    outputBufferCaches_->SetInitFunc(LayerCache::NativeBufferInit);
    outputBufferCaches_->SetCleanUpFunc(LayerCache::NativeBufferCleanUp);
    return HDF_SUCCESS;
}

int32_t DeviceCache::SetClientBufferCacheCount(uint32_t bufferCacheCount)
{
    return clientBufferCaches_->SetCacheMaxCount(bufferCacheCount) ? HDF_SUCCESS : HDF_FAILURE;
}

LayerCache* DeviceCache::LayerCacheInstance(uint32_t layerId) const
{
    return layerCaches_->SearchCache(layerId);
}

int32_t DeviceCache::AddLayerCache(uint32_t id, uint32_t bufferCacheCount)
{
    LayerCache* layer = LayerCache::Create(id);
    DISPLAY_CHK_RETURN(layer == nullptr, HDF_FAILURE, HDF_LOGE("%{public}s: Create cache failed", __func__));

    int32_t retResult = layer->SetBufferCacheMaxCount(bufferCacheCount);
    DISPLAY_CHK_RETURN(retResult != HDF_SUCCESS, retResult,
        HDF_LOGE("%{public}s: set buffer cache max count failed", __func__));

    bool ret = layerCaches_->InsertCache(id, layer);
    DISPLAY_CHK_RETURN(ret != true, HDF_FAILURE, HDF_LOGE("%{public}s: insert cache failed", __func__));
    return HDF_SUCCESS;
}

int32_t DeviceCache::RemoveLayerCache(uint32_t id)
{
    bool ret = layerCaches_->EraseCache(id);
    DISPLAY_CHK_RETURN(ret != true, HDF_FAILURE, HDF_LOGE("%{public}s: Destroy cache failed", __func__));

    return HDF_SUCCESS;
}

int32_t DeviceCache::SetDisplayClientBuffer(const BufferHandle* buffer, uint32_t seqNo, bool &needFreeBuffer,
    std::function<int32_t (const BufferHandle&)> realFunc)
{
    BufferHandle* handle = BufferCacheUtils::NativeBufferCache(clientBufferCaches_,
        const_cast<BufferHandle*>(buffer), seqNo, deviceId_, needFreeBuffer);
    DISPLAY_CHK_RETURN(handle == nullptr, HDF_FAILURE,
        HDF_LOGE("%{public}s: call NativeBufferCache fail", __func__));
    auto ret = realFunc(*handle);
    DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret, HDF_LOGE("%{public}s: call realFunc fail", __func__));

    return ret;
}

int32_t DeviceCache::SetVirtualDisplayBuffer(const BufferHandle* buffer, uint32_t seqNo, bool &needFreeBuffer,
    std::function<int32_t (const BufferHandle&)> realFunc)
{
    int32_t ret = HDF_FAILURE;
    if (CacheType() == DEVICE_TYPE_VIRTUAL) {
        BufferHandle* handle = BufferCacheUtils::NativeBufferCache(outputBufferCaches_,
            const_cast<BufferHandle*>(buffer), seqNo, deviceId_, needFreeBuffer);
        DISPLAY_CHK_RETURN(handle == nullptr, HDF_FAILURE,
            HDF_LOGE("%{public}s: call NativeBufferCache fail", __func__));
        ret = realFunc(*handle);
        DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret, HDF_LOGE("%{public}s: call realFunc fail", __func__));
    } else {
        HDF_LOGE("%{public}s: not a virtual display", __func__);
    }

    return ret;
}

DeviceCache::DeviceType DeviceCache::CacheType() const
{
    return cacheType_;
}

void DeviceCache::Dump() const
{
    clientBufferCaches_->TravelCaches([this](int32_t id, const NativeBuffer& buffer)->void {
        auto info = buffer.Dump();
        HDF_LOGE("devId-%{public}d, clientBuffer[%{public}d]: %{public}s", deviceId_, id, info.c_str());
    });
    outputBufferCaches_->TravelCaches([this](int32_t id, const NativeBuffer& buffer)->void {
        auto info = buffer.Dump();
        HDF_LOGE("devId-%{public}d, outputBuffer[%{public}d]: %{public}s", deviceId_, id, info.c_str());
    });
    layerCaches_->TravelCaches([](int32_t id, const LayerCache& cache)->void {
        cache.Dump();
    });
}
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
