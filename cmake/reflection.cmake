include(CheckCXXSourceCompiles)


# fluxmt is built on C++26 static reflection (P2996): no fallback without it.
function(fluxmt_require_reflection)
	if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.1)
		message(FATAL_ERROR "fluxmt requires GCC >= 16.1 with -freflection, got ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}.")
	endif()

	set(CMAKE_REQUIRED_FLAGS "-std=c++26 -freflection")
	check_cxx_source_compiles("
		#include <meta>
		struct S { int x; };
		constexpr auto r = ^^S;
		static_assert(std::meta::is_type(r));
		int main() {}
	" FLUXMT_HAVE_P2996)

	if(NOT FLUXMT_HAVE_P2996)
		message(FATAL_ERROR "fluxmt requires C++26 static reflection, '${CMAKE_CXX_COMPILER}' can't compile it.")
	endif()
endfunction()
