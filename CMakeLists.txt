#
cmake_minimum_required (VERSION 3.8)


#
project ("crypto-exchange-client-core")

if (MSVC)
	add_compile_options(-D_WIN32_WINNT=0x0601)
	add_compile_options(-bigobj)
endif()


#
##
set(Boost_MULTITHREADED ON)

find_package(Boost REQUIRED)
include_directories(
	${Boost_INCLUDE_DIRS}
)

##
include_directories("include")


#
add_subdirectory ("src/crypto-exchange-client-core")
