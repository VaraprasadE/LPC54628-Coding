# Add set(CONFIG_USE_device_LPC54628_CMSIS true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

if(CONFIG_USE_CMSIS_Include_core_cm AND (CONFIG_DEVICE_ID STREQUAL LPC54628J512))

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
${CMAKE_CURRENT_LIST_DIR}/
)

else()

message(SEND_ERROR "device_LPC54628_CMSIS.LPC54628 dependency does not meet, please check ${CMAKE_CURRENT_LIST_FILE}.")

endif()
