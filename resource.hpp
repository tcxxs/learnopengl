#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include "pattern.hpp"
#include "model.hpp"

bool readFile(const std::filesystem::path& path, std::string& content);

using MeshVec = std::vector<Mesh>;
using MeshMgr = Singleton<MeshVec>;