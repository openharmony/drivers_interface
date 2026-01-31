/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef HC_GEN_LOG_H
#define HC_GEN_LOG_H

#include <iostream>
#include <map>
#include <string>

#include "option.h"

namespace OHOS {
namespace Hardware {

class Logger {
public:
    Logger() : level_(INFO) {}

    inline ~Logger()
    {
        if (level_ > INFO) {
            ::std::cout << ERROR_COLOR_END;
        }
        if (level_ <= DEBUG && !Option::Instance().VerboseLog()) {
            return;
        }
        ::std::cout << ::std::endl;
    }

    template <typename T>
    inline Logger &operator<<(const T &v)
    {
        if (level_ <= DEBUG && !Option::Instance().VerboseLog()) {
            return *this;
        }
        ::std::cout << v;
        return *this;
    }

    inline Logger &Debug()
    {
        level_ = DEBUG;
        if (Option::Instance().VerboseLog()) {
            ShowLevel();
        }
        return *this;
    }

    inline Logger &Info()
    {
        level_ = INFO;
        ShowLevel();
        return *this;
    }

    inline Logger &Warning()
    {
        level_ = WARNING;
        ShowLevel();
        return *this;
    }

    inline Logger &Error()
    {
        level_ = ERROR;
        ShowLevel();
        return *this;
    }

    inline Logger &Fatal()
    {
        level_ = FATAL;
        ShowLevel();
        return *this;
    }

private:
    enum LogLevel {
        NONE,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL,
    } level_;

    void ShowLevel() const
    {
        static ::std::map<LogLevel, ::std::string> levelStrMap = {
            {NONE,    ""       },
            {DEBUG,   "Debug"  },
            {INFO,    "Info"   },
            {WARNING, "Warning"},
            {ERROR,   "Error"  },
            {FATAL,   "Fatal"  }
        };
        if (level_ > INFO) {
            ::std::cout << ERROR_COLOR_PREFIX;
        }
        ::std::cout << "[" << levelStrMap[level_] << "] ";
    }

#ifdef OS_LINUX
    static constexpr const char *ERROR_COLOR_PREFIX = "\033[31m";
    static constexpr const char *ERROR_COLOR_END = "\033[0m";
#else
    static constexpr const char *ERROR_COLOR_PREFIX = "";
    static constexpr const char *ERROR_COLOR_END = "";
#endif
};

} // namespace Hardware
} // namespace OHOS
#endif // HC_GEN_LOG_H
