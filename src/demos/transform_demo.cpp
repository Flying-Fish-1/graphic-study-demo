#include "transform_demo.h"

TransformDemo::TransformDemo(Graphics* graphics) : m_graphics(graphics) {
}

void TransformDemo::drawRotatedTriangle(float centerX, float centerY, float angle, float scale, const Color& color) {
    // 定义一个标准三角形的顶点（相对于原点）
    Vector2 vertices[3] = {
        Vector2(0, -30),    // 顶点
        Vector2(-26, 15),   // 左下
        Vector2(26, 15)     // 右下
    };
    
    // 创建变换矩阵：缩放 -> 旋转 -> 平移
    Matrix3 transform = Matrix3::translation(centerX, centerY) * 
                    Matrix3::rotation(angle) * 
                    Matrix3::scale(scale, scale);
    
    // 变换顶点
    Vector2 transformedVertices[3];
    for (int i = 0; i < 3; i++) {
        transformedVertices[i] = transform.transform(vertices[i]);
    }
    
    // 绘制三角形
    m_graphics->rasterizeTriangle(
        transformedVertices[0].x, transformedVertices[0].y,
        transformedVertices[1].x, transformedVertices[1].y,
        transformedVertices[2].x, transformedVertices[2].y,
        color, color, color
    );
}

void TransformDemo::drawTransformedQuad(float centerX, float centerY, float angle, 
                                       float scaleX, float scaleY, const Color& color) {
    // 定义正方形的四个顶点
    Vector2 vertices[4] = {
        Vector2(-20, -20),  // 左下
        Vector2(20, -20),   // 右下
        Vector2(20, 20),    // 右上
        Vector2(-20, 20)    // 左上
    };
    
    // 创建变换矩阵
    Matrix3 transform = Matrix3::translation(centerX, centerY) * 
                    Matrix3::rotation(angle) * 
                    Matrix3::scale(scaleX, scaleY);
    
    // 变换顶点
    Vector2 transformedVertices[4];
    for (int i = 0; i < 4; i++) {
        transformedVertices[i] = transform.transform(vertices[i]);
    }
    
    // 绘制两个三角形组成正方形
    // 第一个三角形：左下、右下、右上
    m_graphics->rasterizeTriangle(
        transformedVertices[0].x, transformedVertices[0].y,
        transformedVertices[1].x, transformedVertices[1].y,
        transformedVertices[2].x, transformedVertices[2].y,
        color, color, color
    );
    
    // 第二个三角形：左下、右上、左上
    m_graphics->rasterizeTriangle(
        transformedVertices[0].x, transformedVertices[0].y,
        transformedVertices[2].x, transformedVertices[2].y,
        transformedVertices[3].x, transformedVertices[3].y,
        color, color, color
    );
}

void TransformDemo::draw3DProjectionDemo(float time) {
    // 定义一个3D立方体的顶点
    Vector3 cubeVertices[8] = {
        Vector3(-50, -50, -50), Vector3(50, -50, -50), Vector3(50, 50, -50), Vector3(-50, 50, -50),  // 后面
        Vector3(-50, -50, 50),  Vector3(50, -50, 50),  Vector3(50, 50, 50),  Vector3(-50, 50, 50)   // 前面
    };
    
    // 创建3D变换矩阵：旋转
    Matrix4 rotationX = Matrix4::rotationX(time * 0.5f);
    Matrix4 rotationY = Matrix4::rotationY(time * 0.7f);
    Matrix4 rotationZ = Matrix4::rotationZ(time * 0.3f);
    Matrix4 transform = rotationZ * rotationY * rotationX;
    
    // 变换并投影到2D
    Vector2 projectedVertices[8];
    for (int i = 0; i < 8; i++) {
        Vector3 transformed = transform * cubeVertices[i];
        // 简单的透视投影
        float perspective = 200.0f / (200.0f + transformed.z);
        projectedVertices[i] = Vector2(
            400 + transformed.x * perspective,  // 屏幕中心x
            300 + transformed.y * perspective   // 屏幕中心y
        );
    }
    
    // 绘制立方体的边框
    Color edgeColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // 后面的4条边
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        m_graphics->drawLine(
            projectedVertices[i].x, projectedVertices[i].y,
            projectedVertices[next].x, projectedVertices[next].y,
            edgeColor, edgeColor
        );
    }
    
    // 前面的4条边
    for (int i = 4; i < 8; i++) {
        int next = 4 + ((i - 4 + 1) % 4);
        m_graphics->drawLine(
            projectedVertices[i].x, projectedVertices[i].y,
            projectedVertices[next].x, projectedVertices[next].y,
            edgeColor, edgeColor
        );
    }
    
    // 连接前后面的4条边
    for (int i = 0; i < 4; i++) {
        m_graphics->drawLine(
            projectedVertices[i].x, projectedVertices[i].y,
            projectedVertices[i + 4].x, projectedVertices[i + 4].y,
            edgeColor, edgeColor
        );
    }
}

void TransformDemo::drawVectorDemo(float time) {
    Vector2 center(200, 150);
    
    // 创建两个旋转的向量
    Vector2 vec1(std::cos(time) * 80, std::sin(time) * 80);
    Vector2 vec2(std::cos(time * 1.5f + Constants::PI_2) * 60, 
              std::sin(time * 1.5f + Constants::PI_2) * 60);
    
    // 计算向量和
    Vector2 vecSum = vec1 + vec2;
    
    // 绘制向量
    Color red(1.0f, 0.0f, 0.0f, 1.0f);
    Color green(0.0f, 1.0f, 0.0f, 1.0f);
    Color blue(0.0f, 0.0f, 1.0f, 1.0f);
    
    // 绘制第一个向量
    m_graphics->drawLine(
        center.x, center.y,
        center.x + vec1.x, center.y + vec1.y,
        red, red
    );
    
    // 绘制第二个向量
    m_graphics->drawLine(
        center.x, center.y,
        center.x + vec2.x, center.y + vec2.y,
        green, green
    );
    
    // 绘制向量和
    m_graphics->drawLine(
        center.x, center.y,
        center.x + vecSum.x, center.y + vecSum.y,
        blue, blue
    );
    
    // 绘制向量和的平行四边形构造
    Color gray(0.5f, 0.5f, 0.5f, 1.0f);
    m_graphics->drawLine(
        center.x + vec1.x, center.y + vec1.y,
        center.x + vecSum.x, center.y + vecSum.y,
        gray, gray
    );
    
    m_graphics->drawLine(
        center.x + vec2.x, center.y + vec2.y,
        center.x + vecSum.x, center.y + vecSum.y,
        gray, gray
    );
}
