####################################################################
# Automatically-generated file. Do not edit!                       #
####################################################################

set(SDK_PATH "C:/Users/tasrivas/.silabs/slt/installs/conan/p/simpl508ee6c1a6569/p")
set(COPIED_SDK_PATH "simplicity_sdk_2026.6.0")
set(PKG_PATH "C:/Users/tasrivas/.silabs/slt/installs")

add_library(slc OBJECT
    "../${COPIED_SDK_PATH}/boards/hardware/board/src/sl_board_control_gpio.c"
    "../${COPIED_SDK_PATH}/boards/hardware/board/src/sl_board_init.c"
    "../${COPIED_SDK_PATH}/boards/hardware/driver/mx25_flash_shutdown/src/sl_mx25_flash_shutdown_eusart/sl_mx25_flash_shutdown.c"
    "../${COPIED_SDK_PATH}/cmsis/RTOS2/Source/os_systick.c"
    "../${COPIED_SDK_PATH}/freertos/cmsis/Source/cmsis_os2.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/croutine.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/event_groups.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/list.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/mpu_wrappers_v2_asm.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/port.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/portable/MemMang/heap_3.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/queue.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/stream_buffer.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/tasks.c"
    "../${COPIED_SDK_PATH}/freertos/kernel/timers.c"
    "../${COPIED_SDK_PATH}/platform_core/hardware/driver/configuration_over_swo/src/sl_cos.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/common/src/sl_assert.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/common/src/sl_cmsis_os2_common.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/common/src/sl_core_cortexm.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/common/src/sl_slist.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/common/src/sl_syscalls.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/Device/SiliconLabs/EFR32MG24/Source/startup_efr32mg24.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/Device/SiliconLabs/EFR32MG24/Source/system_efr32mg24.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/debug/src/sl_debug_swo.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/dma_channel/src/sl_dma_channel.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/dma_channel/src/sl_dma_descriptor_allocator.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/gpio/src/sl_gpio.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/leddrv/src/sl_led.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/leddrv/src/sl_simple_led.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_burtc.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_cmu.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_emu.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_eusart.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_gpio.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_msc.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_prs.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_system.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_timer.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/src/em_usart.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/src/sl_hal_eusart.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/src/sl_hal_gpio.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/src/sl_hal_ldma.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/src/sl_hal_prs.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/src/sl_hal_syscfg.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/src/sl_hal_sysrtc.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/src/sl_hal_sysrtc_subsystem.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/src/sl_hal_system.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/clock_manager/src/sl_clock_manager.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/clock_manager/src/sl_clock_manager_hal_s2.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/clock_manager/src/sl_clock_manager_init.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/clock_manager/src/sl_clock_manager_init_hal_s2.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_init/src/sl_device_init_dcdc_s2.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_init/src/sl_device_init_emu_s2.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_manager/clocks/sl_device_clock_efr32xg24.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_manager/devices/sl_device_peripheral_hal_efr32xg24.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_manager/dma/sl_device_dma_s2.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_manager/src/sl_device_clock.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_manager/src/sl_device_dma.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_manager/src/sl_device_gpio.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_manager/src/sl_device_peripheral.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/dma_manager/src/sl_dma_manager.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/dma_manager/src/sl_dma_manager_hal_ldma.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/interrupt_manager/src/sl_interrupt_manager_cortexm.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/src/sl_memory_manager.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/src/sl_memory_manager_dynamic_reservation.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/src/sl_memory_manager_pool.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/src/sl_memory_manager_pool_common.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/src/sl_memory_manager_region.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/src/sl_memory_manager_retarget.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/src/sli_memory_manager_common.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sl_main/src/rtos/main_retarget.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sl_main/src/sl_main_init.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sl_main/src/sl_main_init_memory.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sl_main/src/sl_main_kernel.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sleeptimer/src/sl_sleeptimer.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sleeptimer/src/sl_sleeptimer_hal_burtc.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sleeptimer/src/sl_sleeptimer_hal_sysrtc.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sleeptimer/src/sl_sleeptimer_hal_timer.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/udelay/src/sl_udelay.c"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/udelay/src/sl_udelay_armv6m_gcc.S"
    "../app.c"
    "../elev_app.c"
    "../elev_gpio.c"
    "../elev_tx.c"
    "../elev_rx.c"
    "../elev_parser.c"
    "../elev_ctrl.c"
    "../autogen/sl_board_default_init.c"
    "../autogen/sl_dma_manager_instances.c"
    "../autogen/sl_event_handler.c"
    "../autogen/sl_simple_led_instances.c"
    "../main.c"
)

