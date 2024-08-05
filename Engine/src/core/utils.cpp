#include "utils.hpp"

namespace utils
{
    std::string GetFileName(const std::string& aFilePath) {
        size_t Pos = aFilePath.find_last_of("/\\");
        if (Pos == std::string::npos) {
            return aFilePath;
        } else {
            return aFilePath.substr(Pos + 1);
        }
    }
}
