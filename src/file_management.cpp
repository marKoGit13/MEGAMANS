#include "file_management.h"
#include <spdlog/spdlog.h>

std::tuple<int, std::string, float, float> ReadFromConfigFile(const std::string &file)
{
    std::ifstream ifs(file);
    if (!ifs.is_open())
    {
        spdlog::error("No se pudo abrir el archivo: {}", file);
        return {0, "", 120.f, 136.f};
    }

    nlohmann::json j;
    try
    {
        ifs >> j;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Error al parsear JSON: {}", e.what());
        return {0, "", 120.f, 136.f};
    }

    int cantidad = j.value("cantidad", 0);
    std::string ubicacion = j.value("ubicacion_imagen", "");
    float ancho = j.value("ancho", 120.f);
    float alto = j.value("alto", 136.f);

    return {cantidad, ubicacion, ancho, alto};
}