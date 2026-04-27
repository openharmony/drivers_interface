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
#ifndef CODEC_KEYVALUE
#define CODEC_KEYVALUE
#include <type_traits>

namespace OHOS::HDI::Codec {

// keys for decoder / encoder
inline constexpr char KEY_RESOLUTION[] = "resolution";  // struct Resolution
inline constexpr char KEY_FRAME_RATE[] = "frameRate";  // uint32_t
inline constexpr char KEY_HIGH_PERF_FLAG[] = "highPerfFlag";  // bool
inline constexpr char KEY_CALLER_NAME[] = "callerName";  // string
inline constexpr char KEY_UV_ORDER[] = "uvOrder";  // bool

// keys for decoder
inline constexpr char KEY_INPUT_STREAM_ERROR[] = "inputStreamError";  // int32_t
inline constexpr char KEY_CONSUMER_USAGE[] = "consumerUsage";  // uint64_t

// keys for encoder
inline constexpr char KEY_PROFILE[] = "profile";  // enum Profile
inline constexpr char KEY_I_FRAME_INTERVAL[] = "iFrameInterval";  // int32_t
inline constexpr char KEY_REQUEST_IDR[] = "requestIDR";  // bool
inline constexpr char KEY_BITRATE_CONTROL_MODE[] = "bitrateControlMode";  // enum BitrateControlMode
inline constexpr char KEY_TARGET_BITRATE[] = "targetBitrate";  // uint32_t
inline constexpr char KEY_TARGET_QUALITY[] = "targetQuality";  // uint32_t
inline constexpr char KEY_TARGET_QP[] = "targetQp";  // int32_t
inline constexpr char KEY_COLOR_ASPECTS[] = "colorAspects";  // struct ColorAspects
inline constexpr char KEY_OPERATING_RATE[] = "operatingRate";  // uint32_t

// keys for 3DGS
inline constexpr char KEY_SORTING_MODE[] = "sortingMode";  // enum SortingMode
inline constexpr char KEY_NUM_OF_GS[] = "numOfGS";  // uint32_t
inline constexpr char KEY_SH_PRDER[] = "shOrder";  // uint16_t
inline constexpr char KEY_MIN_MAX[] = "minMax";   // vector<float>


enum class SampleFlag : uint32_t {
    EOS = 0x1,
    CSD = 0x2,
    IFRAME = 0x4,
};

enum ComponentType {
    VIDEO_DECODER,
    VIDEO_ENCODER,
    THREEDGS_SORTING,
};

enum Standard {
    AVC,
    HEVC,
    VVC,
    VP9,
    AV1,
    PROVIVID,
};

struct Resolution {
    uint32_t w = 0;
    uint32_t h = 0;
};
static_assert(std::is_trivially_copyable_v<Resolution>);

struct ColorAspects {
    bool range = false;
    uint8_t primaries = 2;
    uint8_t transfer = 2;
    uint8_t matrixCoeffs = 2;
};
static_assert(std::is_trivially_copyable_v<ColorAspects>);

enum AvcProfile {
    AVC_PROFILE_BASELINE,
    AVC_PROFILE_MAIN,
    AVC_PROFILE_HIGH
};

enum HevcProfile {
    HEVC_PROFILE_MAIN,
    HEVC_PROFILE_MAIN_10
};

enum BitrateControlMode {
    VBR,
    CBR,
    CQ,
    SQR,
    CBR_VIDEOCALL,
    CRF,
};

enum SortingMode {
    SLOW,
    MEDIUM,
    FAST,
    VERY_FAST,
};

}
#endif