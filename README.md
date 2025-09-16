Proyecto: Motor de Videojuego 2D con Megamans
Descripción
Este proyecto implementa un motor de videojuego 2D básico utilizando C++ estándar (C++22) y librerías externas como SDL3, SDL3_image, SDL3_ttf, spdlog y nlohmann/json. El objetivo es spawnear múltiples instancias de "Megaman" que se mueven aleatoriamente por la pantalla, rebotan en los bordes y entre sí, con mecánicas adicionales como puntos de vida (HP) y eliminación al chocar. Se basa en las especificaciones de un ejercicio individual, cumpliendo con las Preguntas 0 a 3, y agrega mejoras para mayor interactividad y visualización.
El juego muestra Megamans moviéndose, colisionando (reduciendo HP), y eliminándose hasta que quede un "ganador". Incluye interacción con el mouse para arrastrar un Megaman y texto en pantalla para mostrar HP.
Estructura del Proyecto

external/: Librerías externas (SDL3, SDL3_image, SDL3_ttf, spdlog, nlohmann/json).
assets/: Recursos como data.json (configuración), megaman.png (imagen), y arial.ttf (fuente para texto).
src/: Código fuente (.h y .cpp): main.cpp (lógica principal), file_management.cpp (lectura JSON), colisiones.cpp (colisiones AABB).
bin/: Archivos compilados y ejecutable (main.exe).
Makefile: Para compilar y ejecutar (usa g++ para MinGW/Windows).
.gitignore: Ignora archivos temporales y binarios.

Cumplimiento de Requisitos
El proyecto cumple con las Preguntas especificadas en el ejercicio:

Pregunta 0 (Configuración del Proyecto):

Se creó la estructura de carpetas: external/, assets/, src/, bin/.
El Makefile compila y linkea las librerías requeridas: SDL3, SDL3_image, spdlog, nlohmann/json (y SDL3_ttf para mejoras).
Ejemplo de comando: make para compilar, make run para ejecutar.


Pregunta 1 (Lectura de JSON y Spawnear):

Lee data.json en assets/ usando nlohmann/json para obtener cantidad, ubicacion_imagen, ancho y alto.
Spawnea cantidad Megamans en posiciones aleatorias dentro de la pantalla (no fuera), con velocidades aleatorias en [-300, 300] para X e Y.
Al mover, chequea bordes y multiplica velocidades por -1 si salen (con clamp para evitar atascos).


Pregunta 2 (DeltaTime para Movimiento):

Usa SDL_GetTicksNS() para calcular deltaTime (tiempo entre frames en segundos).
Actualiza posiciones: pos += vel * deltaTime, asegurando movimiento suave e independiente de FPS (guiado por el video proporcionado).


Pregunta 3 (Colisiones - Elegí 3.2: Rectangulares con AABB):

Nueva unidad: colisiones.h/cpp con struct Rectangulo y función ColisionRectangular (implementa AABB: chequea overlap en X/Y).
En cada frame, chequea colisiones entre pares de Megamans; si colisiona, multiplica velocidades por -1 y resuelve overlap separándolos.
Agregué ancho y alto al JSON; centro calculado como pos + half_size.



Cómo Funciona

Compilación y Ejecución:

Requiere MinGW (g++) y librerías en external/.
Ejecuta make para compilar (genera objetos en bin/ y linkea a main.exe).
make run para lanzar el juego.
Presiona ESC o cierra la ventana para salir.


Flujo del Juego:

Inicializa SDL3, ventana fullscreen (800x600), renderer y fuente TTF.
Lee data.json: Si falla, loguea error y sale (usa std::optional para manejo seguro).
Carga imagen de Megaman y spawnea instancias (evita superposiciones iniciales).
Bucle principal:

Maneja eventos: Quit/ESC, mouse (click para seleccionar y arrastrar un Megaman).
Actualiza posiciones con deltaTime (salta si arrastrando).
Chequea bordes y colisiones: Reduce HP, rebota, separa.
Elimina Megamans con HP <=0.
Si <=1 restante, loguea ganador y cierra.


Render: Limpia pantalla, dibuja Megamans, y texto "HP: X" arriba de cada uno (usando SDL_ttf).
Limpieza: Libera recursos al final.


Gestión de Errores:

Usa spdlog para logs.
std::optional en lectura JSON: Devuelve nullopt en fallos, chequeado en main para salida limpia.
Chequeos en cargas (imagen, fuente) con salida temprana.



Mejoras Implementadas
Más allá de los requisitos base, agregué funcionalidades para mejorar la jugabilidad y visualización:

Puntos de Vida (HP): Cada Megaman inicia con 10 HP. Pierde 1 por colisión. A 0, se elimina. Termina cuando quede un ganador (logueado).
Texto en Pantalla: Muestra "HP: X" arriba de cada Megaman (usando SDL3_ttf y fuente TTF).
Interacción con Mouse: Click para seleccionar y arrastrar un Megaman (pausa su movimiento automático).
Anti-Superposición Inicial: Al spawnear, chequea colisiones para posiciones válidas (hasta 100 intentos).
Resolución de Colisiones: Calcula overlap y separa objetos para evitar "pegado" al chocar.
Velocidad Aumentada: Rango [-300, 300] para movimiento más dinámico (original era lento).
Tamaño Ajustable: Megamans más pequeños (configurable en JSON, ej. 60x68).

Estas mejoras hacen el juego más interactivo y visual, sin alterar los requisitos core.
Dependencias

SDL3, SDL3_image, SDL3_ttf (gráficos, imágenes, texto).
spdlog (logs).
nlohmann/json (JSON).
Configuradas en Makefile con -I y -L.

Para más detalles, revisa el código fuente o contacta al desarrollador.
