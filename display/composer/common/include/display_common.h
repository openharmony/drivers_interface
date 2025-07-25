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

#ifndef DISPLAY_COMMON_H
#define DISPLAY_COMMON_H

#include <functional>

namespace OHOS {
namespace HDI {
namespace Display {

typedef void (*HotPlugCallback)(uint32_t devId, bool connected, void* data);
typedef void (*VBlankCallback)(unsigned int sequence, uint64_t ns, void* data);
typedef void (*RefreshCallback)(uint32_t devId, void* data);
typedef void (*ModeCallback)(uint32_t modeId, uint64_t vBlankPeriod, void* data);
typedef void (*SeamlessChangeCallback)(uint32_t devId, void* data);
typedef void (*VBlankIdleCallback)(uint32_t devId, uint64_t ns, void* data);
typedef void (*HwcEventCallback)(uint32_t devId, uint32_t eventId, const std::vector<int32_t>& eventData, void* data);
} // namespace Display
} // namespace HDI
} // namespace OHOS
#endif /* DISPLAY_COMMON_H */
