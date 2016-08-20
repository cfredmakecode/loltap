REM W:\loltap>emcc w:\loltap\loltap.cpp -o w:\loltap\loltap.html -Iinclude --embed-file bg.bmp --embed-file clouds.png c:\emscripten\emscripten\1.35.0\system\lib\libSDL2.a
cl loltap.cpp /nologo /Zi /MT /Iinclude lib\sdl2\x64\sdl2.lib /link /subsystem:console
