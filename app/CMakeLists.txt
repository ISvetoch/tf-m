#-------------------------------------------------------------------------------
# Copyright (c) 2017-2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.7)

#Tell cmake where our modules can be found
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake)

#Include common stuff to control cmake.
include("Common/BuildSys")

#Start an embedded project.
embedded_project_start(CONFIG "${CMAKE_CURRENT_LIST_DIR}/../ConfigDefault.cmake")
project(tfm_ns LANGUAGES ASM C)
embedded_project_fixup()

set(APP_DIR ${CMAKE_CURRENT_LIST_DIR})
get_filename_component(TFM_ROOT_DIR ${APP_DIR}/.. ABSOLUTE)
set(INTERFACE_DIR ${TFM_ROOT_DIR}/interface)
set(TEST_DIR ${TFM_ROOT_DIR}/test)

#Include BL2 bootloader related functions
set(MCUBOOT_DIR "${TFM_ROOT_DIR}/bl2/ext/mcuboot")
include("${MCUBOOT_DIR}/MCUBoot.cmake")

#Set variables
get_filename_component(CMSIS_5_DIR ${TFM_ROOT_DIR}/../CMSIS_5 ABSOLUTE)

if(NOT EXISTS ${CMSIS_5_DIR})
	message(FATAL_ERROR "Missing CMSIS_5. Please clone the CMSIS_5 repo to directory \"${CMSIS_5_DIR}\".")
endif()

if (NOT DEFINED BL2)
	message(FATAL_ERROR "Incomplete build configuration: BL2 is undefined. ")
endif ()

if (NOT DEFINED TFM_PARTITION_AUDIT_LOG)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_AUDIT_LOG is undefined.")
endif()

if (NOT DEFINED TFM_PARTITION_PLATFORM)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_PLATFORM is undefined.")
endif()

set(NS_APP_SRC "${CMSIS_5_DIR}/CMSIS/RTOS2/RTX/Config/RTX_Config.c"
	"${CMSIS_5_DIR}/CMSIS/RTOS2/RTX/Source/rtx_lib.c"
	"${APP_DIR}/main_ns.c"
	"${APP_DIR}/tfm_integ_test.c"
	"${APP_DIR}/os_wrapper_rtx.c"
	"${INTERFACE_DIR}/src/tfm_sst_api.c"
	"${INTERFACE_DIR}/src/tfm_crypto_api.c"
	"${INTERFACE_DIR}/src/tfm_initial_attestation_api.c"
	"${INTERFACE_DIR}/src/tfm_ns_lock_rtx.c"
	)

if (TFM_PARTITION_AUDIT_LOG)
	list(APPEND NS_APP_SRC "${INTERFACE_DIR}/src/tfm_audit_api.c")
endif()

if (TFM_PARTITION_PLATFORM)
	list(APPEND NS_APP_SRC "${INTERFACE_DIR}/src/tfm_platform_api.c")
endif()

if (NOT DEFINED TFM_NS_CLIENT_IDENTIFICATION)
	message(FATAL_ERROR "Incomplete build configuration: TFM_NS_CLIENT_IDENTIFICATION is undefined.")
elseif (TFM_NS_CLIENT_IDENTIFICATION)
	list(APPEND NS_APP_SRC
		"${INTERFACE_DIR}/src/tfm_nspm_svc_handler.c"
		"${INTERFACE_DIR}/src/tfm_nspm_api.c"
		)
endif()

if (PSA_API_TEST_NS)
	list(APPEND NS_APP_SRC "${APP_DIR}/psa_api_test.c")
endif()

if (NOT DEFINED TFM_PSA_API)
	message(FATAL_ERROR "Incomplete build configuration: TFM_PSA_API is undefined. ")
elseif (TFM_PSA_API)
	list(APPEND NS_APP_SRC "${INTERFACE_DIR}/src/tfm_psa_ns_api.c")
endif()

set(BUILD_CMSIS_CORE On)
set(BUILD_RETARGET On)
set(BUILD_NATIVE_DRIVERS On)
set(BUILD_TIME Off)
set(BUILD_STARTUP On)
set(BUILD_TARGET_CFG Off)
set(BUILD_TARGET_HARDWARE_KEYS Off)
set(BUILD_TARGET_NV_COUNTERS Off)
set(BUILD_CMSIS_DRIVERS On)
set(BUILD_UART_STDOUT Off)
set(BUILD_FLASH Off)
set(BUILD_BOOT_SEED Off)
set(BUILD_DEVICE_ID Off)
if(NOT DEFINED PLATFORM_CMAKE_FILE)
	message (FATAL_ERROR "Platform specific CMake is not defined. Please set PLATFORM_CMAKE_FILE.")
