cmake_minimum_required(VERSION 3.21)

project(SteamDeckKeyboardFixSkyrim VERSION 0.3.2 LANGUAGES CXX)

find_package(minhook REQUIRED)

set(BUILD_TESTS OFF CACHE BOOL "Build CommonLibSSE tests" FORCE)
set(ENABLE_SKYRIM_SE OFF CACHE BOOL "Enable pre-1.6 Skyrim support" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "Enable Skyrim 1.6 support" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "Enable Skyrim VR support" FORCE)
set(SKSE_SUPPORT_XBYAK OFF CACHE BOOL "Enable CommonLibSSE Xbyak support" FORCE)
add_subdirectory(lib/commonlibsse-ng EXCLUDE_FROM_ALL)

add_library(${PROJECT_NAME} SHARED plugin.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /experimental:deterministic
        "/pathmap:${CMAKE_SOURCE_DIR}=."
        "/pathmap:${VCPKG_INSTALLED_DIR}=./vcpkg_installed"
    )
    target_compile_options(CommonLibSSE PRIVATE
        /experimental:deterministic
        "/pathmap:${CMAKE_SOURCE_DIR}=."
        "/pathmap:${VCPKG_INSTALLED_DIR}=./vcpkg_installed"
    )
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        CommonLibSSE::CommonLibSSE
        minhook::minhook
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION SKSE/Plugins)
install(FILES
    LICENSE
    README.md
    TECHNICAL.md
    THIRD_PARTY_LICENSES.txt
    DESTINATION docs
)
install(FILES lib/commonlibsse-ng/COPYING
    DESTINATION docs
    RENAME COMMONLIBSSE_GPL-3.0.txt
)
install(FILES lib/commonlibsse-ng/EXCEPTIONS.md
    DESTINATION docs
    RENAME COMMONLIBSSE_EXCEPTIONS.md
)
install(FILES lib/commonlibsse-ng/LICENSE
    DESTINATION docs
    RENAME COMMONLIBSSE_MIT.txt
)

set(CPACK_GENERATOR ZIP)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_v${PROJECT_VERSION}")
include(CPack)
