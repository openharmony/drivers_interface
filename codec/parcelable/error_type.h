/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef VIDEO_ERROR_TYPE
#define VIDEO_ERROR_TYPE

namespace OHOS::HDI::Codec {

enum ErrorType {
    ERROR_BEGIN = -0xFFFF,
    INVALID_VAL,
    INVALID_STATE,
    TIME_OUT,
    NO_MEM,
    ALL_FAIL,      // for vector
    PARTIAL_SUCC,  // for vector
    POOL_IS_FULL,
    UNSUPPORT,
    UNKNOWN,
    OK = 0,
};

}
#endif