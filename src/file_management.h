#pragma once

#include <tuple>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include <optional>  

std::optional<std::tuple<int, std::string, float, float>> ReadFromConfigFile(const std::string& file);