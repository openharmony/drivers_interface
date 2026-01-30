#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import platform
import shutil
import subprocess
import sys
import time
import base64


class TestConfig(object):
    CMAKE_GEN_PATH = "cmake-build-debug"
    WORK_DIR = ""
    HCGEN = ""
    TEMP_DIR = 'temp'
    ERROR_COLOR_PREFIX = "\033[31m"
    ERROR_COLOR_END = "\033[0m"
    SHOULD_CLEAN_TEMP = True
    PERFORMANCE_MAX_COMPILE_TIME_MS = 1000


def text_file_compare(file_a, file_target):
    if not os.path.exists(file_target):
        return True

    with open(file_a, 'r') as f_a:
        with open(file_target, 'r') as f_b:
            a_content = f_a.read().replace(r'\r\n', r'\n')
            b_content = f_b.read().replace(r'\r\n', r'\n')
            return a_content == b_content


def binary_file_compare(file_a, file_target, skip_size=0,
                        target_base64_encode=False):
    if not os.path.exists(file_target):
        return True

    with open(file_a, 'rb') as f_a:
        with open(file_target, 'rb') as f_b:
            a_content = f_a.read()
            b_content = f_b.read()
            if target_base64_encode:
                b_content = base64.b64decode(b_content)
            return a_content[skip_size:] == b_content[skip_size:]


def exec_command(command):
    return subprocess.getstatusoutput(command)


def setup_hcgen_compiler():
    if len(sys.argv) > 1:
        hcgen_path = os.path.abspath(sys.argv[1])
        if hcgen_path.find('hc-gen') >= 0 and os.access(hcgen_path, os.X_OK):
            TestConfig.HCGEN = hcgen_path
            print('use specified hsc:' + hcgen_path)
            return

    source_root = '../../'
    compiler_name = "hc-gen"
    if platform.system() == "Windows":
        source_root = source_root.replace("/", "\\")
        compiler_name += ".exe"

    source_root = os.path.abspath(os.path.join(TestConfig.WORK_DIR, source_root))
    hcgen = os.path.join(source_root, compiler_name)
    if not os.access(hcgen, os.X_OK):
        hcgen = os.path.join(source_root, TestConfig.CMAKE_GEN_PATH, compiler_name)
        if not os.access(hcgen, os.X_OK):
            print("Error: hcgen not found, please make first")
            exit(1)
    TestConfig.HCGEN = hcgen


def index_case(case_path):
    cases = []
    for dir_name in os.listdir(case_path):
        if os.path.isdir(os.path.join(case_path, dir_name)):
            cases.append(dir_name)
    cases.sort()
    return cases


def get_golden_compile_result(mode, case_name):
    result_file_name = os.path.join(TestConfig.WORK_DIR, case_name,
                                    'golden_%s_compile_result.txt' % mode)
    status_prefix = '[compile exit status]:'
    output_prefix = '[compile console output]:\n'
    with open(result_file_name, 'r') as result_file:
        status = result_file.readline()
        status = status[len(status_prefix):]
        console_output = result_file.read()
        console_output = console_output[len(output_prefix):]

        return int(status), console_output.strip()


def compile_status_to_str(status):
    if status:
        return 'success'
    else:
        return 'failed'


def test_compile(case_name, mode):
    output_dir = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR, case_name)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    output_file = os.path.join(output_dir, 'golden')
    source_file = os.path.join(TestConfig.WORK_DIR, case_name, 'case.hcs')
    temp_dir = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR)

    if mode == 'text':
        command = "%s -o %s -t  %s" % (TestConfig.HCGEN, output_file, source_file)
    else:
        command = "%s -o %s %s" % (TestConfig.HCGEN, output_file, source_file)

    status, output = exec_command(command)
    golden_status, golden_output = get_golden_compile_result(mode, case_name)
    if bool(status) != bool(golden_status):
        print("%s mode: case %s expect compile %s but %s" %
              (mode, case_name, compile_status_to_str(status),
               compile_status_to_str(golden_status)))
        print("Console output :\n" + output)
        return False

    output = output.replace(temp_dir, ".").replace(TestConfig.WORK_DIR, "."). \
        replace('\\', '/').replace(TestConfig.ERROR_COLOR_PREFIX, ""). \
        replace(TestConfig.ERROR_COLOR_END, "")
    if output.strip() != golden_output:
        print("output is different with golden for %s compile:" % mode)
        print("EXPECT:\n" + golden_output)
        print("ACTUAL:\n" + output.strip())
        return False

    return True


