# GameEngine

A game engine i've been making for the past few months.

My main goal with this project is to learn more about C / C++ and how game engines work in detail.

Todo: 
- Add support for calling dvar functions instead of only being able to change dvar values
- Script VM is mostly complete, need to add starting threads from a script
- Refactor OpenGL backend to make it simpler to read / use
- Add an index buffer and shadows to OpenGL
- Improve 2D rendering to make it less buggy and nicer to look at
- Add support for higher resolutions and changing / moving the window
- Improve physics to not slow you down at the edge of blocks
- Improve the model builder to add support for making both objects and worlds that aren't made of cubes

Completed:
- Basic scripting VM with threads and builtin calls (similar to LUA).
- C# Script compiler (Essentially typeless C -> opcodes)
- Development variables + console
- Asset loading / finding
- Input system
- Basic physics
- Basic world builder
