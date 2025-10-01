#ifndef SCENE_CAMERA_H
#define SCENE_CAMERA_H

#include "../core/math/vector.h"
#include "../core/math/matrix.h"

namespace Scene {

using namespace Core::Math;

/**
 * @brief 视锥体结构
 */
struct Frustum {
    Vector4 planes[6];      // 6个裁剪平面
    
    /**
     * @brief 检查点是否在视锥体内
     */
    bool isPointInside(const Vector3& point) const;
    
    /**
     * @brief 检查球体是否在视锥体内
     */
    bool isSphereInside(const Vector3& center, float radius) const;
    
    /**
     * @brief 检查包围盒是否在视锥体内
     */
    bool isBoxInside(const Vector3& min, const Vector3& max) const;
};

/**
 * @brief 相机类
 * 
 * 负责管理3D场景的视点和投影
 */
class Camera {
private:
    Vector3 m_position;     // 相机位置
    Vector3 m_target;       // 目标位置
    Vector3 m_up;           // 上方向
    
    float m_fov;            // 视野角度（弧度）
    float m_aspect;         // 宽高比
    float m_near;           // 近裁剪面
    float m_far;            // 远裁剪面
    
    // 缓存的矩阵
    mutable Matrix4 m_viewMatrix;
    mutable Matrix4 m_projectionMatrix;
    mutable Matrix4 m_viewProjectionMatrix;
    mutable Frustum m_frustum;
    
    // 脏标记
    mutable bool m_viewDirty;
    mutable bool m_projectionDirty;
    mutable bool m_frustumDirty;
    
public:
    /**
     * @brief 构造函数
     */
    Camera();
    
    /**
     * @brief 构造函数
     * @param position 相机位置
     * @param target 目标位置
     * @param up 上方向
     */
    Camera(const Vector3& position, const Vector3& target, const Vector3& up);
    
    /**
     * @brief 设置相机位置和朝向
     * @param position 相机位置
     * @param target 目标位置
     * @param up 上方向
     */
    void lookAt(const Vector3& position, const Vector3& target, const Vector3& up);
    
    /**
     * @brief 设置透视投影参数
     * @param fov 视野角度（弧度）
     * @param aspect 宽高比
     * @param near 近裁剪面
     * @param far 远裁剪面
     */
    void setPerspective(float fov, float aspect, float near, float far);
    
    /**
     * @brief 设置正交投影参数
     * @param left 左边界
     * @param right 右边界
     * @param bottom 下边界
     * @param top 上边界
     * @param near 近裁剪面
     * @param far 远裁剪面
     */
    void setOrthographic(float left, float right, float bottom, float top, float near, float far);
    
    /**
     * @brief 获取视图矩阵
     */
    const Matrix4& getViewMatrix() const;
    
    /**
     * @brief 获取投影矩阵
     */
    const Matrix4& getProjectionMatrix() const;
    
    /**
     * @brief 获取视图投影矩阵
     */
    const Matrix4& getViewProjectionMatrix() const;
    
    /**
     * @brief 获取视口矩阵
     * @param screenWidth 屏幕宽度
     * @param screenHeight 屏幕高度
     */
    Matrix4 getViewportMatrix(int screenWidth, int screenHeight) const;
    
    /**
     * @brief 获取视锥体
     */
    const Frustum& getFrustum() const;
    
    /**
     * @brief 获取前向向量
     */
    Vector3 getForward() const;
    
    /**
     * @brief 获取右向向量
     */
    Vector3 getRight() const;
    
    /**
     * @brief 获取上向向量
     */
    Vector3 getUp() const;
    
    /**
     * @brief 移动相机
     * @param offset 位移偏移
     */
    void move(const Vector3& offset);
    
    /**
     * @brief 围绕目标旋转相机
     * @param yaw 偏航角（弧度）
     * @param pitch 俯仰角（弧度）
     */
    void orbit(float yaw, float pitch);
    
    /**
     * @brief 第一人称相机旋转
     * @param yaw 偏航角（弧度）
     * @param pitch 俯仰角（弧度）
     */
    void rotate(float yaw, float pitch);
    
    /**
     * @brief 沿前向移动
     * @param distance 移动距离
     */
    void moveForward(float distance);
    
    /**
     * @brief 沿右向移动
     * @param distance 移动距离
     */
    void moveRight(float distance);
    
    /**
     * @brief 沿上向移动
     * @param distance 移动距离
     */
    void moveUp(float distance);
    
    // Getter方法
    const Vector3& getPosition() const { return m_position; }
    const Vector3& getTarget() const { return m_target; }
    const Vector3& getUpVector() const { return m_up; }
    float getFOV() const { return m_fov; }
    float getAspect() const { return m_aspect; }
    float getNear() const { return m_near; }
    float getFar() const { return m_far; }
    
    // Setter方法
    void setPosition(const Vector3& position);
    void setTarget(const Vector3& target);
    void setUpVector(const Vector3& up);
    void setRotation(const Vector3& rotation);  // 设置旋转（欧拉角）
    void setFOV(float fov);
    void setAspect(float aspect);
    void setNear(float near);
    void setFar(float far);
    
private:
    /**
     * @brief 更新视图矩阵
     */
    void updateViewMatrix() const;
    
    /**
     * @brief 更新投影矩阵
     */
    void updateProjectionMatrix() const;
    
    /**
     * @brief 更新视锥体
     */
    void updateFrustum() const;
    
    /**
     * @brief 标记视图矩阵为脏
     */
    void markViewDirty();
    
    /**
     * @brief 标记投影矩阵为脏
     */
    void markProjectionDirty();
};

} // namespace Scene

#endif // SCENE_CAMERA_H
