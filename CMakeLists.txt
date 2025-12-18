if(CONFIG_DEVICE_T_GLASSES_P4)
    set(src_dirs "device_config/t-glasses-p4/")
endif()
    
idf_component_register(
        SRC_DIRS 
        ${src_dirs} 
        INCLUDE_DIRS 
        ${src_dirs} 
    )