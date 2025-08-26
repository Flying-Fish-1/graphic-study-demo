#ifndef RENDERER_LIGHTING_LIGHT_H
#define RENDERER_LIGHTING_LIGHT_H

#include "../../core/math/vector.h"
#include "../../core/types/color.h"

namespace Renderer {
namespace Lighting {

using namespace Core::Math;
using namespace Core::Types;

/**
 * @brief 光源基类
 * 
 * 定义了所有光源的通用接口
 */
class Light {
public:
    enum Type { POINT, DIRECTIONAL, SPOT };
    
protected:
    Type m_type;            // 光源类型
    Color m_color;          // 光源颜色
    float m_intensity;      // 光源强度
    float m_ambientIntensity; // 环境光强度
    
public:
    /**
     * @brief 构造函数
     * @param type 光源类型
     * @param color 光源颜色
     * @param intensity 光源强度
     */
    Light(Type type, const Color& color = Color::WHITE, float intensity = 1.0f);
    
    /**
     * @brief 虚析构函数
     */
    virtual ~Light() = default;
    
    /**
     * @brief 获取光源到指定位置的方向
     * @param worldPos 世界坐标位置
     * @return 光线方向向量（从表面指向光源）
     */
    virtual Vector3 getDirection(const Vector3& worldPos) const = 0;
    
    /**
     * @brief 获取光线在指定位置的衰减
     * @param worldPos 世界坐标位置
     * @return 衰减系数 [0-1]
     */
    virtual float getAttenuation(const Vector3& worldPos) const = 0;
    
    /**
     * @brief 检查位置是否在光源照射范围内
     * @param worldPos 世界坐标位置
     * @return 是否可见
     */
    virtual bool isVisible(const Vector3& worldPos) const = 0;
    
    // Getter方法
    Type getType() const { return m_type; }
    const Color& getColor() const { return m_color; }
    float getIntensity() const { return m_intensity; }
    float getAmbientIntensity() const { return m_ambientIntensity; }
    
    // Setter方法
    void setColor(const Color& color) { m_color = color; }
    void setIntensity(float intensity) { m_intensity = intensity; }
    void setAmbientIntensity(float intensity) { m_ambientIntensity = intensity; }
};

/**
 * @brief 点光源
 * 
 * 从一个点向所有方向发射光线
 */
class PointLight : public Light {
private:
    Vector3 m_position;     // 光源位置
    float m_range;          // 光照范围
    float m_constantAttenuation;   // 常数衰减
    float m_linearAttenuation;     // 线性衰减
    float m_quadraticAttenuation;  // 二次衰减
    
public:
    /**
     * @brief 构造函数
     * @param position 光源位置
     * @param color 光源颜色
     * @param intensity 光源强度
     * @param range 光照范围
     */
    PointLight(const Vector3& position, const Color& color = Color::WHITE, 
               float intensity = 1.0f, float range = 100.0f);
    
    Vector3 getDirection(const Vector3& worldPos) const override;
    float getAttenuation(const Vector3& worldPos) const override;
    bool isVisible(const Vector3& worldPos) const override;
    
    // Getter/Setter
    const Vector3& getPosition() const { return m_position; }
    void setPosition(const Vector3& position) { m_position = position; }
    
    float getRange() const { return m_range; }
    void setRange(float range) { m_range = range; }
    
    void setAttenuation(float constant, float linear, float quadratic);
};

/**
 * @brief 方向光源
 * 
 * 从无限远处发射平行光线（如太阳光）
 */
class DirectionalLight : public Light {
private:
    Vector3 m_direction;    // 光线方向
    
public:
    /**
     * @brief 构造函数
     * @param direction 光线方向
     * @param color 光源颜色
     * @param intensity 光源强度
     */
    DirectionalLight(const Vector3& direction, const Color& color = Color::WHITE, 
                     float intensity = 1.0f);
    
    Vector3 getDirection(const Vector3& worldPos) const override;
    float getAttenuation(const Vector3& worldPos) const override;
    bool isVisible(const Vector3& worldPos) const override;
    
    // Getter/Setter
    const Vector3& getLightDirection() const { return m_direction; }
    void setLightDirection(const Vector3& direction) { m_direction = direction.normalize(); }
};

} // namespace Lighting
} // namespace Renderer

#endif // RENDERER_LIGHTING_LIGHT_H
