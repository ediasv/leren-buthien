#pragma once

#include <filesystem>

namespace Recursos {

std::filesystem::path resolverCaminhoRecurso(
    const std::filesystem::path& caminhoRelativo);

}  // namespace Recursos