elseif(NOT EXISTS ${PLATFORM_CMAKE_FILE})
	message (FATAL_ERROR "Platform specific CMake \"${PLATFORM_CMAKE_FILE}\" file does not exist. Please fix value of PLATFORM_CMAKE_FILE.")
else()
	include(${PLATFORM_CMAKE_FILE})
endif()

if(NOT DEFINED NS_SCATTER_FILE_NAME)
	message(FATAL_ERROR "ERROR: Incomplete Configuration: NS_SCATTER_FILE_NAME not defined, Include this file from a Config*.cmake")
endif()
embedded_set_target_linker_file(TARGET ${PROJECT_NAME} PATH  "${NS_SCATTER_FILE_NAME}")

#Create an object library to avoid compiling all source files twice, when two executables
#with different memory map need to be linked(BL2 non-swapping)
set(PROJECT_OBJ_LIB ${PROJECT_NAME}_obj_lib)
add_library(${PROJECT_OBJ_LIB} OBJECT ${ALL_SRC_C} ${ALL_SRC_C_NS} ${ALL_SRC_ASM_NS} ${NS_APP_SRC})

#Set common compiler flags
config_setting_shared_compiler_flags(${PROJECT_OBJ_LIB})

#Set macro definitions
target_compile_definitions(${PROJECT_OBJ_LIB} PRIVATE __thumb2__ __DOMAIN_NS=1 __ARM_FEATURE_CMSE=3 LOG_MSG_HANDLER_MODE_PRINTF_ENABLED)

#Set include directories.
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${TEST_INTERFACE_DIR}/include ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${INTERFACE_DIR}/include ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${TFM_ROOT_DIR} ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${TFM_ROOT_DIR}/secure_fw/spm ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${TFM_ROOT_DIR}/secure_fw/core ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${CMSIS_5_DIR}/CMSIS/RTOS2/RTX/Include ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${CMSIS_5_DIR}/CMSIS/RTOS2/Include ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${CMSIS_5_DIR}/CMSIS/RTOS2/RTX/Config  ABSOLUTE APPEND)

if (NOT DEFINED TFM_NS_CLIENT_IDENTIFICATION)
	message(FATAL_ERROR "Incomplete build configuration: TFM_NS_CLIENT_IDENTIFICATION is undefined.")
elseif (TFM_NS_CLIENT_IDENTIFICATION)
	target_compile_definitions(${PROJECT_OBJ_LIB} PRIVATE TFM_NS_CLIENT_IDENTIFICATION)
endif()

