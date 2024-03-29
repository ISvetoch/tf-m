#-------------------------------------------------------------------------------
# Copyright (c) 2017-2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.7)

# Tell cmake where our modules can be found
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../cmake)

#Include common stuff to control cmake.
include("Common/BuildSys")

#Start an embedded project.
embedded_project_start(CONFIG "${CMAKE_CURRENT_LIST_DIR}/../../../ConfigDefault.cmake")
project(tfm_storage LANGUAGES ASM C)
embedded_project_fixup()

###Some project global settings
set (SECURE_STORAGE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(TFM_ROOT_DIR "${SECURE_STORAGE_DIR}/../../.." ABSOLUTE)

###Get the definition of what files we need to build
include(CMakeLists.inc)

if (NOT DEFINED TFM_LVL)
	message(FATAL_ERROR "Incomplete build configuration: TFM_LVL is undefined. ")
endif()

# Specify what we build (for the secure storage service, build as a static library)
add_library(tfm_storage STATIC ${ALL_SRC_ASM} ${ALL_SRC_C})

#Set common compiler and linker flags
config_setting_shared_compiler_flags(tfm_storage)
config_setting_shared_linker_flags(tfm_storage)

embedded_set_target_compile_defines(TARGET tfm_storage LANGUAGE C DEFINES __ARM_FEATURE_CMSE=3 __thumb2__ TFM_LVL=${TFM_LVL})

embedded_project_end(tfm_storage)
