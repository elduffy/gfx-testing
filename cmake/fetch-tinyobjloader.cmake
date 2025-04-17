FetchContent_Declare(
        tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
        GIT_TAG v2.0.0rc13
        GIT_SHALLOW ON
        GIT_PROGRESS TRUE
        SYSTEM
)
FetchContent_MakeAvailable(tinyobjloader)