# For the non-swapping BL2 configuration two executables need to be built.
# One can be executed from flash partition slot_0 and other from slot_1.
# Only the linking phase is different. This function captures common settings
# and eliminates copy-paste.
function(set_up_app_build)
	set( _OPTIONS_ARGS)                                                            #Option (on/off) arguments (e.g. IGNORE_CASE)
	set( _ONE_VALUE_ARGS NS_TARGET S_TARGET FULL_BIN SIGN_BIN VENEER_NAME POSTFIX) #Single option arguments (e.g. PATH "./foo/bar")
	set( _MULTI_VALUE_ARGS LINK_DEFINES)                                           #List arguments (e.g. LANGUAGES C ASM CXX)
	cmake_parse_arguments(_MY_PARAMS "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

	if (NOT DEFINED _MY_PARAMS_NS_TARGET)
		message(FATAL_ERROR "set_up_app_build(): mandatory parameter 'NS_TARGET' missing.")
	endif()

	if (NOT DEFINED _MY_PARAMS_S_TARGET)
		message(FATAL_ERROR "set_up_app_build(): mandatory parameter 'S_TARGET' missing.")
	endif()

	if (NOT DEFINED _MY_PARAMS_FULL_BIN)
		message(FATAL_ERROR "set_up_app_build(): mandatory parameter 'FULL_BIN' missing.")
	endif()

	if (NOT DEFINED _MY_PARAMS_SIGN_BIN)
		message(FATAL_ERROR "set_up_app_build(): mandatory parameter 'SIGN_BIN' missing.")
	endif()

	if (NOT DEFINED _MY_PARAMS_VENEER_NAME)
		message(FATAL_ERROR "set_up_app_build(): mandatory parameter 'VENEER_NAME' missing.")
	endif()

	set(EXE_NAME ${_MY_PARAMS_NS_TARGET}${_MY_PARAMS_POSTFIX})
	set(S_BIN ${_MY_PARAMS_S_TARGET}${_MY_PARAMS_POSTFIX})
	set(FULL_NAME ${_MY_PARAMS_FULL_BIN}${_MY_PARAMS_POSTFIX})
	set(SIGN_NAME ${_MY_PARAMS_SIGN_BIN}${_MY_PARAMS_POSTFIX})
	set(VENEER_NAME ${_MY_PARAMS_VENEER_NAME}${_MY_PARAMS_POSTFIX}.o)

	#Create linker target: add object library to executable
	add_executable(${EXE_NAME} $<TARGET_OBJECTS:${PROJECT_OBJ_LIB}>)

	#Set common linker flags
	config_setting_shared_linker_flags(${EXE_NAME})

	#Set individual linker flags per linker target/executable
	foreach(flag ${_MY_PARAMS_LINK_DEFINES})
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "${flag}")
	endforeach(flag)

	embedded_set_target_linker_file(TARGET ${EXE_NAME} PATH "${NS_SCATTER_FILE_NAME}")

	#Add the RTX library
	if(NOT DEFINED RTX_LIB_PATH)
		message(FATAL_ERROR "ERROR: Incomplete Configuration: RTX_LIB_PATH is not defined.")
	endif()
	target_link_libraries(${EXE_NAME} "${RTX_LIB_PATH}")

	#Add the PSA API compliance test libraries
	if(NOT DEFINED PSA_API_TEST_BUILD_PATH)
		#If not specified, assume it's the default build folder checked out at the same level of TFM root dir
		set(PSA_API_TEST_BUILD_PATH "${TFM_ROOT_DIR}/../psa-arch-tests/api-tests/BUILD")
	endif()
	if(PSA_API_TEST_NS)
		target_link_libraries(${EXE_NAME} "${PSA_API_TEST_BUILD_PATH}/val/val_nspe.a")
		target_link_libraries(${EXE_NAME} "${PSA_API_TEST_BUILD_PATH}/platform/pal_nspe.a")
	endif()
	if(PSA_API_TEST_NS AND PSA_API_TEST_SECURE_STORAGE)
		target_link_libraries(${EXE_NAME} "${PSA_API_TEST_BUILD_PATH}/dev_apis/protected_storage/test_combine.a")
	endif()
	if(PSA_API_TEST_NS AND PSA_API_TEST_CRYPTO)
		target_link_libraries(${EXE_NAME} "${PSA_API_TEST_BUILD_PATH}/dev_apis/crypto/test_combine.a")
	endif()
	if(PSA_API_TEST_NS AND PSA_API_TEST_ATTESTATION)
		target_link_libraries(${EXE_NAME} "${PSA_API_TEST_BUILD_PATH}/dev_apis/initial_attestation/test_combine.a")
	endif()

	if(NOT DEFINED PLATFORM_LINK_INCLUDES)
		message(FATAL_ERROR "ERROR: Incomplete Configuration: PLATFORM_LINK_INCLUDES is not defined.")
	endif()
	embedded_set_target_link_includes(TARGET ${EXE_NAME} INCLUDES "${PLATFORM_LINK_INCLUDES}")

	#Generate binary file from axf
	compiler_generate_binary_output(${EXE_NAME})

	#Generate MCUBoot compatible payload
	if (BL2)
		mcuboot_create_boot_payload(S_BIN    ${S_BIN}
									NS_BIN   ${EXE_NAME}
									FULL_BIN ${FULL_NAME}
									SIGN_BIN ${SIGN_NAME}
									POSTFIX  ${_MY_PARAMS_POSTFIX})
	endif()

	if (NOT DEFINED TFM_PARTITION_TEST_CORE)
		message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_CORE is undefined. ")
	elseif (TFM_PARTITION_TEST_CORE)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_CORE")
	endif()

	if (NOT DEFINED TFM_PARTITION_TEST_CORE_IPC)
		message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_CORE_IPC is undefined.")
	elseif (TFM_PARTITION_TEST_CORE_IPC)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_CORE_IPC")
	endif()

	if (NOT DEFINED TFM_PARTITION_TEST_SECURE_SERVICES)
		message(FATAL_ERROR "Incomplete build configuration: TFM_PARTITION_TEST_SECURE_SERVICES is undefined. ")
	elseif (TFM_PARTITION_TEST_SECURE_SERVICES)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TFM_PARTITION_TEST_SECURE_SERVICES")
	endif()

	if (NOT DEFINED TEST_FRAMEWORK_S)
		message(FATAL_ERROR "Incomplete build configuration: TEST_FRAMEWORK_S is undefined.")
	elseif (TEST_FRAMEWORK_S)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TEST_FRAMEWORK_S")
	endif()

	if (NOT DEFINED TEST_FRAMEWORK_NS)
		message(FATAL_ERROR "Incomplete build configuration: TEST_FRAMEWORK_NS is undefined.")
	elseif (TEST_FRAMEWORK_NS)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "TEST_FRAMEWORK_NS")
	endif()

	#Set BL2 specific settings.
	if (BL2)
		embedded_set_target_link_defines(TARGET ${EXE_NAME} DEFINES "BL2")
	endif()

	#We depend on the non secure tests. See if the library target is available.
	if(TARGET tfm_non_secure_tests)
		#If yes, then use the library.
		target_link_libraries(${EXE_NAME} tfm_non_secure_tests)
		#Ensure library is built first.
		add_dependencies(${EXE_NAME} tfm_non_secure_tests)
	endif()

	#Ensure secure_fw is built before our executable.
	add_dependencies(${EXE_NAME} ${S_BIN})

	#Add the veneers to the executable.
	set(S_VENEER_FILE "${CMAKE_CURRENT_BINARY_DIR}/${VENEER_NAME}")
	set_property(TARGET ${EXE_NAME} APPEND PROPERTY LINK_LIBRARIES ${S_VENEER_FILE})

	#Collect executables to common location: build/install/outputs/
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.axf
				  ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.bin
			DESTINATION outputs/${TARGET_PLATFORM}/)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.axf
				  ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.bin
			DESTINATION outputs/fvp/)
