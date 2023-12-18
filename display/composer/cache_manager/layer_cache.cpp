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

#include "layer_cache.h"

#include "buffer_cache_utils.h"
#include "common/include/display_interface_utils.h"
#include "hdf_base.h"
#include "hdf_log.h"

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {

LayerCache* LayerCache::Create(uint32_t id)
{
    LayerCache* layer = new LayerCache(id);
    DISPLAY_CHK_RETURN(layer == nullptr, nullptr, HDF_LOGE("%{public}s: create layer cache failed", __func__));

    int32_t ret = layer->Init();
    if (ret != HDF_SUCCESS) {
        delete layer;
        layer = nullptr;
        HDF_LOGE("%{public}s: layer cache init failed", __func__);
    }

    return layer;
}

LayerCache::LayerCache(uint32_t id) : layerId_(id)
{
}

LayerCache::~LayerCache()
{
}

int32_t LayerCache::Init()
{
    bufferCaches_.reset(new CacheManager<uint32_t, NativeBuffer>());
    DISPLAY_CHK_RETURN(bufferCaches_ == nullptr, HDF_FAILURE,
        HDF_LOGE("%{public}s: create buffer caches failed", __func__));

    return HDF_SUCCESS;
}

int32_t LayerCache::SetBufferCacheMaxCount(uint32_t cacheCount)
{
    bool ret = bufferCaches_->SetCacheMaxCount(cacheCount);
    DISPLAY_CHK_RETURN(ret == false, HDF_FAILURE, HDF_LOGE("%{public}s: failed", __func__));
    return HDF_SUCCESS;
}

int32_t LayerCache::SetLayerBuffer(const BufferHandle* buffer, uint32_t seqNo, bool &needFreeBuffer,
    const std::vector<uint32_t>& deletingList, std::function<int32_t (const BufferHandle&)> realFunc)
{
    for (auto num : deletingList) {
        bool result = bufferCaches_->EraseCache(num);
        DISPLAY_CHECK(!result, HDF_LOGE("%{public}s: warning, erase buffer cache fail, num: %{public}u",
            __func__, num));
    }

    BufferHandle* handle = BufferCacheUtils::NativeBufferCache(bufferCaches_,
        const_cast<BufferHandle*>(buffer), seqNo, layerId_, needFreeBuffer);
    DISPLAY_CHK_RETURN(handle == nullptr, HDF_FAILURE,
        HDF_LOGE("%{public}s: call NativeBufferCache fail", __func__));
    int32_t ret = realFunc(*handle);
    DISPLAY_CHK_RETURN(ret != HDF_SUCCESS, ret, HDF_LOGE("%{public}s: call realFunc fail", __func__));

    return HDF_SUCCESS;
}

void LayerCache::Dump() const
{
    bufferCaches_->TravelCaches([this](const int32_t id, const NativeBuffer& buffer)->void {
        auto info = buffer.Dump();
        HDF_LOGE("layerId-%{public}d, buffer[%{public}d]: %{public}s", layerId_, id, info.c_str());
    });
}
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
