#pragma once

#include <QWidget>
#include "vulkanwindow.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget();

    VulkanWindow* vulkanWindow = nullptr;
    QVulkanInstance inst;
    QWidget* container = nullptr;

protected:
    void resizeEvent(QResizeEvent* event) override;
};