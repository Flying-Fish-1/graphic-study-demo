#include "depth_buffer.h"
#include <cstring>
#include <limits>

namespace Renderer {
namespace Pipeline {

DepthBuffer::DepthBuffer(int width, int height) 
    : m_width(width), m_height(height), m_enabled(true) {
    m_buffer = new float[width * height];
    clear();
}

DepthBuffer::~DepthBuffer() {
    delete[] m_buffer;
}

void DepthBuffer::clear(float depth) {
    for (int i = 0; i < m_width * m_height; i++) {
        m_buffer[i] = depth;
    }
}

bool DepthBuffer::testAndSet(int x, int y, float depth) {
    if (!m_enabled || !isValidCoordinate(x, y)) {
        return true; // 如果深度测试关闭或坐标无效，总是通过
    }
    
    int index = y * m_width + x;
    
    // 深度测试：如果新深度值更近（更小），则通过测试
    if (depth < m_buffer[index]) {
        m_buffer[index] = depth;
        return true;
    }
    
    return false;
}

bool DepthBuffer::test(int x, int y, float depth) const {
    if (!m_enabled || !isValidCoordinate(x, y)) {
        return true;
    }
    
    int index = y * m_width + x;
    return depth < m_buffer[index];
}

float DepthBuffer::getDepth(int x, int y) const {
    if (!isValidCoordinate(x, y)) {
        return std::numeric_limits<float>::max();
    }
    
    return m_buffer[y * m_width + x];
}

void DepthBuffer::setDepth(int x, int y, float depth) {
    if (isValidCoordinate(x, y)) {
        m_buffer[y * m_width + x] = depth;
    }
}

void DepthBuffer::resize(int width, int height) {
    if (width != m_width || height != m_height) {
        delete[] m_buffer;
        
        m_width = width;
        m_height = height;
        m_buffer = new float[width * height];
        
        clear();
    }
}

bool DepthBuffer::isValidCoordinate(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

} // namespace Pipeline
} // namespace Renderer
