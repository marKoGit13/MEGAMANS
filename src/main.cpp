#include "file_management.h"
#include "colisiones.h"
#include <string>
#include <spdlog/spdlog.h>
#include <vector>
#include <random>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

struct Megaman
{
    float posX;
    float posY;
    float velocidadX;
    float velocidadY;
    SDL_Texture* textura;
    int hp;
};

std::vector<Megaman> SpawnearMegamans(int cantidad, SDL_Texture* textura, int windowW, int windowH, float displayW, float displayH)
{
    std::vector<Megaman> megamans;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(0.f, static_cast<float>(windowW) - displayW);
    std::uniform_real_distribution<float> disY(0.f, static_cast<float>(windowH) - displayH);
    std::uniform_real_distribution<float> disVel(-300.f, 300.f);

    for (int i = 0; i < cantidad; ++i)
    {
        bool posicionValida = false;
        float newX, newY;
        int intentos = 0;
        const int maxIntentos = 100;

        while (!posicionValida && intentos < maxIntentos)
        {
            newX = disX(gen);
            newY = disY(gen);

            Rectangulo newRect{{displayW, displayH}, {newX + displayW / 2, newY + displayH / 2}};
            posicionValida = true;

            for (const auto& m : megamans)
            {
                Rectangulo existingRect{{displayW, displayH}, {m.posX + displayW / 2, m.posY + displayH / 2}};
                if (ColisionRectangular(newRect, existingRect))
                {
                    posicionValida = false;
                    break;
                }
            }
            intentos++;
        }

        if (!posicionValida)
        {
            spdlog::warn("No se pudo encontrar posición válida para Megaman después de {} intentos. Usando posición default.", maxIntentos);
            newX = static_cast<float>(i * displayW);
            newY = 0.f;
        }

        Megaman m{
            newX,
            newY,
            disVel(gen),
            disVel(gen),
            textura,
            10
        };
        megamans.push_back(m);
    }
    return megamans;
}


