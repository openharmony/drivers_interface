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

    bufferCaches_->SetInitFunc(NativeBufferInit);
    bufferCaches_->SetCleanUpFunc(NativeBufferCleanUp);
    return HDF_SUCCESS;
}

int32_t LayerCache::SetBufferCacheMaxCount(uint32_t cacheCount)
{
    bool ret = bufferCaches_->SetCacheMaxCount(cacheCount);
    DISPLAY_CHK_RETURN(ret == false, HDF_FAILURE, HDF_LOGE("%{public}s: failed", __func__));
    return HDF_SUCCESS;
}

int32_t LayerCache::SetLayerBuffer(BufferHandle*& buffer, uint32_t seqNo, bool &needFreeBuffer,
    const std::vector<uint32_t>& deletingList, std::function<int32_t (const BufferHandle&)> realFunc)
{
    for (auto num : deletingList) {
        (void)bufferCaches_->EraseCache(num);
    }

    BufferHandle* handle = BufferCacheUtils::NativeBufferCache(bufferCaches_, buffer, seqNo, layerId_, needFreeBuffer);
    DISPLAY_CHK_RETURN(handle == nullptr, HDF_FAILURE,
        HDF_LOGE("%{public}s: call NativeBufferCache fail", __func__));
    int32_t ret = realFunc(*handle);
    if (ret != HDF_SUCCESS) {
        bufferCaches_->EraseCache(seqNo);
        buffer = nullptr;
        HDF_LOGE("%{public}s: call realFunc fail", __func__);
    }

    return HDF_SUCCESS;
}

int32_t LayerCache::ResetLayerBuffer()
{
    HDF_LOGI("%{public}s", __func__);
    return Init();
}

void LayerCache::NativeBufferInit(sptr<NativeBuffer>& buffer)
{
    if (buffer == nullptr) {
        HDF_LOGW("NativeBufferInit buffer nullptr!");
        return;
    }
    int32_t ret = RegisterBuffer(buffer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: RegisterBuffer failed with %{public}d!", __func__, ret);
    }
}

void LayerCache::NativeBufferCleanUp(sptr<NativeBuffer>& buffer)
{
    if (buffer == nullptr) {
        HDF_LOGW("NativeBufferCleanUp buffer nullptr!");
        return;
    }
    int32_t ret = FreeMem(buffer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%{public}s: FreeMem failed with %{public}d!", __func__, ret);
    }
}

sptr<Buffer::V1_1::IMetadata> LayerCache::GetMetaService()
{
    static sptr<Buffer::V1_1::IMetadata> metaService = nullptr;
    if (metaService == nullptr) {
        metaService = Buffer::V1_1::IMetadata::Get(true);
    }
    return metaService;
}

sptr<Buffer::V1_2::IMapper> LayerCache::GetMapperService()
{
    static sptr<Buffer::V1_2::IMapper> mapperService = nullptr;
    if (mapperService == nullptr) {
        mapperService = Buffer::V1_2::IMapper::Get(true);
    }
    return mapperService;
}

int32_t LayerCache::Mmap(sptr<NativeBuffer>& buffer)
{
    auto mapperService = GetMapperService();
    if (mapperService == nullptr) {
        HDF_LOGE("GetMapperService failed!");
        return HDF_FAILURE;
    }
    return mapperService->Mmap(buffer);
}

int32_t LayerCache::Unmap(sptr<NativeBuffer>& buffer)
{
    auto mapperService = GetMapperService();
    if (mapperService == nullptr) {
        HDF_LOGE("GetMapperService failed!");
        return HDF_FAILURE;
    }
    return mapperService->Unmap(buffer);
}

int32_t LayerCache::FreeMem(sptr<NativeBuffer>& buffer)
{
    auto mapperService = GetMapperService();
    if (mapperService == nullptr) {
        HDF_LOGE("GetMapperService failed!");
        return HDF_FAILURE;
    }
	int32_t ret = Unmap(buffer);
	if( ret != HDF_SUCCESS) {
        HDF_LOGE("Unmap failed!");		
	}
    return mapperService->FreeMem(buffer);
}

int32_t LayerCache::RegisterBuffer(sptr<NativeBuffer>& buffer)
{
    auto metaService = GetMetaService();
    if (metaService == nullptr) {
        return HDF_FAILURE;
    }
    int32_t ret = metaService->RegisterBuffer(buffer);
	if( ret != HDF_SUCCESS) {
        HDF_LOGE("Register Buffer failed!");
        return ret;
	}
    ret = Mmap(buffer);
	if( ret != HDF_SUCCESS) {
        HDF_LOGE("Mmap failed!");		
	}
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