def binary_code_compile(case_name):
    compile_result = test_compile(case_name, 'binary')
    if not compile_result:
        return False

    compile_start_time = get_current_time_ms()

    case_hcb = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR, case_name, 'golden.hcb')
    golden_hcb = os.path.join(TestConfig.WORK_DIR, case_name, 'golden.hcb')
    hcb_header_size = 20  # hcb compare skip hcb header
    output_compare = \
        binary_file_compare(case_hcb, golden_hcb, hcb_header_size, True)
    if not output_compare:
        print('Error: hcb output mismatch with golden')
        return False

    compile_finish_time = get_current_time_ms()
    compile_used_time = compile_finish_time - compile_start_time
    if compile_used_time > TestConfig.PERFORMANCE_MAX_COMPILE_TIME_MS:
        print('Error: compile time %d, out of threshold %d ms'
              % (compile_used_time, TestConfig.PERFORMANCE_MAX_COMPILE_TIME_MS))
        return False

    decompile_result = test_decompile(case_name)

    return decompile_result


def test_text_code_compile(case_name):
    compile_result = test_compile(case_name, 'text')
    if not compile_result:
        return False

    case_c_file = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR, case_name, 'golden.c')
    golden_c_file = os.path.join(TestConfig.WORK_DIR, case_name, 'golden.c.gen')
    c_file_compare = text_file_compare(case_c_file, golden_c_file)
    if not c_file_compare:
        print("Error: The generated C file mismatch with golden")

    case_header_file = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR, case_name, 'golden.h')
    golden_header_file = os.path.join(TestConfig.WORK_DIR, case_name, 'golden.h.gen')
    header_file_compare = \
        text_file_compare(case_header_file, golden_header_file)
    if not header_file_compare:
        print("Error: The generated header file mismatch with golden")
    return c_file_compare and header_file_compare


def test_decompile(case_name):
    golden_decompile_file_name = \
        os.path.join(TestConfig.WORK_DIR, case_name, 'golden.d.hcs')
    if not os.path.exists(golden_decompile_file_name):
        return True

    output_dir = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR, case_name)
    output_file = os.path.join(output_dir, 'case.hcs')
    source_file = os.path.join(output_dir, 'golden.hcb')
    command = "%s -o %s -d %s" % (TestConfig.HCGEN, output_file, source_file)

    status, output = exec_command(command)
    if status != 0:
        print('decompile fail')
        print(output)
        return False

    decompile_golden_result = text_file_compare(
        os.path.join(output_dir, 'case.d.hcs'), golden_decompile_file_name)
    if not decompile_golden_result:
        print('Error: case %s decompile hcs mismatch with golden' % case_name)
        return False

    return True


def get_current_time_ms():
    return int(round(time.time() * 1000))


def test_cases(cases):
    print('[==========] running %d cases form hcgen test' % len(cases))
    failed_cases = []
    test_start_time = get_current_time_ms()
    for case in cases:
        case_start_time = get_current_time_ms()
        print('[ RUN      ] %s' % case)
        binary_compile_result = binary_code_compile(case)
        text_compile_result = test_text_code_compile(case)
        case_finish_time = get_current_time_ms()
        used_time_str = ' (%d ms)' % (case_finish_time - case_start_time)
        if (not binary_compile_result) or (not text_compile_result):
            print('[    ERROR ] %s%s' % (case, used_time_str))
            failed_cases.append(case)
        else:
            print('[       OK ] %s%s' % (case, used_time_str))
    test_finish_time = get_current_time_ms()
    print('\n[==========] running %d case (%d ms)'
          % (len(cases), test_finish_time - test_start_time))
    print('[  PASSED  ] %d cases' % (len(cases) - len(failed_cases)))
    if len(failed_cases) > 0:
        TestConfig.SHOULD_CLEAN_TEMP = False
        print('[  FAILED  ] %d cases, list below:' % len(failed_cases))
        for case in failed_cases:
            print('[  FAILED  ] %s' % case)


def setup_work_dir():
    pwd = os.path.abspath(sys.argv[0])
    pwd = pwd[:pwd.rfind(os.sep)]
    TestConfig.WORK_DIR = pwd


def test_setup():
    temp_dir = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR)
    if not os.path.exists(temp_dir):
        os.mkdir(temp_dir)


def test_teardown():
    if not TestConfig.SHOULD_CLEAN_TEMP:
        return
    temp_dir = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR)
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)


def clean_up():
    temp_dir = os.path.join(TestConfig.WORK_DIR, TestConfig.TEMP_DIR)
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)


if __name__ == "__main__":
    setup_work_dir()
    clean_up()
    setup_hcgen_compiler()
    print("hcgen path : " + TestConfig.HCGEN)
    cases_list = index_case(TestConfig.WORK_DIR)
    test_setup()
    test_cases(cases_list)
    test_teardown()
