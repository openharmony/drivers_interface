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
#ifndef OHOS_CODEC_HDI_PARCELABLE_PARAM_H
#define OHOS_CODEC_HDI_PARCELABLE_PARAM_H

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <shared_mutex>
#include <unordered_map>
#include <type_traits>
#include "parcel.h"
#include <iostream>

#define HDI_IF_FALSE_RETURN(cond) \
    do {                      \
        if (!(cond)) {          \
            return false;     \
        }                     \
    } while (0)

namespace OHOS::HDI::Codec {
class ParcelableParam : public Parcelable {
public:
    using ValueType =
        std::variant<bool, int8_t, int32_t, int64_t, uint8_t, uint32_t, uint64_t,
                     double, std::string, std::vector<uint8_t>>;

    enum TypeIndex : uint8_t {
        BOOL,
        I8,
        I32,
        I64,
        U8,
        U32,
        U64,
        DOUBLE,
        STRING,
        VECTOR,
    };

    static sptr<ParcelableParam> Create()
    {
        return new ParcelableParam();
    }

    template <typename T>
    void Set(const std::string &key, const T &value)
    {
        std::lock_guard<std::shared_mutex> lk(mtx_);
        if constexpr (std::is_enum_v<T>) {
            items_[key] = static_cast<std::underlying_type_t<T>>(value);
        } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string> ||
                      std::is_same_v<T, std::vector<uint8_t>>) {
            items_[key] = value;
        } else if constexpr (std::is_trivially_copyable_v<T>) {
            auto *begin = reinterpret_cast<const uint8_t *>(&value);
            items_[key] = std::vector<uint8_t>(begin, begin + sizeof(T));
        }
    }

