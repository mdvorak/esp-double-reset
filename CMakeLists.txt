cmake_minimum_required(VERSION 3.11.0)

idf_component_register(
    SRC_DIRS src
    INCLUDE_DIRS src
    REQUIRES nvs_flash log freertos
)
