set(IMGUI_VERSION 1.91.9)
FetchContent_Declare(
        imgui
        URL "https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip"
        SYSTEM
)

FetchContent_MakeAvailable(imgui)
