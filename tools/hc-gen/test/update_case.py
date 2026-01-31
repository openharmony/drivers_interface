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
import stat
import subprocess
import sys
import base64


class TestConfig(object):
    CMAKE_GEN_PATH = "cmake-build-debug/"
    WORK_DIR = ""
    ERROR_COLOR_PREFIX = "\033[31m"
    ERROR_COLOR_END = "\033[0m"
    ALIGNED_PARM = ''


def setup_hcgen_compiler():
    if len(sys.argv) > 1:
        hcgen_path = os.path.abspath(sys.argv[1])
        if hcgen_path.find('hc-gen') >= 0 and os.access(hcgen_path, os.X_OK):
            print('use specified hsc:' + hcgen_path)
            return hcgen_path

    source_root = '../../'
    compiler_name = "hc-gen"
    if platform.system() == "Windows":
        source_root = source_root.replace("/", "\\")
        compiler_name += ".exe"

    source_root = os.path.abspath(os.path.join(TestConfig.WORK_DIR, source_root))
    hcgen_path = os.path.join(source_root, compiler_name)
    if not os.access(hcgen_path, os.X_OK):
        hcgen_path = os.path.join(source_root, TestConfig.CMAKE_GEN_PATH, compiler_name)
        if not os.access(hcgen_path, os.X_OK):
            print("Error: hcgen not found, please make first")
            exit(1)

    return hcgen_path


def index_case(case_path):
    cases = []
    for directory in os.listdir(case_path):
        if os.path.isdir(os.path.join(case_path, directory)):
            cases.append(directory)
    cases.sort()
    return cases


def save_compile_result(mode, case_name, status, output):
    result_file_name = os.path.join(TestConfig.WORK_DIR, case_name,
                                    'golden_%s_compile_result.txt' % mode)
    flags = os.O_RDWR | os.O_CREAT
    modes = stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IWGRP | stat.S_IROTH
    with os.fdopen(os.open(result_file_name, flags, modes),
                   "w", encoding="utf-8") as result_output:
        status_str = "[compile exit status]:" + str(status)
        output_str = "\n[compile console output]:\n" + output
        result_output.write(status_str)
        result_output.write(output_str)


def gen_decompile_golden(hcgen_path, case_name):
    source_file = os.path.join(TestConfig.WORK_DIR, case_name, 'golden.hcb')
    target_file = os.path.join(TestConfig.WORK_DIR, case_name, 'golden')
    command = "%s -o %s -d %s" % (hcgen_path, target_file, source_file)
    status, output = subprocess.getstatusoutput(command)
    if status:
        print('case %s decompile fail:\n %s, status=%d' %
              (case_name, output, status))

    return status == 0


def recode_hcb_file(file_path):
    with open(file_path, 'rb') as hcb_file:
        content = hcb_file.read()
        hcb_file.close()

    with open(file_path, 'wb') as hcb_file:
        base64_content = base64.b64encode(content)
        hcb_file.write(base64_content)
        hcb_file.close()


def build_binary(hcgen_path, case_name):
    source_file = os.path.join(TestConfig.WORK_DIR, case_name, 'case.hcs')
    target_file = os.path.join(TestConfig.WORK_DIR, case_name, 'golden')
    command = "%s %s -o %s %s" % \
              (hcgen_path, TestConfig.ALIGNED_PARM, target_file, source_file)
    status, output = subprocess.getstatusoutput(command)
    if case_name.endswith('_ei'):
        if status == 0:
            print("CASE_ERROR:error identify case " + case_name
                  + "expect build fail but success at binary mode")
            return False
    elif status != 0:
        print("CASE_ERROR:case " + case_name
              + " expect build success but failed at binary mode")
        return False
    output = output.replace(TestConfig.WORK_DIR, ".").replace('\\', '/') \
        .replace(TestConfig.ERROR_COLOR_PREFIX, "") \
        .replace(TestConfig.ERROR_COLOR_END, "").strip()

    save_compile_result('binary', case_name, status, output)

    result = True
    if status == 0:
        result = gen_decompile_golden(hcgen_path, case_name)
        if result:
            recode_hcb_file(target_file + '.hcb')
    return result