int megamanSeleccionado = -1;
float offsetX = 0, offsetY = 0;

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        spdlog::error("No se pudo inicializar SDL: {}", SDL_GetError());
        return -1;
    }

    if (TTF_Init() < 0)  // Inicializar SDL_ttf
    {
        spdlog::error("No se pudo inicializar SDL_ttf: {}", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Ejercicio individual 1",
        800,
        600,
        SDL_WINDOW_FULLSCREEN
    );

    if (window == nullptr)
    {
        spdlog::error("No se pudo crear la ventana: {}", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (renderer == nullptr)
    {
        spdlog::error("No se pudo crear el renderer: {}", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Cargar fuente (ajusta el path y tamaño)
    TTF_Font* font = TTF_OpenFont("./assets/arial.ttf", 16);  // Tamaño 16 px
    if (font == nullptr)
    {
        spdlog::error("No se pudo cargar la fuente: {}", SDL_GetError());
        // Continuar sin texto, pero puedes salir si quieres
    }

    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);

    auto [cantidad, ubicacion, ancho, alto] = ReadFromConfigFile("./assets/data.json");

    if (cantidad <= 0 || ubicacion.empty())
    {
        spdlog::error("Configuración inválida del archivo JSON");
        if (font) TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Surface* surface = IMG_Load(ubicacion.c_str());
    if (surface == nullptr)
    {
        spdlog::error("No se pudo cargar la imagen: {}", SDL_GetError());
        if (font) TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (texture == nullptr)
    {
        spdlog::error("No se pudo crear la textura: {}", SDL_GetError());
        if (font) TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    std::vector<Megaman> megamans = SpawnearMegamans(cantidad, texture, windowW, windowH, ancho, alto);

    bool isRunning = true;
    uint64_t lastTime = SDL_GetTicksNS();

    while (isRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    isRunning = false;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    float mouseX = event.button.x;
                    float mouseY = event.button.y;
                    for (size_t i = 0; i < megamans.size(); ++i) {
                        const auto& m = megamans[i];
                        if (mouseX >= m.posX && mouseX <= m.posX + ancho &&
                            mouseY >= m.posY && mouseY <= m.posY + alto) {
                            megamanSeleccionado = static_cast<int>(i);
                            offsetX = mouseX - m.posX;
                            offsetY = mouseY - m.posY;
                            break;
                        }
                    }
                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    megamanSeleccionado = -1;
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    if (megamanSeleccionado != -1) {
                        float mouseX = event.motion.x;
                        float mouseY = event.motion.y;
                        megamans[megamanSeleccionado].posX = mouseX - offsetX;
                        megamans[megamanSeleccionado].posY = mouseY - offsetY;
                    }
                    break;
            }
        }

        uint64_t now = SDL_GetTicksNS();
        float deltaTime = (now - lastTime) / 1000000000.0f;
        lastTime = now;

        // Actualizar posiciones y chequear bordes
        
        for (size_t i = 0; i < megamans.size(); ++i)
        {
            if (static_cast<int>(i) == megamanSeleccionado) continue; // No mover el seleccionado

            megamans[i].posX += megamans[i].velocidadX * deltaTime;
            megamans[i].posY += megamans[i].velocidadY * deltaTime;

            if (megamans[i].posX < 0 || megamans[i].posX + ancho > windowW || megamans[i].posY < 0 || megamans[i].posY + alto > windowH)
            {
                megamans[i].velocidadX *= -1;
                megamans[i].velocidadY *= -1;

                if (megamans[i].posX < 0) megamans[i].posX = 0;
                if (megamans[i].posX + ancho > windowW) megamans[i].posX = windowW - ancho;
                if (megamans[i].posY < 0) megamans[i].posY = 0;
                if (megamans[i].posY + alto > windowH) megamans[i].posY = windowH - alto;
            }
        }

        // Chequear colisiones
        std::vector<size_t> toRemove;
        for (size_t i = 0; i < megamans.size(); ++i)
        {
            for (size_t j = i + 1; j < megamans.size(); ++j)
            {
                Rectangulo r1{{ancho, alto}, {megamans[i].posX + ancho / 2, megamans[i].posY + alto / 2}};
                Rectangulo r2{{ancho, alto}, {megamans[j].posX + ancho / 2, megamans[j].posY + alto / 2}};

                if (ColisionRectangular(r1, r2))
                {
                    megamans[i].hp -= 1;
                    megamans[j].hp -= 1;

                    megamans[i].velocidadX *= -1;
                    megamans[i].velocidadY *= -1;
                    megamans[j].velocidadX *= -1;
                    megamans[j].velocidadY *= -1;

                    float overlapX = (ancho / 2 + ancho / 2) - std::abs(megamans[i].posX - megamans[j].posX);
                    float overlapY = (alto / 2 + alto / 2) - std::abs(megamans[i].posY - megamans[j].posY);

                    if (overlapX > 0 && overlapY > 0)
                    {
                        float signX = (megamans[i].posX < megamans[j].posX) ? -1.0f : 1.0f;
                        float signY = (megamans[i].posY < megamans[j].posY) ? -1.0f : 1.0f;

                        float separationX = (overlapX / 2.0f + 0.1f) * signX;
                        float separationY = (overlapY / 2.0f + 0.1f) * signY;

                        megamans[i].posX += separationX;
                        megamans[i].posY += separationY;
                        megamans[j].posX -= separationX;
                        megamans[j].posY -= separationY;
                    }

                    if (megamans[i].hp <= 0) toRemove.push_back(i);
                    if (megamans[j].hp <= 0) toRemove.push_back(j);
                }
            }
        }

        std::sort(toRemove.begin(), toRemove.end(), std::greater<size_t>());
        for (auto idx : toRemove)
        {
            if (idx < megamans.size())
            {
                megamans.erase(megamans.begin() + idx);
            }
        }

        if (megamans.size() <= 1)
        {
            if (megamans.empty())
            {
                spdlog::info("¡Todos los Megamans han sido eliminados! No hay ganador.");
            }
            else
            {
                spdlog::info("¡Hay un ganador! El ultimo Megaman restante.");
            }
            isRunning = false;
        }

        // Renderizar
        SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
        SDL_RenderClear(renderer);

        for (const auto& m : megamans)
        {
            SDL_FRect rectMegaman{
                m.posX,
                m.posY,
                ancho,
                alto
            };
            SDL_RenderTexture(renderer, m.textura, nullptr, &rectMegaman);

            // Renderizar HP si la fuente cargó
            if (font)
            {
                std::string hpText = "HP: " + std::to_string(m.hp);
                SDL_Color color = {255, 255, 255, 255};  // Blanco
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, hpText.c_str(), hpText.length(), color);
                if (textSurface)
                {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture)
                    {
                        int textW = textSurface->w;
                        int textH = textSurface->h;
                        SDL_FRect textRect{
                            m.posX + (ancho / 2 - textW / 2),  // Centrado arriba
                            m.posY - textH - 5,  // Arriba del Megaman
                            static_cast<float>(textW),
                            static_cast<float>(textH)
                        };
                        SDL_RenderTexture(renderer, textTexture, nullptr, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_DestroySurface(textSurface);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    if (font) TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}