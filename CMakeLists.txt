cmake_minimum_required(VERSION 3.16)
project(ERL_HUB LANGUAGES CXX)

add_executable(erl_hub
    src/DispatchManifest.cpp
    src/DistributionDepot.cpp
    src/PersistenceManager.cpp
    src/ReliefCargo.cpp
    src/ReliefHub.cpp
    src/ReliefVehicle.cpp
    src/main.cpp
)

target_include_directories(erl_hub PRIVATE include)
target_compile_features(erl_hub PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(erl_hub PRIVATE /W4 /permissive-)
else()
    target_compile_options(erl_hub PRIVATE -Wall -Wextra -Wpedantic)
endif()