endfunction()

if (NOT TARGET_TFM_S_EXISTED)
	set(S_VENEER_FILE_LOCATION "${CMAKE_CURRENT_BINARY_DIR}")
	add_subdirectory(../secure_fw ${CMAKE_CURRENT_BINARY_DIR}/secure_fw)
endif()

if (LINK_TO_BOTH_MEMORY_REGION)
	#Link to primary memory region
	set_up_app_build(NS_TARGET     ${PROJECT_NAME}
					 S_TARGET      tfm_s
					 FULL_BIN      tfm_full
					 SIGN_BIN      tfm_sign
					 VENEER_NAME   s_veneers
					 POSTFIX       "_0")

	#Link to secondary memory region(add extra linker flag)
	set_up_app_build(NS_TARGET     ${PROJECT_NAME}
					 LINK_DEFINES  "LINK_TO_SECONDARY_PARTITION"
					 S_TARGET      tfm_s
					 FULL_BIN      tfm_full
					 SIGN_BIN      tfm_sign
					 VENEER_NAME   s_veneers
					 POSTFIX       "_1")
else()
	#Link to primary memory region only
	set_up_app_build(NS_TARGET     ${PROJECT_NAME}
					 S_TARGET      tfm_s
					 FULL_BIN      tfm_full
					 SIGN_BIN      tfm_sign
					 VENEER_NAME   s_veneers)
endif()

#If the tfm_non_secure_tests target is not available
if(NOT TARGET tfm_non_secure_tests)
	#Add the test source to the build.
	#As of today since secure_fw is built as a sub-project this code will never execute.
	option(ENABLE_INVERT_SERVICE_TESTS "" TRUE)
	option(ENABLE_SECURE_STORAGE_SERVICE_TESTS "" TRUE)
	include(../test/CMakeLists.inc)
	target_sources(${PROJECT_OBJ_LIB} PUBLIC ${ALL_SRC_C} ${ALL_SRC_C_NS})
endif()

#Finally let CMake system apply changes after the whole project is defined.
if (TARGET ${PROJECT_NAME})
	embedded_project_end(${PROJECT_NAME})
endif()

if (TARGET ${PROJECT_NAME}_0)
	embedded_project_end(${PROJECT_NAME}_0)
endif()

if (TARGET ${PROJECT_NAME}_1)
	embedded_project_end(${PROJECT_NAME}_1)
endif()

embedded_project_end(${PROJECT_OBJ_LIB})
