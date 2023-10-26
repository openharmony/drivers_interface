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

#ifndef OHOS_HDI_BUFFER_HANDLE_H
#define OHOS_HDI_BUFFER_HANDLE_H

#include <parcel.h>
#include <string>
#include <vector>
#include "surface.h"

namespace OHOS {
namespace HDI {
namespace Camera {
namespace V1_0 {
using OHOS::Parcelable;
using OHOS::Parcel;
using OHOS::sptr;

class BufferHandleSequenceable : public Parcelable {
public:
    BufferHandleSequenceable() = default;
    virtual ~BufferHandleSequenceable() = default;

    explicit BufferHandleSequenceable(const std::shared_ptr<BufferHandle> &bufferHandle):bufferHandle_(bufferHandle) {}
    explicit BufferHandleSequenceable(const BufferHandle &bufferHandle);
    BufferHandleSequenceable(const BufferHandleSequenceable &other) {}
    BufferHandleSequenceable &operator=(const BufferHandleSequenceable &other) = delete;

    bool Marshalling(Parcel &parcel) const override;
    static sptr<BufferHandleSequenceable> Unmarshalling(Parcel &parcel);

    std::shared_ptr<BufferHandle> bufferHandle_ = nullptr;
};
} // V1_0
} // Camera
} // HDI
} // OHOS
#endif // OHOS_HDI_SEQUENCE_DATA_H
