#pragma once

#include <QVulkanWindow>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>

class VulkanRenderer;
class WavesWidget;

class VulkanWindow : public QVulkanWindow
{
    Q_OBJECT

public:
    explicit VulkanWindow(int w, int h);

    QVulkanWindowRenderer* createRenderer() override;

    bool isDebugEnabled() const { return m_debug; }

    WavesWidget* m_wavesWidget   = nullptr;
    QTimer*      animationTimer  = nullptr;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    bool            m_debug    = false;
    VulkanRenderer* m_renderer = nullptr;
};