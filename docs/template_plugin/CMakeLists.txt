# Copyright (C) 2018-2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

# [cmake:main]
cmake_minimum_required(VERSION 3.13)

project(OpenVINOTemplatePlugin)

set(TEMPLATE_PLUGIN_SOURCE_DIR ${OpenVINOTemplatePlugin_SOURCE_DIR})

find_package(OpenVINODeveloperPackage REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX)
    ov_add_compiler_flags(-Wall)
endif()

add_subdirectory(src)

if(ENABLE_TESTS)
    include(CTest)
    enable_testing()

    if(ENABLE_FUNCTIONAL_TESTS)
        add_subdirectory(tests/functional)
    endif()
endif()

# install

ie_cpack(template)
# [cmake:main]
