//Cabezeras:

#include "file_management.h" // Incluye la función ReadFromConfigFile
#include "colisiones.h" // Incluye la función ColisionRectangular
#include <string>   // std::string
#include <spdlog/spdlog.h>  // para logging
#include <vector> // std::vector
#include <random> // Generación de números aleatorios
#include <SDL3/SDL.h> // SDL principal
#include <SDL3_image/SDL_image.h> // SDL_image para cargar imágenes
#include <SDL3_ttf/SDL_ttf.h>   // SDL_ttf para renderizar texto

// Estructura para representar un Megaman
struct Megaman
{
    float posX;
    float posY;
    float velocidadX;
    float velocidadY;
    SDL_Texture* textura;
    int hp;
};

// Función para spawnear Megamans en posiciones aleatorias sin sobreponerse
std::vector<Megaman> SpawnearMegamans(int cantidad, SDL_Texture* textura, int windowW, int windowH, float displayW, float displayH)
{
    std::vector<Megaman> megamans; // Vector para almacenar los Megamans
    std::random_device rd; // Semilla para el generador de números aleatorios
    std::mt19937 gen(rd()); // Generador Mersenne Twister
    std::uniform_real_distribution<float> disX(0.f, static_cast<float>(windowW) - displayW); // Distribución para la posición X
    std::uniform_real_distribution<float> disY(0.f, static_cast<float>(windowH) - displayH); // Distribución para la posición Y
    std::uniform_real_distribution<float> disVel(-300.f, 300.f); // Distribución para la velocidad

    for (int i = 0; i < cantidad; ++i) // Iterar para crear cada Megaman
    {
        bool posicionValida = false; // Bandera para verificar si la posición es válida
        float newX, newY;  // Nuevas coordenadas para el Megaman
        int intentos = 0; // Contador de intentos para encontrar una posición válida
        const int maxIntentos = 100; // Máximo de intentos para evitar bucles infinitos

        while (!posicionValida && intentos < maxIntentos) // Intentar hasta encontrar una posición válida o alcanzar el máximo de intentos
        {
            newX = disX(gen); // Generar nueva posición X
            newY = disY(gen); // Generar nueva posición Y

            Rectangulo newRect{{displayW, displayH}, {newX + displayW / 2, newY + displayH / 2}}; // Nuevo rectángulo del Megaman
            posicionValida = true; // Asumir que la posición es válida

            for (const auto& m : megamans) // Verificar colisiones con Megamans existentes (importante usamos referencia para evitar copia)
            {
                Rectangulo existingRect{{displayW, displayH}, {m.posX + displayW / 2, m.posY + displayH / 2}}; // Rectángulo del Megaman existente
                if (ColisionRectangular(newRect, existingRect)) // Si hay colisión, la posición no es válida
                {
                    posicionValida = false; // Marcar posición como no válida
                    break; // Salir del bucle ya que no es necesario seguir verificando
                }
            }
            intentos++; // Incrementar el contador de intentos
        }

        if (!posicionValida) // Si no se encontró una posición válida después de varios intentos, usar una posición por defecto
        {
            spdlog::warn("No se pudo encontrar posición válida para Megaman después de {} intentos. Usando posición default.", maxIntentos);
            newX = static_cast<float>(i * displayW); // Colocar en línea horizontal por defecto
            newY = 0.f; // Colocar en la parte superior por defecto
        }

        Megaman m{// Crear el Megaman con posición, velocidad y textura
            newX,
            newY,
            disVel(gen),
            disVel(gen),
            textura,
            10
        };
        megamans.push_back(m); // Agregar el Megaman al vector
    }
    return megamans;// Retornar el vector de Megamans
}


int megamanSeleccionado = -1; // Índice del Megaman seleccionado (-1 si ninguno) porque no hay selección
float offsetX = 0, offsetY = 0; // Offset del mouse respecto al Megaman seleccionado

