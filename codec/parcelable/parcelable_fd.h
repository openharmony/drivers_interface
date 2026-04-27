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
#ifndef OHOS_CODEC_HDI_PARCELABLE_FD_H
#define OHOS_CODEC_HDI_PARCELABLE_FD_H

#include <unistd.h>
#include <message_parcel.h>  // foundation/communication/ipc/interfaces/innerkits/ipc_core/include/

namespace OHOS::HDI::Codec {
class ParcelableFd : public Parcelable {
public:
    static sptr<ParcelableFd> Create(int fd, bool transferOwnership)
    {
        int finalFd = transferOwnership ? fd : dup(fd);
        if (finalFd < 0) {
            return nullptr;
        }
        return new ParcelableFd(finalFd);
    }

    ParcelableFd(const ParcelableFd&) = delete;
    ParcelableFd& operator=(const ParcelableFd&) = delete;

    ~ParcelableFd() override
    {
        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
    }

    int GetFd() const
    {
        return fd_;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        MessageParcel &msgParcel = static_cast<MessageParcel &>(parcel);
        return msgParcel.WriteFileDescriptor(fd_);  // dup occurred in this function
    }

    static sptr<ParcelableFd> Unmarshalling(Parcel &parcel)
    {
        MessageParcel &msgParcel = static_cast<MessageParcel &>(parcel);
        int fd = msgParcel.ReadFileDescriptor();  // this returned fd has already been duped
        return ParcelableFd::Create(fd, true);
    }

protected:
    explicit ParcelableFd(int32_t fd) : fd_(fd) {}
    int fd_ = -1;
};
}
#endif
