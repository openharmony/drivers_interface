/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <cstdint>
#include <parameters.h>
#include "hilog/log.h"
#include "parse_persist_int32.h"

#ifndef LOG_TAG
#define LOG_TAG "DISP_HDI_CONFIG"
#endif

namespace OHOS {
namespace HDI {
namespace Display {
static bool GetEnableParallel()
{
    static bool enableParallel = []() {
        constexpr int32_t defaultValue = 1;
        const std::string text = system::GetParameter("persist.debug.composer.hdi.enableparallel", "1");
        int32_t value = defaultValue;
        if (!ParsePersistInt32(text, value)) {
            HDF_LOGE("%{public}s: invalid persist.debug.composer.hdi.enableparallel: %{public}s",
                __func__, text.c_str());
            return defaultValue != 0;
        }
        return value != 0;
    }();
    return enableParallel;
}
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif /* DISPLAY_CONFIG_H */
