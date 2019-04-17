#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

bool readFile(const std::filesystem::path& path, std::string& content);