# Add set(CONFIG_USE_driver_power true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_driver_common AND (CONFIG_DEVICE_ID STREQUAL LPC54628J512))

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
${CMAKE_CURRENT_LIST_DIR}/drivers/fsl_power.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
${CMAKE_CURRENT_LIST_DIR}/drivers
)

if(CONFIG_TOOLCHAIN STREQUAL mdk AND CONFIG_CORE STREQUAL cm4f)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
-Wl,--start-group
${CMAKE_CURRENT_LIST_DIR}/arm/keil_lib_power.lib
-Wl,--end-group
)
endif()

if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm4f)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
-Wl,--start-group
${CMAKE_CURRENT_LIST_DIR}/iar/iar_lib_power.a
-Wl,--end-group
)
endif()

if(CONFIG_TOOLCHAIN STREQUAL mcux AND CONFIG_CORE STREQUAL cm4f)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
-Wl,--start-group
${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/libpower_hardabi.a
${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/libpower_softabi.a
-Wl,--end-group
)
endif()

if(CONFIG_TOOLCHAIN STREQUAL armgcc AND CONFIG_CORE STREQUAL cm4f)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
-Wl,--start-group
${CMAKE_CURRENT_LIST_DIR}/gcc/libpower_hardabi.a
${CMAKE_CURRENT_LIST_DIR}/gcc/libpower_softabi.a
-Wl,--end-group
)
endif()

else()

message(SEND_ERROR "driver_power.LPC54628 dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