    template <typename T>
    bool Get(const std::string &key, T &value) const
    {
        std::shared_lock<std::shared_mutex> lk(mtx_);
        auto iter = items_.find(key);
        if (iter == items_.end()) {
            return false;
        }
        const ValueType &val = iter->second;
        if constexpr (std::is_enum_v<T>) {
            const auto *found = std::get_if<std::underlying_type_t<T>>(&val);
            if (found == nullptr) {
                return false;
            }
            value = static_cast<T>(*found);
            return true;
        } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string> ||
                      std::is_same_v<T, std::vector<uint8_t>>) {
            const T *found = std::get_if<T>(&val);
            if (found == nullptr) {
                return false;
            }
            value = *found;
            return true;
        } else if constexpr (std::is_trivially_copyable_v<T>) {
            const std::vector<uint8_t> *found = std::get_if<std::vector<uint8_t>>(&val);
            if (found == nullptr) {
                return false;
            }
            if (found->size() != sizeof(T)) {
                return false;
            }
            std::copy(found->begin(), found->end(), reinterpret_cast<uint8_t *>(&value));
            return true;
        }
        return false;
    }

    size_t GetKeyCount() const
    {
        std::shared_lock<std::shared_mutex> lk(mtx_);
        return items_.size();
    }

    class Iterator : public std::iterator<std::forward_iterator_tag, std::pair<const std::string, ValueType>> {
    public:
        explicit Iterator(std::unordered_map<std::string, ValueType>::iterator iter) : iter_(iter) {}
        Iterator &operator++()
        {
            iter_++;
            return *this;
        }
        bool operator==(Iterator other) const
        {
            return iter_ == other.iter_;
        }
        bool operator!=(Iterator other) const
        {
            return iter_ != other.iter_;
        }
        reference operator*() const
        {
            return *iter_;
        }

    private:
        std::unordered_map<std::string, ValueType>::iterator iter_;
    };

    Iterator begin()
    {
        return Iterator(items_.begin());
    }
    Iterator end()
    {
        return Iterator(items_.end());
    }

    bool Marshalling(Parcel &parcel) const override
    {
        bool ret = parcel.WriteInt32(MAGIC);
        HDI_IF_FALSE_RETURN(ret);
        std::shared_lock<std::shared_mutex> lk(mtx_);
        ret = parcel.WriteUint32(items_.size());
        HDI_IF_FALSE_RETURN(ret);
        for (const auto &[k, v] : items_) {
            ret = parcel.WriteString(k);
            HDI_IF_FALSE_RETURN(ret);
            auto typeIdx = static_cast<TypeIndex>(v.index());
            ret = parcel.WriteUint8(typeIdx);
            HDI_IF_FALSE_RETURN(ret);
            switch (typeIdx) {
                case BOOL: {
                    ret = parcel.WriteBool(std::get<bool>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case I8: {
                    ret = parcel.WriteInt8(std::get<int8_t>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case I32: {
                    ret = parcel.WriteInt32(std::get<int32_t>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case I64: {
                    ret = parcel.WriteInt64(std::get<int64_t>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case U8: {
                    ret = parcel.WriteUint8(std::get<uint8_t>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case U32: {
                    ret = parcel.WriteUint32(std::get<uint32_t>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case U64: {
                    ret = parcel.WriteUint64(std::get<uint64_t>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case DOUBLE: {
                    ret = parcel.WriteDouble(std::get<double>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case STRING: {
                    ret = parcel.WriteString(std::get<std::string>(v));
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                case VECTOR: {
                    auto &vec = std::get<std::vector<uint8_t>>(v);
                    ret = parcel.WriteUInt8Vector(vec);
                    HDI_IF_FALSE_RETURN(ret);
                    break;
                }
                default: {
                    return false;
                }
            }
        }
        return true;
    }

    static sptr<ParcelableParam> Unmarshalling(Parcel &parcel)
    {
        int32_t magic;
        if (!parcel.ReadInt32(magic) || magic != MAGIC) {
            return nullptr;
        }
        uint32_t size;
        if (!parcel.ReadUint32(size) || size > MAX_KEY_COUNT) {
            return nullptr;
        }
        sptr<ParcelableParam> bundle = ParcelableParam::Create();
        for (uint32_t i = 0; i < size; i++) {
            std::string key;
            if (!parcel.ReadString(key)) {
                return nullptr;
            }
            uint8_t typeIdxValue;
            if (!parcel.ReadUint8(typeIdxValue)) {
                return nullptr;
            }
            auto typeIdx = static_cast<TypeIndex>(typeIdxValue);
            switch (typeIdx) {
                case BOOL: {
                    bool value;
                    if (!parcel.ReadBool(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case I8: {
                    int8_t value;
                    if (!parcel.ReadInt8(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case I32: {
                    int32_t value;
                    if (!parcel.ReadInt32(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case I64: {
                    int64_t value;
                    if (!parcel.ReadInt64(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case U8: {
                    uint8_t value;
                    if (!parcel.ReadUint8(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case U32: {
                    uint32_t value;
                    if (!parcel.ReadUint32(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case U64: {
                    uint64_t value;
                    if (!parcel.ReadUint64(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case DOUBLE: {
                    double value;
                    if (!parcel.ReadDouble(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case STRING: {
                    std::string value;
                    if (!parcel.ReadString(value)) {
                        return nullptr;
                    }
                    bundle->Set(key, value);
                    break;
                }
                case VECTOR: {
                    std::vector<uint8_t> buf;
                    if (!parcel.ReadUInt8Vector(&buf)) {
                        return nullptr;
                    }
                    bundle->Set(key, buf);
                    break;
                }
                default: {
                    return nullptr;
                }
            }
        }
        return bundle;
    }

private:
    ParcelableParam() = default;
    virtual ~ParcelableParam() = default;
    mutable std::shared_mutex mtx_;
    std::unordered_map<std::string, ValueType> items_;
    static constexpr int32_t MAGIC = 0x3826959;
    static constexpr uint32_t MAX_KEY_COUNT = 1000;
};
}  // OHOS::HDI::Codec
#endif