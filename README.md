# fluxmt

One database layer, many databases: `fluxmt::core` holds what every target shares, and each driver is a target of its own.

| Target         | Option              | vcpkg feature | Client   |
|----------------|---------------------|---------------|----------|
| `fluxmt::core` | always              | —             | —        |
| `fluxmt::psql` | `FLUXMT_BUILD_PSQL` | `psql`        | libpqxx  |

Requires GCC 16.1+ with C++26 static reflection (`-freflection`) and [erslib](https://github.com/EilRoviSoft/erslib) built with `ERSLIB_ENABLE_REFLECTION=ON`.


## Using with FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    fluxmt
    GIT_REPOSITORY <fluxmt repository>
    GIT_TAG master
)

set(FLUXMT_BUILD_PSQL ON)
FetchContent_MakeAvailable(fluxmt)

target_link_libraries(<your_target> PRIVATE fluxmt::psql)
```

erslib is taken from the parent project if it already has `erslib::core`, then from `find_package(erslib)`, otherwise it is fetched (`FLUXMT_ERSLIB_REPOSITORY`, `FLUXMT_ERSLIB_TAG`). To work on both at once, point it at a local checkout:

```
-DFETCHCONTENT_SOURCE_DIR_ERSLIB=/path/to/erslib
```


## Using an installed fluxmt

```cmake
find_package(fluxmt REQUIRED COMPONENTS psql)
target_link_libraries(<your_target> PRIVATE fluxmt::psql)
```
