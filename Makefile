CC=cl
CFLAGS=/FC /LD
INCLUDES=/Iinclude^\
SRC=src\*.c src\*.cpp
OBJ=/Fobuild^\
OUT=build\nostale_packet_publisher.dll

!IF "$(DEBUG)"=="yes"
NT_PUB_DEBUG=/DNT_PUB_DEBUG
!ENDIF

!IF "$(LOG)"=="yes"
NT_PUB_LOG=/DNT_PUB_LOG
!ENDIF

build: {\build}nostale_packet_publisher.dll

clean:
  del build\*.dll 2>NUL
  del build\*.obj 2>NUL

nostale_packet_publisher.dll: src\*.c src\*.h src\*.cpp
  -@ if NOT EXIST "build" mkdir "build"
  CL $(CFLAGS) $(NT_PUB_DEBUG) $(NT_PUB_LOG) $(INCLUDES) $(OBJ) $(SRC) /link /OUT:$(OUT)