#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <QVulkanWindow>
#include <QVulkanFunctions>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <QString>

class VulkanWindow;
class WavesWidget;

struct Vertex
{
    alignas(8) glm::vec2 inPos;
};

struct PushConstants
{
    alignas(16) glm::mat4 projection;
};

class VulkanRenderer : public QVulkanWindowRenderer
{
public:
    explicit VulkanRenderer(VulkanWindow* window);

    void preInitResources() override;
    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;

    void startNextFrame() override;

    void ensureBuffers();

private:
    void createVertices();
    VkShaderModule createShaderModule(const QString& fileName);
    void buildDrawCommand();

private:
    VulkanWindow* m_window = nullptr;

    QVulkanDeviceFunctions* m_devFuncs = nullptr;

    PushConstants myPushConstants{};

    VkShaderModule vertShader{VK_NULL_HANDLE};
    VkShaderModule fragShader{VK_NULL_HANDLE};

    VkPipelineCache  m_pipelineCache{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    VkPipeline       m_pipeline{VK_NULL_HANDLE};

    VkBuffer       m_vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory m_vertexBufferMemory{VK_NULL_HANDLE};
    VkDeviceSize   m_vertexBufferSize{0};

    std::vector<Vertex> m_verticesData;

    int m_instanceCount{0};
    int m_vertexCount{0};
};

#endif // VULKANRENDERER_H