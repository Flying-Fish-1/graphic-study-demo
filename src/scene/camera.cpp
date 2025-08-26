#include "camera.h"
#include <cmath>
#include <algorithm>

namespace Scene {

// ==================== Frustum 实现 ====================

bool Frustum::isPointInside(const Vector3& point) const {
    for (int i = 0; i < 6; i++) {
        float distance = planes[i].x * point.x + planes[i].y * point.y + 
                        planes[i].z * point.z + planes[i].w;
        if (distance < 0) {
            return false;
        }
    }
    return true;
}

bool Frustum::isSphereInside(const Vector3& center, float radius) const {
    for (int i = 0; i < 6; i++) {
        float distance = planes[i].x * center.x + planes[i].y * center.y + 
                        planes[i].z * center.z + planes[i].w;
        if (distance < -radius) {
            return false;
        }
    }
    return true;
}

bool Frustum::isBoxInside(const Vector3& min, const Vector3& max) const {
    for (int i = 0; i < 6; i++) {
        Vector3 positive(
            planes[i].x > 0 ? max.x : min.x,
            planes[i].y > 0 ? max.y : min.y,
            planes[i].z > 0 ? max.z : min.z
        );
        
        float distance = planes[i].x * positive.x + planes[i].y * positive.y + 
                        planes[i].z * positive.z + planes[i].w;
        if (distance < 0) {
            return false;
        }
    }
    return true;
}

// ==================== Camera 实现 ====================

Camera::Camera() 
    : m_position(0, 0, 5), m_target(0, 0, 0), m_up(0, 1, 0),
      m_fov(Constants::PI / 4), m_aspect(16.0f / 9.0f), m_near(0.1f), m_far(100.0f),
      m_viewDirty(true), m_projectionDirty(true), m_frustumDirty(true) {
}

Camera::Camera(const Vector3& position, const Vector3& target, const Vector3& up)
    : m_position(position), m_target(target), m_up(up),
      m_fov(Constants::PI / 4), m_aspect(16.0f / 9.0f), m_near(0.1f), m_far(100.0f),
      m_viewDirty(true), m_projectionDirty(true), m_frustumDirty(true) {
}

void Camera::lookAt(const Vector3& position, const Vector3& target, const Vector3& up) {
    m_position = position;
    m_target = target;
    m_up = up;
    markViewDirty();
}

void Camera::setPerspective(float fov, float aspect, float near, float far) {
    m_fov = fov;
    m_aspect = aspect;
    m_near = near;
    m_far = far;
    markProjectionDirty();
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float near, float far) {
    m_near = near;
    m_far = far;
    markProjectionDirty();
    
    // 设置正交投影矩阵
    m_projectionMatrix = Matrix4::orthographic(left, right, bottom, top, near, far);
    m_projectionDirty = false;
}

const Matrix4& Camera::getViewMatrix() const {
    if (m_viewDirty) {
        updateViewMatrix();
    }
    return m_viewMatrix;
}

const Matrix4& Camera::getProjectionMatrix() const {
    if (m_projectionDirty) {
        updateProjectionMatrix();
    }
    return m_projectionMatrix;
}

const Matrix4& Camera::getViewProjectionMatrix() const {
    if (m_viewDirty || m_projectionDirty) {
        m_viewProjectionMatrix = getProjectionMatrix() * getViewMatrix();
    }
    return m_viewProjectionMatrix;
}

const Frustum& Camera::getFrustum() const {
    if (m_frustumDirty || m_viewDirty || m_projectionDirty) {
        updateFrustum();
    }
    return m_frustum;
}

Vector3 Camera::getForward() const {
    return (m_target - m_position).normalize();
}

Vector3 Camera::getRight() const {
    return getForward().cross(m_up).normalize();
}

Vector3 Camera::getUp() const {
    return getRight().cross(getForward()).normalize();
}

void Camera::move(const Vector3& offset) {
    m_position = m_position + offset;
    m_target = m_target + offset;
    markViewDirty();
}

void Camera::orbit(float yaw, float pitch) {
    Vector3 direction = m_position - m_target;
    float radius = direction.length();
    
    // 转换为球坐标
    float currentYaw = std::atan2(direction.x, direction.z);
    float currentPitch = std::asin(direction.y / radius);
    
    // 应用旋转
    currentYaw += yaw;
    currentPitch += pitch;
    
    // 限制俯仰角
    currentPitch = std::max(-Constants::PI_2 + 0.1f, std::min(Constants::PI_2 - 0.1f, currentPitch));
    
    // 转换回笛卡尔坐标
    direction.x = radius * std::cos(currentPitch) * std::sin(currentYaw);
    direction.y = radius * std::sin(currentPitch);
    direction.z = radius * std::cos(currentPitch) * std::cos(currentYaw);
    
    m_position = m_target + direction;
    markViewDirty();
}

void Camera::rotate(float yaw, float pitch) {
    Vector3 forward = getForward();
    Vector3 right = getRight();
    Vector3 up = getUp();
    
    // 围绕世界Y轴旋转（偏航）
    Matrix3 yawRotation = Matrix3::rotation(yaw);
    forward = yawRotation * forward;
    right = yawRotation * right;
    
    // 围绕右向量旋转（俯仰）
    // 这里简化实现，实际应该使用轴角旋转
    float currentPitch = std::asin(forward.y);
    currentPitch += pitch;
    currentPitch = std::max(-Constants::PI_2 + 0.1f, std::min(Constants::PI_2 - 0.1f, currentPitch));
    
    forward.y = std::sin(currentPitch);
    float cosP = std::cos(currentPitch);
    forward.x = forward.x * cosP / std::sqrt(forward.x * forward.x + forward.z * forward.z);
    forward.z = forward.z * cosP / std::sqrt(forward.x * forward.x + forward.z * forward.z);
    
    m_target = m_position + forward;
    markViewDirty();
}

void Camera::moveForward(float distance) {
    Vector3 forward = getForward();
    move(forward * distance);
}

void Camera::moveRight(float distance) {
    Vector3 right = getRight();
    move(right * distance);
}

void Camera::moveUp(float distance) {
    Vector3 up = getUp();
    move(up * distance);
}

void Camera::setPosition(const Vector3& position) {
    m_position = position;
    markViewDirty();
}

void Camera::setRotation(const Vector3& rotation) {
    // 根据欧拉角计算新的目标位置
    // 这里简化实现，假设相机始终看向原点
    Vector3 direction = Vector3(
        std::sin(rotation.y) * std::cos(rotation.x),
        std::sin(rotation.x),
        std::cos(rotation.y) * std::cos(rotation.x)
    );
    
    m_target = m_position + direction;
    markViewDirty();
}

void Camera::setTarget(const Vector3& target) {
    m_target = target;
    markViewDirty();
}

void Camera::setUpVector(const Vector3& up) {
    m_up = up;
    markViewDirty();
}

void Camera::setFOV(float fov) {
    m_fov = fov;
    markProjectionDirty();
}

void Camera::setAspect(float aspect) {
    m_aspect = aspect;
    markProjectionDirty();
}

void Camera::setNear(float near) {
    m_near = near;
    markProjectionDirty();
}

void Camera::setFar(float far) {
    m_far = far;
    markProjectionDirty();
}

void Camera::updateViewMatrix() const {
    Vector3 forward = (m_target - m_position).normalize();
    Vector3 right = forward.cross(m_up).normalize();
    Vector3 up = right.cross(forward);
    
    // 构建视图矩阵
    m_viewMatrix = Matrix4({
        right.x,   up.x,   -forward.x,  0,
        right.y,   up.y,   -forward.y,  0,
        right.z,   up.z,   -forward.z,  0,
        -right.dot(m_position), -up.dot(m_position), forward.dot(m_position), 1
    });
    
    m_viewDirty = false;
}

void Camera::updateProjectionMatrix() const {
    m_projectionMatrix = Matrix4::perspective(m_fov, m_aspect, m_near, m_far);
    m_projectionDirty = false;
}

void Camera::updateFrustum() const {
    Matrix4 vp = getViewProjectionMatrix();
    
    // 从视图投影矩阵提取6个裁剪平面
    // 左平面
    m_frustum.planes[0] = Vector4(
        vp.m[3] + vp.m[0],
        vp.m[7] + vp.m[4], 
        vp.m[11] + vp.m[8],
        vp.m[15] + vp.m[12]
    );
    
    // 右平面
    m_frustum.planes[1] = Vector4(
        vp.m[3] - vp.m[0],
        vp.m[7] - vp.m[4],
        vp.m[11] - vp.m[8],
        vp.m[15] - vp.m[12]
    );
    
    // 下平面
    m_frustum.planes[2] = Vector4(
        vp.m[3] + vp.m[1],
        vp.m[7] + vp.m[5],
        vp.m[11] + vp.m[9],
        vp.m[15] + vp.m[13]
    );
    
    // 上平面
    m_frustum.planes[3] = Vector4(
        vp.m[3] - vp.m[1],
        vp.m[7] - vp.m[5],
        vp.m[11] - vp.m[9],
        vp.m[15] - vp.m[13]
    );
    
    // 近平面
    m_frustum.planes[4] = Vector4(
        vp.m[3] + vp.m[2],
        vp.m[7] + vp.m[6],
        vp.m[11] + vp.m[10],
        vp.m[15] + vp.m[14]
    );
    
    // 远平面
    m_frustum.planes[5] = Vector4(
        vp.m[3] - vp.m[2],
        vp.m[7] - vp.m[6],
        vp.m[11] - vp.m[10],
        vp.m[15] - vp.m[14]
    );
    
    // 归一化平面方程
    for (int i = 0; i < 6; i++) {
        Vector3 normal(m_frustum.planes[i].x, m_frustum.planes[i].y, m_frustum.planes[i].z);
        float length = normal.length();
        if (length > 0) {
            m_frustum.planes[i] = m_frustum.planes[i] / length;
        }
    }
    
    m_frustumDirty = false;
}

void Camera::markViewDirty() {
    m_viewDirty = true;
    m_frustumDirty = true;
}

void Camera::markProjectionDirty() {
    m_projectionDirty = true;
    m_frustumDirty = true;
}

} // namespace Scene
