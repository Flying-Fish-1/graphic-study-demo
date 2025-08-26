#ifndef CORE_TYPES_MATERIAL_H
#define CORE_TYPES_MATERIAL_H

#include "color.h"
#include "../math/vector.h"

namespace Core {
namespace Types {

using namespace Core::Math;

// 前向声明
class Texture;
class Light;

/**
 * @brief 材质类
 * 
 * 定义表面的材质属性，用于光照计算
 */
class Material {
private:
    Color m_ambient;        // 环境光颜色
    Color m_diffuse;        // 漫反射颜色
    Color m_specular;       // 镜面反射颜色
    float m_shininess;      // 光泽度
    Texture* m_diffuseMap;  // 漫反射贴图
    Texture* m_normalMap;   // 法线贴图
    
public:
    /**
     * @brief 默认构造函数
     */
    Material();
    
    /**
     * @brief 构造函数
     * @param ambient 环境光颜色
     * @param diffuse 漫反射颜色
     * @param specular 镜面反射颜色
     * @param shininess 光泽度
     */
    Material(const Color& ambient, const Color& diffuse, const Color& specular, float shininess);
    
    /**
     * @brief 析构函数
     */
    ~Material();
    
    // Getter方法
    const Color& getAmbient() const { return m_ambient; }
    const Color& getDiffuse() const { return m_diffuse; }
    const Color& getSpecular() const { return m_specular; }
    float getShininess() const { return m_shininess; }
    Texture* getDiffuseMap() const { return m_diffuseMap; }
    Texture* getNormalMap() const { return m_normalMap; }
    
    // Setter方法
    void setAmbient(const Color& ambient) { m_ambient = ambient; }
    void setDiffuse(const Color& diffuse) { m_diffuse = diffuse; }
    void setSpecular(const Color& specular) { m_specular = specular; }
    void setShininess(float shininess) { m_shininess = shininess; }
    void setDiffuseMap(Texture* texture) { m_diffuseMap = texture; }
    void setNormalMap(Texture* texture) { m_normalMap = texture; }
    
    /**
     * @brief 计算光照颜色
     * @param normal 表面法向量
     * @param lightDir 光线方向
     * @param viewDir 视线方向
     * @param worldPos 世界坐标位置
     * @param light 光源
     * @return 计算后的颜色
     */
    Color calculateLighting(const Vector3& normal, const Vector3& lightDir, 
                           const Vector3& viewDir, const Vector3& worldPos, 
                           const Light& light) const;
    
    /**
     * @brief 获取在指定纹理坐标处的漫反射颜色
     * @param texCoord 纹理坐标
     * @return 漫反射颜色
     */
    Color getDiffuseColor(const Vector2& texCoord) const;
    
    /**
     * @brief 获取在指定纹理坐标处的法向量
     * @param texCoord 纹理坐标
     * @return 法向量
     */
    Vector3 getNormal(const Vector2& texCoord) const;
    
    /**
     * @brief 创建预定义材质
     */
    static Material* createDefaultMaterial();
    static Material* createRedPlastic();
    static Material* createBlueMetal();
    static Material* createWhiteDiffuse();
};

} // namespace Types
} // namespace Core

#endif // CORE_TYPES_MATERIAL_H
