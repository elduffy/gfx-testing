set(FASTGLTF_COMPILE_AS_CPP20 ON)

FetchContent_Declare(
        fastgltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf.git
        GIT_TAG v0.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        SYSTEM
)

FetchContent_MakeAvailable(fastgltf)
