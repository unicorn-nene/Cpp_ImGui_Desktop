# 0.) Create Source and CMakeFile
# 1.) mkdir build
# 2.) cd build
# 3.) cmake .. - Generting the build Files / Configure the project
# 4.) cmake --build .
# 5.) ./Excutable

cmake_minimum_required(VERSION 3.27.7)
cmake_policy(VERSION 3.10)


project(LightYear
    VERSION 1.0.0
    LANGUAGES C CXX
    )

set(CMAKE_CXX_STANDARD          14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS       OFF)

include(FetchContent)
set(SFML_LIB_NAME SFML)

FetchContent_Declare(
	${SFML_LIB_NAME}
	GIT_REPOSITORY https://github.com/SFML/SFML
	GIT_TAG 2.6.2
	GIT_SHALLOW FALSE
)

FetchContent_MakeAvailable(${SFML_LIB_NAME})

# creates variable
set(LIGHT_YEARS_ENGINE_TARGET_NAME LightYearEngine)
set(LIGHT_YEARS_GAME_TARGET_NAME LightYearsGame)

add_subdirectory(LightYearsEngine)
add_subdirectory(LightYearsGame)


