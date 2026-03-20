# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

This is a CMake + vcpkg C++20 project. A `CMakeUserPresets.json` (not committed) must inherit the `vcpkg` preset from `CMakePresets.json` and set the `VCPKG_ROOT` environment variable.

```bash
# Configure (only needed once or after CMakeLists.txt changes)
cmake --preset default

# Build everything (renderer, shaders, tests)
cmake --build build

# Build just the renderer
cmake --build build --target renderer

# Build and run tests (Catch2)
cmake --build build --target tests && ./build/tests

# Run a single test by name
./build/tests "test name substring"

# Run the renderer
./build/app/renderer/renderer
```

Shaders are compiled automatically as part of the build via the `build_shaders` target (HLSL → SPIR-V → JSON → C++ headers). External tools required: `shadercross`, `spirv-cross`, `spirv-opt`.

## Formatting

Uses clang-format (LLVM-based style, 120 column limit, 4-space indent). Run `clang-format -i <file>` to format.

## Architecture

**SDL3 GPU rendering engine** with EnTT ECS, HLSL shaders compiled to SPIR-V, and JSON-driven scene definitions.

### Core components

- **GameContext** (`src/game.hpp`) — Top-level state: SDL context, ECS registry, pipelines, samplers, resource loader. Owns the main loop with FPS capping.
- **SdlContext** (`src/sdl.hpp`) — RAII wrappers around all SDL3 GPU objects (`SdlGfxPipeline`, `SdlGpuBuffer`, `SdlGpuTexture`, etc.). GPU resource lifetime is managed through these wrappers.
- **Scene** (`src/render/scene.hpp`) — Orchestrates per-frame rendering. Holds Camera, SceneObjects, and creates DrawContext per frame.

### Rendering flow

`GameContext::main_loop` → `Scene::draw()` → iterates ECS entities with `RenderObject` components, dispatches draw calls per pipeline through `DrawContext` (wraps SDL command buffer + swapchain texture).

### Pipeline system (`src/pipeline/`)

6 graphics pipelines (SimpleColor, Gooch, Textured, Lines, Lambert, Skybox) and 1 compute pipeline (DebugNormals). Each `PipelineDefinition` is a compile-time description of shader stages and bindings. Entities are tagged with `PipelineNameTag<T>` for runtime dispatch.

### Shader pipeline

HLSL sources live in `content/shaders/src/` with includes in `content/shaders/include/`. The build compiles them to SPIR-V, extracts JSON reflection via `spirv-cross`, then generates C++ headers via the `spirv_header_gen` tool (`app/spirv_header_gen/`). These generated headers provide binding metadata used for compile-time static assertions that verify C++ struct layouts match shader expectations.

### Resource loading (`src/io/`)

`ResourceLoader` loads glTF (fastgltf) and OBJ (tinyobjloader) models, textures (SDL3_image), and cubemaps. `SceneLoader` parses JSON scene files (`content/scenes/`) into `SceneDefinition` structs containing `MeshObjectDef`, `SkyboxDef`, and `PointLightsDef` variants.

### ECS (`src/ecs/`)

EnTT-based. `EntityId` wraps an entity handle with component access helpers. `EntityBase` provides name and parent-child relationships. Components include `RenderObject`, `RotateBehavior`, pipeline tags, etc.

### Debug UI (`src/debug/`)

ImGui-based overlays: ECS entity inspector (`ImGuiEcsView`), performance metrics (`ImGuiPerfView`), debug normal visualization toggle.

## Conventions

- **Member variables**: `mVariableName` prefix
- **Classes/structs/enums**: `PascalCase`
- **Functions/namespaces**: `snake_case`
- **SDL wrappers**: `Sdl*` prefix (e.g., `SdlGpuBuffer`)
- **RAII macros**: `NO_COPY`, `NO_MOVE`, `NO_COPY_NO_MOVE`, `NO_COPY_DEFAULT_MOVE` for move/copy control
- **Assertions**: `ABSL_CHECK` / `FAIL(msg)` for runtime invariants
- **Shader data structs** (`src/shader/`): `VertexData`, `MeshData`, `GpuShaderObject` — these must stay layout-compatible with shader bindings