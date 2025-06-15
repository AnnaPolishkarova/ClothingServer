cmake_minimum_required(VERSION 3.20)
project(ClothingServer)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

add_definitions(-D_WIN32_WINNT=0x0601)
cmake_policy(SET CMP0144 NEW)
cmake_policy(SET CMP0167 NEW)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "")


# Specify Boost root directory and architecture
set(Boost_USE_STATIC_LIBS ON) # Use static libraries
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_ROOT "C:/vcpkg/installed/x64-windows")  # Указываем  расположение Boost
#set(BOOST_INCLUDEDIR "C:/Boost/include/boost-1_87")
#set(BOOST_LIBRARYDIR "C:/Boost/lib")  # Указываем путь к библиотекам Boost
set(Boost_NO_SYSTEM_PATHS ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS asio thread system filesystem)

if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
    message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
else()
    message(FATAL_ERROR "Boost NOT found!")
endif()

# Для MinGW явно линкуем Winsock
if(MINGW)
    list(APPEND EXTRA_LIBS ws2_32 wsock32)
endif()

# Find nlohmann_json
include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Add Boost include directory
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(C:/vcpkg/packages/nlohmann-json_x64-windows/include)

# Add Boost library directory
link_directories(${BOOST_LIBRARYDIR})

# Collect all source files
file(GLOB SOURCES "src/*.cpp")

# Создание исполняемого файла
add_executable(ClothingServer
        src/main.cpp
        src/Server.cpp
        src/Database.cpp
        src/RequestHandler.cpp
        src/ClothingItem.cpp
        src/network/HttpSession.cpp
        src/network/NetworkManager.cpp
)

target_include_directories(ClothingServer PRIVATE
        ${Boost_INCLUDE_DIRS}  # Критически важно для поиска boost/asio.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
#target_include_directories(ClothingServer PRIVATE
#        Boost::boost
#        Boost::asio
#        Boost::thread
#        Boost::system
#        Boost::filesystem
#        ${CMAKE_CURRENT_SOURCE_DIR}/include
#        ${CMAKE_CURRENT_SOURCE_DIR}/src
#)

# Link Boost libraries
#target_link_libraries(ClothingServer PRIVATE
#        Boost::boost
#        Boost::asio
#        Boost::thread
#        Boost::system
#        Boost::filesystem
#        nlohmann_json::nlohmann_json
#)
#target_link_libraries(ClothingServer PRIVATE
#        ${Boost_LIBRARIES}
#        nlohmann_json::nlohmann_json
#)
target_link_libraries(ClothingServer PRIVATE
        Boost::asio
        Boost::system
        Boost::filesystem
        Boost::thread
        nlohmann_json::nlohmann_json
        ${EXTRA_LIBS}  # Winsock для MinGW
)

# Устанавливаем GoogleTest через CMake
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/gtest/googletest/include)
add_subdirectory(third_party/gtest)
## Настройка Google Test
#if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/CMakeLists.txt")
#    add_subdirectory(third_party/gtest)
#else()
#    message(WARNING "GTest not found in third_party/gtest")
#endif()

# Add test source files
set(TEST_SOURCES
        tests/test_database.cpp
        tests/test_requesthandler.cpp
#        tests/test_server.cpp
        tests/test_network.cpp
        src/Database.cpp
        src/RequestHandler.cpp
        src/Server.cpp
        src/ClothingItem.cpp
)

# Создание исполнимых файлов для тестов
#add_executable(test_database
#        tests/test_database.cpp
#        src/Database.cpp
#        src/ClothingItem.cpp
#)
add_executable(test_database
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_database.cpp
        src/Database.cpp
        src/ClothingItem.cpp
)
#add_executable(test_requesthandler
#        tests/test_requesthandler.cpp
#        src/RequestHandler.cpp
#        src/Database.cpp
#        src/ClothingItem.cpp
#)
add_executable(test_requesthandler
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_requesthandler.cpp
        src/RequestHandler.cpp
        src/Database.cpp
        src/ClothingItem.cpp
)
#add_executable(test_network
#        tests/test_network.cpp
#        src/Database.cpp
#        src/ClothingItem.cpp
#        src/RequestHandler.cpp
#        src/network/HttpSession.cpp
#        src/network/NetworkManager.cpp
#)
add_executable(test_network
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_network.cpp
        src/Database.cpp
        src/ClothingItem.cpp
        src/RequestHandler.cpp
        src/network/HttpSession.cpp
        src/network/NetworkManager.cpp
)

# Линковка с GoogleTest
target_link_libraries(test_database PRIVATE
        gtest_main
        Boost::thread
        Boost::system
        Boost::filesystem
)
target_link_libraries(test_requesthandler PRIVATE
        gtest_main
        Boost::thread
        Boost::system
        Boost::filesystem
)

target_link_libraries(test_network PRIVATE
        gtest_main
        Boost::thread
        Boost::system
        Boost::filesystem
        nlohmann_json::nlohmann_json
        ws2_32  # Доб библиотеку Winsock
        mswsock  # Дополнительные библиотеки для Windows Sockets
)

# Включаем тестирование
enable_testing()

# Добавляем тесты для автоматического запуска
add_test(NAME TestDatabase COMMAND test_database)
add_test(NAME TestRequestHandler COMMAND test_requesthandler)
#add_test(NAME TestServer COMMAND test_server)
add_test(NAME TestNetwork COMMAND test_network)
