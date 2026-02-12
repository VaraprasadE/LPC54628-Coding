# Add set(CONFIG_USE_device_LPC54628_startup true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_device_LPC54628_system)

if(CONFIG_TOOLCHAIN STREQUAL iar)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/iar/startup_LPC54628.s "" device_LPC54628_startup.LPC54628)
endif()

if(CONFIG_TOOLCHAIN STREQUAL armgcc)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/startup_LPC54628.S "" device_LPC54628_startup.LPC54628)
endif()

if(CONFIG_TOOLCHAIN STREQUAL mdk)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/arm/startup_LPC54628.S "" device_LPC54628_startup.LPC54628)
endif()

if(CONFIG_TOOLCHAIN STREQUAL mcux)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/startup_lpc54628.c "" device_LPC54628_startup.LPC54628)
add_config_file(${CMAKE_CURRENT_LIST_DIR}/mcuxpresso/startup_lpc54628.cpp "" device_LPC54628_startup.LPC54628)
endif()

else()

message(SEND_ERROR "device_LPC54628_startup.LPC54628 dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