def build_text(hcgen_path, case_name):
    source_file = os.path.join(TestConfig.WORK_DIR, case_name, 'case.hcs')
    target_file = os.path.join(TestConfig.WORK_DIR, case_name, 'golden')
    command = "%s -t -o %s %s" % (hcgen_path, target_file, source_file)
    status, output = subprocess.getstatusoutput(command)
    if case_name.endswith('_ei') or case_name.find('node_duplicate_name') >= 0:
        if status == 0:
            print("CASE_ERROR:error identify case "
                  + case_name + " expect build fail but success at text mode")
            print(output)
            return False
    elif status != 0:
        print("CASE_ERROR:case " + case_name
              + " expect build success but failed at text mode")
        print(output)
        return False
    output = output.replace(TestConfig.WORK_DIR, ".").replace('\\', '/') \
        .replace(TestConfig.ERROR_COLOR_PREFIX, "") \
        .replace(TestConfig.ERROR_COLOR_END, "").strip()

    save_compile_result('text', case_name, status, output)

    if status == 0:
        os.rename(target_file + '.c', target_file + '.c.gen')
        os.rename(target_file + '.h', target_file + '.h.gen')
    return True


def build_macro(hcgen_path, case_name):
    source_file = os.path.join(TestConfig.WORK_DIR, case_name, 'case.hcs')
    target_file = os.path.join(TestConfig.WORK_DIR, case_name, 'macro')
    command = "%s -m -o %s %s" % (hcgen_path, target_file, source_file)
    status, output = subprocess.getstatusoutput(command)
    if case_name.endswith('_ei'):
        if status == 0:
            print("CASE_ERROR:error identify case "
                  + case_name + " expect build fail but success at macro mode")
            print(output)
            return False
    elif status != 0:
        print("CASE_ERROR:case " + case_name
              + " expect build success but failed at macro mode")
        print(output)
        return False
    output = output.replace(TestConfig.WORK_DIR, ".").replace('\\', '/') \
        .replace(TestConfig.ERROR_COLOR_PREFIX, "") \
        .replace(TestConfig.ERROR_COLOR_END, "").strip()

    save_compile_result('macro', case_name, status, output)

    if status == 0:
        os.rename(target_file + '.h', target_file + '.h.gen')
    return True


def build_cases(hcgen_path, cases):
    index = 1
    failed_cases = []
    for case in cases:
        print('[%02d/%d] build %s' % (index, len(cases), case))
        text_compile = build_text(hcgen_path, case)
        binary_compile = build_binary(hcgen_path, case)
        macro_compile = build_macro(hcgen_path, case)
        if not text_compile or not binary_compile or not macro_compile:
            failed_cases.append(case)
        index += 1

    print("\nUpdate cases result :\ntotal case: %d, failed: %d, success: %d" %
          (len(cases), len(failed_cases), len(cases) - len(failed_cases)))
    if len(failed_cases) > 0:
        print("Failed case as below:")
        for case in failed_cases:
            print(case)


def setup_work_dir():
    pwd = os.path.abspath(sys.argv[0])
    pwd = pwd[:pwd.rfind(os.sep)]
    TestConfig.WORK_DIR = pwd


def clean_up():
    list_dirs = os.walk(TestConfig.WORK_DIR)
    for root, dirs, files in list_dirs:
        for file in files:
            if file.startswith('golden'):
                os.remove(os.path.join(root, file))

    temp_dir = os.path.join(TestConfig.WORK_DIR, 'temp')
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[-1] == '--align':
        TestConfig.ALIGNED_PARM = ' -a '
    setup_work_dir()
    hc_gen = setup_hcgen_compiler()
    clean_up()
    print("hc-gen path : " + hc_gen)
    test_case_list = index_case(TestConfig.WORK_DIR)
    build_cases(hc_gen, test_case_list)
