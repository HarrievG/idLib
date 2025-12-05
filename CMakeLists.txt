cmake_minimum_required(VERSION 3.15)
project(idlib_standalone)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

if(NOT TARGET SDL3)
  # If we're being built standalone, the target won't exist, so we fetch it ourselves.
  message(STATUS "SDL target not found, fetching it now.")

	set(SDL_TEST_LIBRARY FALSE)
	set(BUILD_SHARED_LIBS FALSE)
	set(SDL_TESTS OFF)
	set(SDL_SHARED OFF)
	set(SDL_STATIC ON)

	FetchContent_Declare(SDL
	   GIT_REPOSITORY https://github.com/libsdl-org/SDL
	   #GIT_TAG main
	   GIT_TAG release-3.2.8
	   GIT_PROGRESS TRUE
	   GIT_SHALLOW TRUE)
	FetchContent_Makeavailable(SDL)
else()
  message(STATUS "CustomLib: Found existing SDL target provided by parent project, skipping fetch.")
endif()


message(STATUS "idlib : globbing sources")
file(GLOB_RECURSE idlib_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
list(REMOVE_ITEM idlib_sources "test.cpp")
list(REMOVE_ITEM idlib_sources "test_cvar_cmd_fs.cpp")
list(FILTER idlib_sources EXCLUDE REGEX "^out/.*")
message(STATUS "idlib_sources: ${idlib_sources}")

file(GLOB_RECURSE idlib_headers RELATIVE ${CMAKE_SOURCE_DIR} "*.h")
list(FILTER idlib_headers EXCLUDE REGEX "^out/.*")
message(STATUS "idlib_headers: ${idlib_headers}")

add_library(idlib_standalone STATIC ${idlib_sources})
#add_library(idlib_standalone_shared SHARED ${idlib_sources})

target_include_directories(idlib_standalone PUBLIC 
	.
	${SDL3_BINARY_DIR}/include-config-$<LOWER_CASE:$<CONFIG>>
)

target_link_libraries(idlib_standalone PRIVATE SDL3::SDL3-static )

enable_testing()

add_executable(idlib_test test_cvar_cmd_fs.cpp)
target_link_libraries(idlib_test PRIVATE idlib_standalone SDL3::SDL3-static)
target_include_directories(idlib_test PRIVATE .)

add_test(NAME idlib_test COMMAND idlib_test)