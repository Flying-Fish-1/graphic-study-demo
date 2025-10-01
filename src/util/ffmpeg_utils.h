#ifndef UTIL_FFMPEG_UTILS_H
#define UTIL_FFMPEG_UTILS_H

#include <string>

namespace Util {
namespace Ffmpeg {

bool locateFfmpeg(std::string& resolvedPath);
bool convertImage(const std::string& ffmpegPath,
                  const std::string& inputPpm,
                  const std::string& outputImage,
                  std::string& errorMessage);
bool encodeVideo(const std::string& ffmpegPath,
                 const std::string& framePattern,
                 const std::string& outputVideo,
                 int fps,
                 std::string& errorMessage);

} // namespace Ffmpeg
} // namespace Util

#endif // UTIL_FFMPEG_UTILS_H
