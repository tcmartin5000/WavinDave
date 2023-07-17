lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c BackgroundTileMap.c BackgroundTiles1.c Sprite1.c main.c
lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG BackgroundTileMap.o BackgroundTiles1.o Sprite1.o main.o -o .\out\first_proj.gb
del *.o *.asm *.sym *.lst *.noi *.map *.ihx