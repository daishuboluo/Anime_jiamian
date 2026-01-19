#include "vulkanwindow.h"
#include "vulkanrenderer.h"

VulkanWindow::VulkanWindow(int w, int h)
{
    m_wavesWidget = new WavesWidget(this);
    animationTimer = new QTimer(this);

    m_wavesWidget->setLines(w, h);
    m_wavesWidget->tick_();
}

QVulkanWindowRenderer* VulkanWindow::createRenderer()
{
    m_renderer = new VulkanRenderer(this);
    return m_renderer;
}

void VulkanWindow::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    QWindow::resizeEvent(event);
}

void VulkanWindow::mouseMoveEvent(QMouseEvent* event)
{
    m_wavesWidget->updateMouse(event->x(), event->y());
    QWindow::mouseMoveEvent(event);
}