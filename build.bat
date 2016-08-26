REM W:\loltap>emcc w:\loltap\loltap.cpp -o w:\loltap\loltap.html -Iinclude --embed-file chicken_chicken.png --embed-file bg_bg.png --embed-file bg_tower.png --embed-file bg_road.png --embed-file clouds.png --embed-file font.png c:\emscripten\emscripten\1.35.0\system\lib\libSDL2.a
cl loltap.cpp /nologo /Zi /MT /Iinclude lib\sdl2\x64\sdl2.lib /link /subsystem:console