int main() // Función principal
{
    //*Los siguientes bloques de código inicializan SDL, crean la ventana y el renderer, y manejan errores. *//

    if (SDL_Init(SDL_INIT_VIDEO) < 0) // Inicializar SDL, menor a 0 es error porque devuelve -1 y eso es falso
    {
        spdlog::error("No se pudo inicializar SDL: {}", SDL_GetError());
        return -1;
    }

    if (TTF_Init() < 0)  // Inicializar SDL_ttf, menor a 0 es menor a 0 es error porque devuelve -1 y eso es falso
    {
        spdlog::error("No se pudo inicializar SDL_ttf: {}", SDL_GetError());
        SDL_Quit();// Limpiar SDL antes de salir
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow( // Crear ventana
        "Ejercicio individual 1",
        800,
        600,
        SDL_WINDOW_FULLSCREEN
    );

    if (window == nullptr) // Si la ventana no se creó correctamente
    {
        spdlog::error("No se pudo crear la ventana: {}", SDL_GetError());
        TTF_Quit(); // Limpiar SDL_ttf antes de salir
        SDL_Quit(); // Limpiar SDL antes de salir
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr); // Crear renderer para la ventana

    if (renderer == nullptr) // Si el renderer no se creó correctamente
    {
        spdlog::error("No se pudo crear el renderer: {}", SDL_GetError());
        SDL_DestroyWindow(window);  // Limpiar la ventana antes de salir
        TTF_Quit(); // Limpiar SDL_ttf antes de salir
        SDL_Quit(); // Limpiar SDL antes de salir
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("./assets/arial.ttf", 16);  // Cargar fuente para renderizar texto
    if (font == nullptr) // Si la fuente no se cargó correctamente
    {
        spdlog::error("No se pudo cargar la fuente: {}", SDL_GetError());
        // aquí se puede continuar sin renderizar texto
    }

    int windowW, windowH; // Dimensiones de la ventana
    SDL_GetWindowSize(window, &windowW, &windowH); // Obtener dimensiones de la ventana

    auto config = ReadFromConfigFile("./assets/data.json"); // Leer configuración desde el archivo JSON

    if (!config.has_value())  // Chequea si hay valor 
    {
        spdlog::error("Falló la lectura de la configuración JSON.");
        if (font) TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    auto [cantidad, ubicacion, ancho, alto] = *config;  // Desempaqueta el valor (usa * para acceder)

    if (cantidad <= 0 || ubicacion.empty())  // Chequeo adicional por valores inválidos
    {
        spdlog::error("Configuración inválida del archivo JSON (valores no válidos).");
        if (font) TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Surface* surface = IMG_Load(ubicacion.c_str()); // Cargar imagen desde la ubicación especificada con .c_str() para convertir std::string a const char*
    if (surface == nullptr) // Si la imagen no se cargó correctamente
    {
        spdlog::error("No se pudo cargar la imagen: {}", SDL_GetError());
        if (font) TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // Crear textura desde la superficie
    SDL_DestroySurface(surface); // Liberar la superficie ya que no se necesita más

    if (texture == nullptr) // Si la textura no se creó correctamente
    {
        spdlog::error("No se pudo crear la textura: {}", SDL_GetError());
        if (font) TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    std::vector<Megaman> megamans = SpawnearMegamans(cantidad, texture, windowW, windowH, ancho, alto); // Spawnear Megamans

    bool isRunning = true; // Bandera para el bucle principal
    uint64_t lastTime = SDL_GetTicksNS(); // Tiempo del último frame con SDL_GetTicksNS que devuelve tiempo en nanosegundos, y uint64_t es un entero sin signo de 64 bits

    while (isRunning) // Bucle principal
    {
        SDL_Event event; // Evento de SDL para manejar entradas 
        while (SDL_PollEvent(&event)) { // SDL_PollEvent devuelve 1 si hay un evento, 0 si no hay más eventos
            switch (event.type) { // Manejar diferentes tipos de eventos
                case SDL_EVENT_QUIT: // Evento de cerrar la ventana
                    isRunning = false;
                    break;
                case SDL_EVENT_KEY_DOWN: // Evento de tecla presionada
                    if (event.key.key == SDLK_ESCAPE) // Si la tecla es ESCAPE
                        isRunning = false; // Salir del bucle principal
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN: { // Evento de botón del mouse presionado
                    float mouseX = event.button.x; // Coordenada X del mouse
                    float mouseY = event.button.y; // Coordenada Y del mouse
                    for (size_t i = 0; i < megamans.size(); ++i) { // Iterar sobre los Megamans
                        const auto& m = megamans[i]; // Referencia al Megaman actual
                        if (mouseX >= m.posX && mouseX <= m.posX + ancho &&
                            mouseY >= m.posY && mouseY <= m.posY + alto) { // Si el mouse está dentro del área del Megaman
                            megamanSeleccionado = static_cast<int>(i); // Seleccionar el Megaman y guardar el índice
                            offsetX = mouseX - m.posX; // Calcular offset X del mouse respecto al Megaman
                            offsetY = mouseY - m.posY; // Calcular offset Y del mouse respecto al Megaman
                            break; // Salir del bucle ya que se encontró el Megaman seleccionado
                        }
                    }
                    break;// Cerrar el case SDL_EVENT_MOUSE_BUTTON_DOWN
                }
                case SDL_EVENT_MOUSE_BUTTON_UP: // Evento de botón del mouse liberado
                    megamanSeleccionado = -1; // Deseleccionar el Megaman
                    break; // Cerrar el case SDL_EVENT_MOUSE_BUTTON_UP
                case SDL_EVENT_MOUSE_MOTION: // Evento de movimiento del mouse
                    if (megamanSeleccionado != -1) { // Si hay un Megaman seleccionado
                        float mouseX = event.motion.x; // Coordenada X del mouse
                        float mouseY = event.motion.y; // Coordenada Y del mouse
                        megamans[megamanSeleccionado].posX = mouseX - offsetX; // Actualizar posición X del Megaman seleccionado con offset
                        megamans[megamanSeleccionado].posY = mouseY - offsetY; // Actualizar posición Y del Megaman seleccionado con offset
                    }
                    break; // Cerrar el case SDL_EVENT_MOUSE_MOTION
            }
        }

        uint64_t now = SDL_GetTicksNS(); // Tiempo actual en nanosegundos
        float deltaTime = (now - lastTime) / 1000000000.0f; // Calcular deltaTime en segundos
        lastTime = now; // Actualizar lastTime para el próximo frame

        // Actualizar posiciones y chequear bordes 
        
        for (size_t i = 0; i < megamans.size(); ++i) // Iterar sobre los Megamans
        {
            if (static_cast<int>(i) == megamanSeleccionado) continue; // Si es el Megaman seleccionado, no actualizar su posición

            megamans[i].posX += megamans[i].velocidadX * deltaTime; // Actualizar posición X
            megamans[i].posY += megamans[i].velocidadY * deltaTime; // Actualizar posición Y

            if (megamans[i].posX < 0 || megamans[i].posX + ancho > windowW || megamans[i].posY < 0 || megamans[i].posY + alto > windowH) // Chequear colisiones con los bordes de la ventana
            {
                megamans[i].velocidadX *= -1; // Invertir velocidad X
                megamans[i].velocidadY *= -1; // Invertir velocidad Y

                if (megamans[i].posX < 0) megamans[i].posX = 0; // Ajustar posición X si está fuera del borde izquierdo
                if (megamans[i].posX + ancho > windowW) megamans[i].posX = windowW - ancho; // Ajustar posición X si está fuera del borde derecho
                if (megamans[i].posY < 0) megamans[i].posY = 0; // Ajustar posición Y si está fuera del borde superior
                if (megamans[i].posY + alto > windowH) megamans[i].posY = windowH - alto; // Ajustar posición Y si está fuera del borde inferior
            }
        }

        // Chequear colisiones

        std::vector<size_t> toRemove; // Vector para almacenar índices de Megamans a eliminar
        for (size_t i = 0; i < megamans.size(); ++i) // Iterar sobre los Megamans
        {
            for (size_t j = i + 1; j < megamans.size(); ++j) // Iterar sobre los Megamans restantes para evitar duplicados
            {
                Rectangulo r1{{ancho, alto}, {megamans[i].posX + ancho / 2, megamans[i].posY + alto / 2}}; // Rectángulo del Megaman i
                Rectangulo r2{{ancho, alto}, {megamans[j].posX + ancho / 2, megamans[j].posY + alto / 2}}; // Rectángulo del Megaman j

                if (ColisionRectangular(r1, r2)) // Si hay colisión entre los dos Megamans
                {
                    megamans[i].hp -= 1; // Reducir HP de ambos Megamans
                    megamans[j].hp -= 1; 

                    megamans[i].velocidadX *= -1; // Invertir velocidades para simular rebote
                    megamans[i].velocidadY *= -1;
                    megamans[j].velocidadX *= -1;
                    megamans[j].velocidadY *= -1;

                    float overlapX = (ancho / 2 + ancho / 2) - std::abs(megamans[i].posX - megamans[j].posX); // Calcular sobreposicionamiento en X
                    float overlapY = (alto / 2 + alto / 2) - std::abs(megamans[i].posY - megamans[j].posY); // Calcular sobreposicionamiento en Y

                    if (overlapX > 0 && overlapY > 0) // Si hay sobreposicionamiento, separar los Megamans
                    {
                        float signX = (megamans[i].posX < megamans[j].posX) ? -1.0f : 1.0f; // Determinar dirección de separación en X
                        float signY = (megamans[i].posY < megamans[j].posY) ? -1.0f : 1.0f; // Determinar dirección de separación en Y

                        float separationX = (overlapX / 2.0f + 0.1f) * signX; // Calcular separación en X con un pequeño margen
                        float separationY = (overlapY / 2.0f + 0.1f) * signY; // Calcular separación en Y con un pequeño margen

                        megamans[i].posX += separationX; // Separar Megaman i
                        megamans[i].posY += separationY;
                        megamans[j].posX -= separationX; // Separar Megaman j
                        megamans[j].posY -= separationY;
                    }

                    if (megamans[i].hp <= 0) toRemove.push_back(i); // Si el HP de un Megaman llega a 0, marcarlo para eliminar
                    if (megamans[j].hp <= 0) toRemove.push_back(j); 
                }
            }
        }

        std::sort(toRemove.begin(), toRemove.end(), std::greater<size_t>()); // Ordenar índices en orden descendente para eliminar sin afectar los índices restantes
        for (auto idx : toRemove) // Eliminar Megamans marcados
        {
            if (idx < megamans.size()) // Verificar que el índice es válido
            {
                megamans.erase(megamans.begin() + idx); // Eliminar el Megaman del vector
            }
        }

        if (megamans.size() <= 1) // Si queda 0 o 1 Megaman, terminar el juego
        {
            if (megamans.empty()) // Si no quedan Megamans
            {
                spdlog::info("¡Todos los Megamans han sido eliminados! No hay ganador.");
            }
            else // Si queda un solo Megaman
            {
                spdlog::info("¡Hay un ganador! El ultimo Megaman restante.");
            }
            isRunning = false; // Salir del bucle principal
        }

        // Renderizar
        SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255); // Color de fondo oscuro
        SDL_RenderClear(renderer); // Limpiar el renderer con el color de fondo

        for (const auto& m : megamans) // Renderizar cada Megaman
        {
            SDL_FRect rectMegaman{ // Rectángulo para renderizar el Megaman 
                m.posX,
                m.posY,
                ancho,
                alto
            };
            SDL_RenderTexture(renderer, m.textura, nullptr, &rectMegaman); // Renderizar la textura del Megaman

            // Renderizar HP si la fuente cargó
            if (font) // Verificar si la fuente se cargó correctamente
            {
                std::string hpText = "HP: " + std::to_string(m.hp); // Texto del HP
                SDL_Color color = {255, 255, 255, 255};  // Blanco
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, hpText.c_str(), hpText.length(), color); // Crear superficie con el texto
                if (textSurface) // Si la superficie se creó correctamente
                {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface); // Crear textura desde la superficie
                    if (textTexture) // Si la textura se creó correctamente
                    {
                        int textW = textSurface->w; // Ancho del texto
                        int textH = textSurface->h; // Alto del texto
                        SDL_FRect textRect{ // Rectángulo para renderizar el texto
                            m.posX + (ancho / 2 - textW / 2),  // Centrado arriba
                            m.posY - textH - 5,  // Arriba del Megaman
                            static_cast<float>(textW), // Ancho del texto
                            static_cast<float>(textH) // Alto del texto
                        };
                        SDL_RenderTexture(renderer, textTexture, nullptr, &textRect); // Renderizar la textura del texto
                        SDL_DestroyTexture(textTexture); // Liberar la textura del texto
                    }
                    SDL_DestroySurface(textSurface); // Liberar la superficie del texto
                }
            }
        }

        SDL_RenderPresent(renderer); // Presentar el contenido renderizado en la ventana
    }

    // Limpiar y salir
    if (font) TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}