target_include_directories(slc PUBLIC
   "../config"
   "../autogen"
   "../."
    "../${COPIED_SDK_PATH}/platform_core/platform/Device/SiliconLabs/EFR32MG24/Include"
    "../${COPIED_SDK_PATH}/platform_core/platform/common/inc"
    "../${COPIED_SDK_PATH}/boards/hardware/board/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/clock_manager/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/clock_manager/src"
    "../${COPIED_SDK_PATH}/cmsis/Core/Include"
    "../${COPIED_SDK_PATH}/cmsis/Core/Include/m-profile"
    "../${COPIED_SDK_PATH}/cmsis/Core/Include/a-profile"
    "../${COPIED_SDK_PATH}/cmsis/RTOS2/Include"
    "../${COPIED_SDK_PATH}/platform_core/hardware/driver/configuration_over_swo/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/debug/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_manager/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/device_init/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/dma_channel/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/dma_manager/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/dma_manager/src"
    "../${COPIED_SDK_PATH}/platform_core/platform/emlib/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/common/errno_error_codes/inc"
    "../${COPIED_SDK_PATH}/freertos/kernel/include"
    "../${COPIED_SDK_PATH}/freertos/cmsis/Include"
    "../${COPIED_SDK_PATH}/freertos/kernel/portable/GCC/ARM_CM33_NTZ/non_secure"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/gpio/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/peripheral/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/interrupt_manager/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/interrupt_manager/src"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/interrupt_manager/inc/arm"
    "../${COPIED_SDK_PATH}/platform_core/platform/driver/leddrv/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/memory_manager/src"
    "../${COPIED_SDK_PATH}/boards/hardware/driver/mx25_flash_shutdown/inc/sl_mx25_flash_shutdown_eusart"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sl_main/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sl_main/src"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sleeptimer/inc"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/sleeptimer/src"
    "../${COPIED_SDK_PATH}/platform_core/platform/service/udelay/inc"
)

target_compile_definitions(slc PUBLIC
    "DEBUG_EFM=1"
    "EFR32MG24B220F1536IM48=1"
    "SL_CODE_COMPONENT_SYSTEM=system"
    "SL_BOARD_NAME=\"BRD4187C\""
    "SL_BOARD_REV=\"A01\""
    "HARDWARE_BOARD_DEFAULT_RF_BAND_2400=1"
    "HARDWARE_BOARD_SUPPORTS_1_RF_BAND=1"
    "HARDWARE_BOARD_SUPPORTS_RF_BAND_2400=1"
    "HFXO_FREQ=39000000"
    "SL_CODE_COMPONENT_CLOCK_MANAGER=clock_manager"
    "_RTE_=1"
    "SL_COMPONENT_CATALOG_PRESENT=1"
    "SL_CODE_COMPONENT_DEVICE_PERIPHERAL=device_peripheral"
    "SL_CODE_COMPONENT_DMA_CHANNEL=dma_channel"
    "SL_CODE_COMPONENT_DMA_MANAGER=dma_manager"
    "SL_CODE_COMPONENT_FREERTOS_KERNEL=freertos_kernel"
    "SL_CODE_COMPONENT_GPIO=gpio"
    "SL_CODE_COMPONENT_HAL_COMMON=hal_common"
    "SL_CODE_COMPONENT_HAL_GPIO=hal_gpio"
    "SL_CODE_COMPONENT_HAL_LDMA=hal_ldma"
    "SL_CODE_COMPONENT_HAL_SYSRTC=hal_sysrtc"
    "SL_CODE_COMPONENT_INTERRUPT_MANAGER=interrupt_manager"
    "CMSIS_NVIC_VIRTUAL=1"
    "CMSIS_NVIC_VIRTUAL_HEADER_FILE=\"cmsis_nvic_virtual.h\""
    "SL_CODE_COMPONENT_MEMORY_MANAGER=memory_manager"
    "SL_CODE_COMPONENT_CORE=core"
    "SL_CODE_COMPONENT_SLEEPTIMER=sleeptimer"
)

