# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)

# 项目名称和语言 (可以根据需要重命名项目)
project(HikvisionApp LANGUAGES CXX)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 定义海康 SDK 路径 ---
set(HIK_SDK_ROOT /opt/hikvision)
set(HIK_INCLUDE_DIR ${HIK_SDK_ROOT}/hik_header)
set(HIK_LIBRARY_DIR ${HIK_SDK_ROOT}/hik_libs)

# --- 定义可执行文件的所有源文件 ---
set(EXECUTABLE_SOURCES
    src/HikvisionCamera.cpp
    src/system.cpp
    src/main.cpp
)

# =======================================================
# 目标: 创建一个独立的可执行文件
# =======================================================
add_executable(exec_infrared
    ${EXECUTABLE_SOURCES}
)

# --- 配置可执行文件的依赖 ---

# 1. 添加头文件包含目录
target_include_directories(exec_infrared PUBLIC
    # 海康 SDK 的头文件目录
    ${HIK_INCLUDE_DIR}
    # 项目自身的头文件目录 (例如 "src" 目录)
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 2. 添加库文件搜索目录
target_link_directories(exec_infrared PRIVATE ${HIK_LIBRARY_DIR})

# 3. 链接具体的库
# 链接海康的 hcnetsdk 库和 dl 库 (动态链接库加载所需)
target_link_libraries(exec_infrared PRIVATE hcnetsdk dl)


# (可选) 设置 Release 模式下的编译优化级别
set_target_properties(exec_infrared PROPERTIES
    COMPILE_FLAGS_RELEASE "-O2 -DNDEBUG"
)

# --- 定义可执行文件的所有源文件 ---
set(EXECUTABLE_SOURCES
    src/HikvisionCamera.cpp
    src/system.cpp
    src/test_system.cpp
)

# =======================================================
# 目标: 创建一个独立的可执行文件
# =======================================================
add_executable(exec_test_system
    ${EXECUTABLE_SOURCES}
)

# --- 配置可执行文件的依赖 ---

# 1. 添加头文件包含目录
target_include_directories(exec_test_system PUBLIC
    # 海康 SDK 的头文件目录
    ${HIK_INCLUDE_DIR}
    # 项目自身的头文件目录 (例如 "src" 目录)
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 2. 添加库文件搜索目录
target_link_directories(exec_test_system PRIVATE ${HIK_LIBRARY_DIR})

# 3. 链接具体的库
# 链接海康的 hcnetsdk 库和 dl 库 (动态链接库加载所需)
target_link_libraries(exec_test_system PRIVATE hcnetsdk dl)


# (可选) 设置 Release 模式下的编译优化级别
set_target_properties(exec_test_system PROPERTIES
    COMPILE_FLAGS_RELEASE "-O2 -DNDEBUG"
)

# (推荐) 打印出重要路径方便调试
message(STATUS "Hikvision SDK include path: ${HIK_INCLUDE_DIR}")
message(STATUS "Hikvision SDK library path: ${HIK_LIBRARY_DIR}")