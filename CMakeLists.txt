cmake_minimum_required(VERSION 3.28)
project(c-playground C)

set(CMAKE_C_STANDARD 17)

find_package (SQLite3 REQUIRED)
find_package (ODBC REQUIRED)

add_executable(c-playground main.c)
add_executable(c-playground-getpost getpost.c)
add_executable(c-playground-odbc odbc.c
        odbc.c)

target_link_libraries(c-playground-odbc PRIVATE ODBC::ODBC)
target_link_libraries(c-playground-getpost PRIVATE SQLite::SQLite3)
