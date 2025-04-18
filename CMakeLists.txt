cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(xense)

idf_component_register(SRCS "main.c"
                      INCLUDE_DIRS ".")

target_add_binary_data(${COMPONENT_TARGET} "certs/server_cert.pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/server_key.pem" TEXT)
