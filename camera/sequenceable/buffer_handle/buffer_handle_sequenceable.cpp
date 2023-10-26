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

#include "buffer_handle_sequenceable.h"
#include "buffer_handle_parcel.h"
#include <message_parcel.h>
#include "securec.h"
namespace OHOS {
namespace HDI {
namespace Camera {
namespace V1_0 {
BufferHandleSequenceable::BufferHandleSequenceable(const BufferHandle &bufferHandle)
{
    size_t bufferSize = sizeof(BufferHandle) +
        bufferHandle.reserveFds * sizeof(uint32_t) +
        bufferHandle.reserveInts * sizeof(uint32_t);

    std::shared_ptr<BufferHandle> newBufferHandle(static_cast<BufferHandle*>(malloc(bufferSize)));
    if (memcpy_s(newBufferHandle.get(), bufferSize, &bufferHandle, bufferSize) != 0) {
        return;
    }
    bufferHandle_ = newBufferHandle;
}

bool BufferHandleSequenceable::Marshalling(Parcel &parcel) const
{
    if (bufferHandle_ == nullptr) {
        return false;
    }
    OHOS::MessageParcel &dataParcel = static_cast<OHOS::MessageParcel &>(parcel);
    if (WriteBufferHandle(dataParcel, *bufferHandle_) != true) {
        return false;
    }
    return true ;
}

sptr<BufferHandleSequenceable> BufferHandleSequenceable::Unmarshalling(Parcel &parcel)
{
    OHOS::MessageParcel &dataParcel = static_cast<OHOS::MessageParcel &>(parcel);

    sptr<BufferHandleSequenceable> sequenceData;
    std::shared_ptr<BufferHandle> newBufferHandle(ReadBufferHandle(dataParcel));
    if (newBufferHandle != nullptr) {
        sequenceData->bufferHandle_ = newBufferHandle;
    }
    return sequenceData;
}

}
}
}
}