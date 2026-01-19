#include "waveswidget.h"
#include <QDebug>
#include <random>
#include <chrono>

WavesWidget::WavesWidget(QObject* parent)
    : QObject(parent)
{
    config.lineColor     = Qt::white;
    config.waveSpeedX    = 0.02;
    config.waveSpeedY    = 0.01;
    config.waveAmpX      = 40;
    config.waveAmpY      = 20;
    config.xGap          = 12;
    config.yGap          = 36;
    config.friction      = 0.90;
    config.tension       = 0.01;
    config.maxCursorMove = 120;

    mouse.x   = -10;
    mouse.y   = 0;
    mouse.lx  = 0;
    mouse.ly  = 0;
    mouse.sx  = 0;
    mouse.sy  = 0;
    mouse.v   = 0;
    mouse.vs  = 0;
    mouse.a   = 0;
    mouse.set = false;

    std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<> dist(0.0, 1.0);
    noiseGenerator = new Noise(dist(rng));

    timeElapsed.start();
}

WavesWidget::~WavesWidget()
{
    delete noiseGenerator;
}

void WavesWidget::tick_()
{
    qint64 currentTime = timeElapsed.elapsed();

    mouse.sx += (mouse.x - mouse.sx) * 0.1;
    mouse.sy += (mouse.y - mouse.sy) * 0.1;

    qreal dx = mouse.x - mouse.lx;
    qreal dy = mouse.y - mouse.ly;
    qreal d = std::hypot(dx, dy);

    mouse.v = d;
    mouse.vs += (d - mouse.vs) * 0.1;
    mouse.vs = std::min(100.0, mouse.vs);

    mouse.lx = mouse.x;
    mouse.ly = mouse.y;
    mouse.a = std::atan2(dy, dx);

    movePoints(currentTime);
}

void WavesWidget::setLines(int width, int height)
{
    lines.clear();

    qreal oWidth  = width  + 200;
    qreal oHeight = height + 30;

    int totalLines  = static_cast<int>(std::ceil(oWidth  / config.xGap));
    int totalPoints = static_cast<int>(std::ceil(oHeight / config.yGap));

    qreal xStart = (width  - config.xGap * totalLines)  / 2.0;
    qreal yStart = (height - config.yGap * totalPoints) / 2.0;

    for (int i = 0; i <= totalLines; ++i)
    {
        std::vector<Point> pts;

        for (int j = 0; j <= totalPoints; ++j)
        {
            pts.push_back({
                xStart + config.xGap * i,
                yStart + config.yGap * j,
                {0, 0},
                {0, 0, 0, 0}
            });
        }

        lines.push_back(pts);
    }
}

void WavesWidget::movePoints(qint64 time)
{
    for (auto& pts : lines)
    {
        for (auto& p : pts)
        {
            qreal move = noiseGenerator->perlin2(
                (p.x + time * config.waveSpeedX) * 0.002,
                (p.y + time * config.waveSpeedY) * 0.0015
            ) * 12;

            p.wave.x = std::cos(move) * config.waveAmpX;
            p.wave.y = std::sin(move) * config.waveAmpY;

            qreal dx   = p.x - mouse.sx;
            qreal dy   = p.y - mouse.sy;
            qreal dist = std::hypot(dx, dy);
            qreal l    = std::max(175.0, mouse.vs);

            if (dist < l)
            {
                qreal s = 1.0 - dist / l;
                qreal f = std::cos(dist * 0.001) * s;

                p.cursor.vx += std::cos(mouse.a) * f * l * mouse.vs * 0.00065;
                p.cursor.vy += std::sin(mouse.a) * f * l * mouse.vs * 0.00065;
            }

            p.cursor.vx += (0.0 - p.cursor.x) * config.tension;
            p.cursor.vy += (0.0 - p.cursor.y) * config.tension;
            p.cursor.vx *= config.friction;
            p.cursor.vy *= config.friction;

            p.cursor.x += p.cursor.vx * 2;
            p.cursor.y += p.cursor.vy * 2;

            p.cursor.x = std::min(config.maxCursorMove, std::max(-config.maxCursorMove, p.cursor.x));
            p.cursor.y = std::min(config.maxCursorMove, std::max(-config.maxCursorMove, p.cursor.y));
        }
    }
}

QPointF WavesWidget::moved(const Point& point, bool withCursor)
{
    qreal x = point.x + point.wave.x + (withCursor ? point.cursor.x : 0);
    qreal y = point.y + point.wave.y + (withCursor ? point.cursor.y : 0);

    return QPointF(std::round(x * 10) / 10.0, std::round(y * 10) / 10.0);
}

void WavesWidget::updateMouse(int x, int y)
{
    mouse.x = x;
    mouse.y = y;

    if (!mouse.set)
    {
        mouse.sx = mouse.x;
        mouse.sy = mouse.y;
        mouse.lx = mouse.x;
        mouse.ly = mouse.y;
        mouse.set = true;
    }
}