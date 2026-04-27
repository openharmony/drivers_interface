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
#ifndef OHOS_CODEC_HDI_PARCELABLE_BUFFER
#define OHOS_CODEC_HDI_PARCELABLE_BUFFER

#include <sys/mman.h>
#include <base/native_buffer.h>
#include "parcelable_fd.h"

namespace OHOS::HDI::Codec {
class ParcelableBuffer : public ParcelableFd {
public:
    static sptr<ParcelableBuffer> CreateImageBuffer(const sptr<Base::NativeBuffer>& image)
    {
        if (image == nullptr) {
            return nullptr;
        }
        BufferHandle *handle = image->GetBufferHandle();
        if (handle == nullptr || handle->fd < 0 || handle->size <= 0) {
            return nullptr;
        }
        return new ParcelableBuffer(handle->fd, static_cast<uint32_t>(handle->size), image);
    }

    static sptr<ParcelableBuffer> CreateStreamBuffer(int fd, uint32_t capacity, bool transferOwnership)
    {
        if (capacity == 0) {
            return nullptr;
        }
        int finalFd = transferOwnership ? fd : dup(fd);
        if (finalFd < 0) {
            return nullptr;
        }
        return new ParcelableBuffer(finalFd, capacity, nullptr);
    }

    ParcelableBuffer(const ParcelableBuffer&) = delete;
    ParcelableBuffer& operator=(const ParcelableBuffer&) = delete;

    ~ParcelableBuffer() override
    {
        if (va_ != nullptr) {
            munmap(va_, capacity_);
            va_ = nullptr;
        }
        if (image_) {
            fd_ = -1;  // fd will be closed by NativeBuffer
        }
    }

    bool IsImage() const
    {
        return (image_ != nullptr);
    }

    uint32_t GetCapacity() const
    {
        return capacity_;
    }

    void* GetVirAddr()
    {
        if (va_ != nullptr) {
            return va_;
        }
        va_ = mmap(nullptr, capacity_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (va_ == MAP_FAILED) {
            va_ = nullptr;
            return nullptr;
        }
        return va_;
    }

    void* GetHandle()
    {
        if (image_) {
            return static_cast<void*>(image_->GetBufferHandle());
        }
        return nullptr;
    }

    BufferHandle* Clone()
    {
        if (image_) {
            return image_->Clone();
        }
        return nullptr;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        MessageParcel &msgParcel = static_cast<MessageParcel &>(parcel);
        if (image_) {
            if (!msgParcel.WriteBool(true)) {
                return false;
            }
            return image_->Marshalling(parcel);
        }
        if (!msgParcel.WriteBool(false)) {
            return false;
        }
        if (!msgParcel.WriteUint32(capacity_)) {
            return false;
        }
        return msgParcel.WriteFileDescriptor(fd_);  // dup occurred in this function
    }

    static sptr<ParcelableBuffer> Unmarshalling(Parcel &parcel)
    {
        MessageParcel &msgParcel = static_cast<MessageParcel &>(parcel);
        bool isImage = msgParcel.ReadBool();
        if (isImage) {
            sptr<Base::NativeBuffer> image = Base::NativeBuffer::Unmarshalling(parcel);
            return ParcelableBuffer::CreateImageBuffer(image);
        }
        uint32_t capacity = msgParcel.ReadUint32();
        int fd = msgParcel.ReadFileDescriptor();  // this returned fd has already been duped
        return ParcelableBuffer::CreateStreamBuffer(fd, capacity, true);
    }

private:
    ParcelableBuffer(int fd, uint32_t capacity, const sptr<Base::NativeBuffer>& image)
        : ParcelableFd(fd), capacity_(capacity), image_(image) {}

    uint32_t capacity_ = 0;
    sptr<Base::NativeBuffer> image_;
    void* va_ = nullptr;
};
}

#endif