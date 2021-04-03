cmake_minimum_required(VERSION 3.11.0)

idf_component_register(
        SRCS src/double_reset.c
        INCLUDE_DIRS include
        REQUIRES nvs_flash log freertos
)
