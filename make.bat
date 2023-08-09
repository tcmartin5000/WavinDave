:: Build the music driver, change the paths here to wherever the requisite hUGEDriver files are located.
rgbasm -DGBDK -o .\out\hUGEDriver.obj -i .\hUGEDriver .\hUGEDriver\hUGEDriver.asm
python3 .\hUGEDriver\tools\rgb2sdas.py -o .\out\hUGEDriver.o .\out\hUGEDriver.obj
sdar -ru ..\lib\gb\hUGEDriver.lib .\out\hUGEDriver.o

:: Build the actual game.
lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c BackgroundTileMap.c BackgroundTiles1.c Sprite1.c main.c dave_bgm.c
lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG BackgroundTileMap.o BackgroundTiles1.o Sprite1.o main.o dave_bgm.o out\hUGEDriver.o -o .\out\first_proj.gb
del *.o *.asm *.sym *.lst *.noi *.map *.ihx .\out\*.o