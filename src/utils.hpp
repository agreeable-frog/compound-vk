#pragma once

#include <vector>
#include <fstream>

namespace compound {
namespace utils {
    std::vector<char> readFile(const std::string& path) {
        std::ifstream f(path, std::ios::ate | std::ios::binary);
        if (!f.good()) {
            throw std::runtime_error(path + "is invalid");
        }
        size_t size = (size_t)f.tellg();
        std::vector<char> out(size);
        f.seekg(0);
        f.read(out.data(), size);
        return out;
    }
}
}