target_link_libraries(slc PUBLIC
    "-Wl,--start-group"
    "gcc"
    "c"
    "m"
    "nosys"
    "-Wl,--end-group"
)
target_compile_options(slc PUBLIC
    $<$<COMPILE_LANGUAGE:C>:-mcpu=cortex-m33>
    $<$<COMPILE_LANGUAGE:C>:-mthumb>
    $<$<COMPILE_LANGUAGE:C>:-mfpu=fpv5-sp-d16>
    $<$<COMPILE_LANGUAGE:C>:-mfloat-abi=hard>
    $<$<COMPILE_LANGUAGE:C>:-mcmse>
    $<$<COMPILE_LANGUAGE:C>:-Wall>
    $<$<COMPILE_LANGUAGE:C>:-Wextra>
    $<$<COMPILE_LANGUAGE:C>:-Os>
    $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
    $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:C>:-fomit-frame-pointer>
    $<$<COMPILE_LANGUAGE:C>:-g>
    $<$<COMPILE_LANGUAGE:C>:--specs=nano.specs>
    $<$<COMPILE_LANGUAGE:C>:-fno-lto>
    $<$<COMPILE_LANGUAGE:CXX>:-mcpu=cortex-m33>
    $<$<COMPILE_LANGUAGE:CXX>:-mthumb>
    $<$<COMPILE_LANGUAGE:CXX>:-mfpu=fpv5-sp-d16>
    $<$<COMPILE_LANGUAGE:CXX>:-mfloat-abi=hard>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-mcmse>
    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    $<$<COMPILE_LANGUAGE:CXX>:-Os>
    $<$<COMPILE_LANGUAGE:CXX>:-fdata-sections>
    $<$<COMPILE_LANGUAGE:CXX>:-ffunction-sections>
    $<$<COMPILE_LANGUAGE:CXX>:-fomit-frame-pointer>
    $<$<COMPILE_LANGUAGE:CXX>:-g>
    $<$<COMPILE_LANGUAGE:CXX>:--specs=nano.specs>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-lto>
    $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-m33>
    $<$<COMPILE_LANGUAGE:ASM>:-mthumb>
    $<$<COMPILE_LANGUAGE:ASM>:-mfpu=fpv5-sp-d16>
    $<$<COMPILE_LANGUAGE:ASM>:-mfloat-abi=hard>
    "$<$<COMPILE_LANGUAGE:ASM>:SHELL:-x assembler-with-cpp>"
)

set(post_build_command )
set_property(TARGET slc PROPERTY C_STANDARD 17)
set_property(TARGET slc PROPERTY CXX_STANDARD 17)
set_property(TARGET slc PROPERTY CXX_EXTENSIONS OFF)

target_link_options(slc INTERFACE
    -mcpu=cortex-m33
    -mthumb
    -mfpu=fpv5-sp-d16
    -mfloat-abi=hard
    "-T${CMAKE_CURRENT_LIST_DIR}/../autogen/linkerfile.ld"
    --specs=nano.specs
    -Wl,-Map=$<TARGET_FILE_DIR:Elevator_capacity_controller>/Elevator_capacity_controller.map
    "SHELL:-Wl,--wrap=_free_r -Wl,--wrap=_malloc_r -Wl,--wrap=_calloc_r -Wl,--wrap=_realloc_r"
    -Wl,--wrap=main
    -fno-lto
    -Wl,--gc-sections
)

# BEGIN_SIMPLICITY_STUDIO_METADATA=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=END_SIMPLICITY_STUDIO_METADATA