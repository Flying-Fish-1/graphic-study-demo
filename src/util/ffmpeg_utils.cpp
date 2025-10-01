#include "ffmpeg_utils.h"

#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <unistd.h>
#else
#include <unistd.h>
#endif

namespace Util {
namespace Ffmpeg {

namespace {

bool isExecutableFile(const std::filesystem::path& path) {
    std::error_code ec;
    auto status = std::filesystem::status(path, ec);
#ifdef _WIN32
    const bool executable = status.type() == std::filesystem::file_type::regular;
#else
    const bool executable = (status.type() == std::filesystem::file_type::regular) &&
        ((status.permissions() & std::filesystem::perms::owner_exec) != std::filesystem::perms::none ||
         (status.permissions() & std::filesystem::perms::group_exec) != std::filesystem::perms::none ||
         (status.permissions() & std::filesystem::perms::others_exec) != std::filesystem::perms::none);
#endif
    return !ec && executable;
}

std::filesystem::path executableDirectory() {
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
#elif defined(__APPLE__)
    char buffer[1024];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return std::filesystem::path(buffer).parent_path();
    }
    std::vector<char> dynamicBuffer(size);
    if (_NSGetExecutablePath(dynamicBuffer.data(), &size) == 0) {
        return std::filesystem::path(dynamicBuffer.data()).parent_path();
    }
    return std::filesystem::current_path();
#else
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::filesystem::path(buffer).parent_path();
    }
    return std::filesystem::current_path();
#endif
}

std::vector<std::filesystem::path> collectCandidates() {
    std::vector<std::filesystem::path> candidates;
    std::filesystem::path cwd = std::filesystem::current_path();
    candidates.push_back(cwd / "external/ffmpeg/bin/ffmpeg");
    auto exeDir = executableDirectory();
    candidates.push_back(exeDir / "../external/ffmpeg/bin/ffmpeg");
#ifdef _WIN32
    candidates.push_back(cwd / "external/ffmpeg/bin/ffmpeg.exe");
    candidates.push_back(exeDir / "../external/ffmpeg/bin/ffmpeg.exe");
#endif
    const char* ffmpegEnv = std::getenv("FFMPEG_PATH");
    if (ffmpegEnv) {
        candidates.emplace_back(ffmpegEnv);
    }
    const char* pathEnv = std::getenv("PATH");
    if (pathEnv) {
        std::stringstream ss(pathEnv);
        std::string segment;
#ifdef _WIN32
        const char delimiter = ';';
        const char* executableName = "ffmpeg.exe";
#else
        const char delimiter = ':';
        const char* executableName = "ffmpeg";
#endif
        while (std::getline(ss, segment, delimiter)) {
            if (!segment.empty()) {
                candidates.emplace_back(std::filesystem::path(segment) / executableName);
            }
        }
    }
    return candidates;
}

std::string quote(const std::string& value) {
    return '"' + value + '"';
}

} // namespace

bool locateFfmpeg(std::string& resolvedPath) {
    for (const auto& candidate : collectCandidates()) {
        std::error_code ec;
        if (std::filesystem::exists(candidate, ec) && isExecutableFile(candidate)) {
            resolvedPath = candidate.string();
            return true;
        }
    }
    return false;
}

bool convertImage(const std::string& ffmpegPath,
                  const std::string& inputPpm,
                  const std::string& outputImage,
                  std::string& errorMessage) {
    std::ostringstream cmd;
    cmd << quote(ffmpegPath) << " -y -loglevel error -i "
        << quote(inputPpm) << ' ' << quote(outputImage);
    int result = std::system(cmd.str().c_str());
    if (result != 0) {
        errorMessage = "ffmpeg 转换失败，命令: " + cmd.str();
        return false;
    }
    return true;
}

bool encodeVideo(const std::string& ffmpegPath,
                 const std::string& framePattern,
                 const std::string& outputVideo,
                 int fps,
                 std::string& errorMessage) {
    if (fps <= 0) {
        errorMessage = "帧率必须大于0";
        return false;
    }
    std::ostringstream cmd;
    cmd << quote(ffmpegPath)
        << " -y -loglevel error -framerate " << fps
        << " -i " << quote(framePattern)
        << " -c:v libx264 -pix_fmt yuv420p " << quote(outputVideo);
    int result = std::system(cmd.str().c_str());
    if (result != 0) {
        errorMessage = "ffmpeg 视频编码失败，命令: " + cmd.str();
        return false;
    }
    return true;
}

} // namespace Ffmpeg
} // namespace Util
