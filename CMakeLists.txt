set(
        requires
        "driver"
        "fatfs"
        "spiffs"
)

if(CONFIG_LILYGO_DEVICE_DRIVER_T_GLASSES_P4)
    set(
            src_dirs
            "./src/"
            "./src/chip/esp32p4/"
            "./src/device/common/"
            "./src/device/t_glasses_p4/"
    )
    list(APPEND requires
            "cpp_bus_driver"
            "usp_cpp_bus_driver"
            "esp_codec_dev"
    )
elseif(CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4_AIR)
    set(
            src_dirs
            "./src/"
            "./src/chip/esp32p4/"
            "./src/device/common/"
            "./src/device/t_display_p4_air/"
        )
    list(APPEND requires
            "cpp_bus_driver"
            "sensorlib"
            "usp_cpp_bus_driver"
            "stsw-st25rfal002_cpp_bus_driver"
            "esp_codec_dev"
            "bhi2xy_sensorapi_cpp_bus_driver")
elseif(CONFIG_LILYGO_DEVICE_DRIVER_T_SPE)
    set(
            src_dirs
            "./src/"
            "./src/device/t_spe/"
        )
elseif(CONFIG_LILYGO_DEVICE_DRIVER_T_CAN485_C5)
    set(
            src_dirs
            "./src/"
            "./src/device/t_can485_c5/"
        )
elseif(CONFIG_LILYGO_DEVICE_DRIVER_T_DISPLAY_P4)
    if(CONFIG_LILYGO_DEVICE_DRIVER_DEVICE_VERSION_V2)
        set(p4_version_dir "./src/device/t_display_p4/v2/")
        list(APPEND requires "esp_codec_dev"
                "bhi2xy_sensorapi_cpp_bus_driver")
    else()
        set(p4_version_dir "./src/device/t_display_p4/v1/")
    endif()
    list(APPEND requires "stsw-st25rfal002_cpp_bus_driver" "sensorlib")
    set(
            src_dirs
            "./src/"
            "./src/chip/esp32p4/"
            "./src/device/common/"
            "./src/device/t_display_p4/"
            "${p4_version_dir}"
        )
    list(APPEND requires
            "cpp_bus_driver"
            "usp_cpp_bus_driver")
endif()

# 所有设备共用独立日志模块。
list(APPEND src_dirs "./src/core/")

idf_component_register(
        SRC_DIRS
        ${src_dirs}
        INCLUDE_DIRS
        ${src_dirs}
        REQUIRES
        ${requires}
    )
