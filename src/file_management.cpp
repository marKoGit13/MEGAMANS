#include "file_management.h"
#include <spdlog/spdlog.h> 

std::optional<std::tuple<int, std::string, float, float>> ReadFromConfigFile(const std::string &file)
{
    std::ifstream ifs(file); // Abrir el archivo
    if (!ifs.is_open()) // Si no se pudo abrir el archivo
    {
        spdlog::error("No se pudo abrir el archivo: {}", file);
        return std::nullopt; // Retornar sin valor
    }

    nlohmann::json j = nlohmann::json::parse(ifs, nullptr, false); // Parsear el JSON
    if (j.is_discarded()) {   // Si el parseo falló
        spdlog::error("Error al parsear JSON: formato inválido");
        return std::nullopt;
    }

    // Leer los valores con valores por defecto si no existen
    int cantidad = j.value("cantidad", 0);
    std::string ubicacion = j.value("ubicacion_imagen", "");
    float ancho = j.value("ancho", 120.f);
    float alto = j.value("alto", 136.f);

    return std::make_optional(std::make_tuple(cantidad, ubicacion, ancho, alto)); // Retornar los valores empaquetados en una tupla
}