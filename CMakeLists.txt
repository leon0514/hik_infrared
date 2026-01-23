# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)

# 项目名称和语言
project(hikinfrared LANGUAGES CXX)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 寻找依赖 ---
find_package(pybind11 REQUIRED)
find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development)

# --- 定义海康 SDK 路径 ---
set(HIK_SDK_ROOT /opt/hikvision)
set(HIK_INCLUDE_DIR ${HIK_SDK_ROOT}/hik_header)
set(HIK_LIBRARY_DIR ${HIK_SDK_ROOT}/hik_libs)

# --- 定义源文件 ---
# 为了清晰和共用，我们明确列出源文件

# 1. 模块和可执行文件共用的源文件 (例如，核心逻辑)
set(COMMON_SOURCES
    src/HikvisionCamera.cpp
    src/system.cpp
)

# 2. Python 绑定接口文件
set(BINDING_SOURCES
    src/bindings.cpp
)

# 3. C++ 可执行文件的入口文件
set(EXECUTABLE_SOURCES
    src/main.cpp
)


# =======================================================
# 目标 1: 创建 Python 模块 (和之前一样)
# =======================================================
pybind11_add_module(hikinfrared SHARED 
    ${COMMON_SOURCES}
    ${BINDING_SOURCES}
)

# 为模块链接依赖
target_include_directories(hikinfrared PUBLIC
    ${HIK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_directories(hikinfrared PRIVATE ${HIK_LIBRARY_DIR})
target_link_libraries(hikinfrared PRIVATE hcnetsdk dl)

# =======================================================
# 目标 2: 创建一个独立的可执行文件
# =======================================================
add_executable(exec_infrared
    ${COMMON_SOURCES}
    ${EXECUTABLE_SOURCES}
)

# 为可执行文件链接相同的依赖
target_include_directories(exec_infrared PUBLIC
    ${HIK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_directories(exec_infrared PRIVATE ${HIK_LIBRARY_DIR})
target_link_libraries(exec_infrared PRIVATE hcnetsdk dl)


# (可选) 设置 Release 模式下的优化级别
set_target_properties(hikinfrared exec_infrared PROPERTIES
    COMPILE_FLAGS_RELEASE "-O2 -DNDEBUG"
)