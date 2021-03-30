# Shader obliczeniowy
## Co
Shadery obliczeniowe to, podobnie jak reszta shaderow, programy działające na karcie graficznej. Odróżniają się one jednak brakiem udziału w procesie rendorowania - sa one wykonywane w dowolnym momencie i służą, jak nazwa sugeruje, do przeprowadzania obliczeń na karcie graficznej. Zaleta tego podejścia w porównaniu do obliczania na procesorze jest duże zwiększenie prędkości dla algorytmów, które pozwalają na wielowątkowość wykonania.

## Jak
W celu wykorzystania shadera obliczeniowego w OpenGL będziemy potrzebować minimum wersji 4.3. 

Wpierw musimy zaladowac nasz shader

```cpp
unsigned int computeShader = glCreateShader( GL_COMPUTE_SHADER );
// shaderLoadSource to funkcja pomocnicza do odczytywania teksu z pliku
char* fragmentShaderSource = shaderLoadSource( filePath );
if ( !fragmentShaderSource )
    return -1;

glShaderSource( computeShader, 1, ( const char** )&fragmentShaderSource, NULL );
glCompileShader( computeShader );

unsigned int shaderProgram = glCreateProgram();
glAttachShader( shaderProgram, computeShader );
glLinkProgram( shaderProgram );
```

Wazne, aby przy tworzeniu jako typ podac `GL_COMPUTE_SHADER` oraz aby nasz program zawieral tylko jeden shader.
Wymagane jest takze, aby nasz shader definiowal zmienna `layout( local_size_x = X,  local_size_y = Y, local_size_z = Z )   in;`. Wiecej o tym pozniej.

Aby uruchomic nasz shader, wykorzystujemy funkcje `glDispatchCompute( x, y, z )`. O parametrach tej funkcji pozniej.

## Dostarczanie i odbieranie danych

Mozna to osiagnac na wiele sposobow, jednym z nich sa SSBO (Shader Storage Buffer Object). 

```cpp
unsigned int buffer;
glGenBuffers( 1, &buffer );
glBindBuffer( GL_SHADER_STORAGE_BUFFER, buffer );
glBufferData( GL_SHADER_STORAGE_BUFFER, SIZE, positions, GL_STREAM_DRAW );

```
Tak zdefiniowany buffor jest uzywany w bardzo podobny sposob jak inne buffory, o ktorych sie uczylismy, jednakze pozwala nam na wieksza dowolnosc pod wzgledem przechowywanych danych (np listy o niesprecyzowanej dlugosci).

Uzycie takiego bufora w shaderze wyglada w sposób nastepujacy:
```cpp
layout (std430, binding=1) buffer BUFFOR_NAME{
    TYPE NAME[];
};
```
Musimy takze oznaczyc, ze obiekt buffora w cpp powinien byc przypisany pod konkretny buffor w shaderze.
```cpp
glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, buffer );
```

Funkcja ta dziala podobnie jak `localtion` i `glVerterAtribbPointer`, ktorych uzywalismy przy "tradycyjnych shaderach"

## Bariery pamieci

Zazwyczaj chcemy takze "poczekac", az nasz shader zakonczy pracowac zanim pojdziemy dalej w naszym programie. Mozemy zastosowac do tego funkcje `glMemoryBarrier`. Bedzie ona czekala, az typ bufora, ktory przekazujemy jak pierwszy argument, przestanie byc uzywany przez karte graficzna, co czesto jest rowne zakonczeniu pracy shadera.

Podobna operacje mozemy takze wykonac w samym shaderze poprzez funkcje `barrier`, ktora sprawi ze dane wykonanie shadera poczeka, az reszta wykonan takze dojdzie do tego momentu. 

## Grupy globalne i lokalne

Przy wywolywaniu funkcji `glDispatchCompute( x, y, z )` oraz przy definiowany zmiennej `layout( local_size_x = X,  local_size_y = Y, local_size_z = Z )   in;` wymagane jest podanie liczb odpowiednio grup i grup lokalnych. Opisuja one jak dane zadanie powinno byc podzielone na karcie graficznej (optymalne ulozenie zalezy od architektury karty). Sprawdznie roznych kombinacji moze znacznie przyspieszych dzialanie naszego shadera. Trzeba jednak pamietac, ze `ILOSC_GRUP * ILOSC_GRUP_LOKALNYCH` jest rowna ilosci naszych "watkow" czyli rownoleglych wykonan naszego shadera. 

Czesto przydaje nam sie takze wiedziec w srodku shadera ktora czesc pracy mamy wykonywac. Slozy do tego zmienna `gl_GlobalInvocationID`, ktora mowinam ID pojedynczego wykonania shadera, w ktorym aktualnie sie znajdujemy. Jest to `vec3`i wartosci sa z zakresu `0 .. ILOSC_GRUP.x * ILOSC_GRUP_LOKALNYCH.x`, podobnie dla wyamiarow `y` oraz `z`.