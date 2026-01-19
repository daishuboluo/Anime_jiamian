// 更推薦的寫法（現代化一點）

Widget::Widget(QWidget* parent)
    : QWidget(parent)
{
    if (!inst.create())
    {
        qFatal("無法創建 Vulkan 實例: %d", inst.errorCode());
    }

    qInfo() << "Vulkan 實例創建成功！";

    vulkanWindow = new VulkanWindow(640, 480);
    vulkanWindow->setVulkanInstance(&inst);

    container = QWidget::createWindowContainer(vulkanWindow, this);
    container->setMinimumSize(640, 480);
}

void Widget::resizeEvent(QResizeEvent* /*event*/)
{
    if (container)
    {
        container->setGeometry(rect());
    }
}