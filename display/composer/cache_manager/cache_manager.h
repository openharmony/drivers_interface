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

#ifndef OHOS_HDI_DISPLAY_V1_0_CACHE_MANAGER_H
#define OHOS_HDI_DISPLAY_V1_0_CACHE_MANAGER_H

#include <functional>
#include <memory>
#include <unordered_map>
#include "hdf_log.h"
#include "nocopyable.h"

#undef LOG_TAG
#define LOG_TAG "DISP_CACHE_MGR"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002500

namespace OHOS {
namespace HDI {
namespace Display {
namespace Composer {

template <typename IdType, typename CacheType>
class CacheManager : public NoCopyable {
public:
    CacheManager() : cacheCountMax_(0)
    {
    }

    virtual ~CacheManager()
    {
        caches_.clear();
    }

    bool SetCacheMaxCount(uint32_t count)
    {
        bool ret = true;
        uint32_t originalMaxCount = cacheCountMax_;
        if (count >= cacheCountMax_) {
            cacheCountMax_ = count;
        } else if (Size() <= count) {
            cacheCountMax_ = count;
        } else {
            HDF_LOGE("%{public}s error: clientCacheCount can't be set, because cacheCountMax_ > count", __func__);
            ret = false;
        }
        HDF_LOGI("%{public}s: set cache max count from %{public}u to %{public}u",
            __func__, originalMaxCount, cacheCountMax_);
        return ret;
    }

    uint32_t Size() const
    {
        return caches_.size();
    }

    bool InsertCache(IdType id, CacheType* cache)
    {
        if (SearchCache(id) != nullptr) {
            HDF_LOGI("%{public}s: intend to insert a existing cache, SeqNo=%{public}d", __func__, id);
        } else {
            if (cacheCountMax_ != 0 && Size() >= cacheCountMax_) {
                HDF_LOGE("%{public}s: Caches is full, new seqNo:%{public}d can't be inserted", __func__, id);
                return false;
            }
        }
        caches_[id] = std::move(*(new std::unique_ptr<CacheType>(cache)));

        return true;
    }

    bool EraseCache(IdType id)
    {
        bool ret = false;
        if (SearchCache(id) != nullptr) {
            caches_.erase(id);
            ret = true;
        } else {
            HDF_LOGE("%{public}s: Cache %{public}d is not existing\n", __func__, id);
        }

        return ret;
    }

    CacheType* SearchCache(IdType id) const
    {
        auto cacheItem = caches_.find(id);
        if (cacheItem == caches_.end()) {
            return nullptr;
        }

        return cacheItem->second.get();
    }

    void TravelCaches(std::function<void (IdType id, const CacheType& cache)> func) const
    {
        for (auto const& [key, value] : caches_) {
            func(key, *value.get());
        }
    }

private:
    uint32_t cacheCountMax_;
    std::unordered_map<IdType, std::unique_ptr<CacheType>> caches_;
};
} // namespace Composer
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif // OHOS_HDI_DISPLAY_V1_0_CACHE_MANAGER_H