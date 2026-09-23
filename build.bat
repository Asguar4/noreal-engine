set io=src\engine\io\io.cpp
set render=src\engine\render\render.cpp src\engine\render\iRender.cpp
set files=src\main.cpp %io% %render%
set libs=C:\Work\game-engine\lib\glew32s.lib C:\Work\game-engine\lib\glfw3_mt.lib opengl32.lib user32.lib gdi32.lib shell32.lib

CL /Zi /EHsc /MT /DGLEW_STATIC /I C:\Work\game-engine\include %files% /link %libs% /OUT:game.exe
