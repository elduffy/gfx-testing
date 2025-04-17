FetchContent_Declare(
        Boost
        GIT_REPOSITORY https://github.com/boostorg/boost.git
        GIT_TAG boost-1.88.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        SYSTEM
)
FetchContent_MakeAvailable(Boost)