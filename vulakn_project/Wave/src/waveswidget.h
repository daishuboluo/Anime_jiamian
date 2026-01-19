#ifndef WAVESWIDGET_H
#define WAVESWIDGET_H

#include <QObject>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QElapsedTimer>
#include <QColor>
#include <cmath>
#include <vector>
#include <QPainterPath>

struct Grad
{
    qreal x, y, z;

    Grad() : x(0.0), y(0.0), z(0.0) {}
    Grad(qreal gx, qreal gy, qreal gz) : x(gx), y(gy), z(gz) {}

    qreal dot2(qreal dx, qreal dy) const { return x * dx + y * dy; }
};

class Noise
{
public:
    explicit Noise(qreal seedVal = 0);

    void seed(qreal seedVal);

    static qreal fade(qreal t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    static qreal lerp(qreal a, qreal b, qreal t) { return (1 - t) * a + t * b; }

    qreal perlin2(qreal x, qreal y) const;

private:
    std::vector<Grad> grad3;
    std::vector<int> p;
    std::vector<int> perm;
    std::vector<Grad> gradP;
};

struct Point
{
    qreal x, y;
    struct Wave { qreal x, y; } wave;
    struct Cursor { qreal x, y, vx, vy; } cursor;
};

struct MouseState
{
    qreal x, y;
    qreal lx, ly;
    qreal sx, sy;
    qreal v;
    qreal vs;
    qreal a;
    bool set;
};

struct Config
{
    QColor lineColor;
    qreal waveSpeedX;
    qreal waveSpeedY;
    qreal waveAmpX;
    qreal waveAmpY;
    qreal xGap;
    qreal yGap;
    qreal friction;
    qreal tension;
    qreal maxCursorMove;
};

class WavesWidget : public QObject
{
    Q_OBJECT

public:
    explicit WavesWidget(QObject* parent = nullptr);
    ~WavesWidget();

    void setWaveSpeedX(qreal speed) { config.waveSpeedX = speed; }
    void setWaveSpeedY(qreal speed) { config.waveSpeedY = speed; }
    void setWaveAmpX(qreal amp)     { config.waveAmpX = amp; }
    void setWaveAmpY(qreal amp)     { config.waveAmpY = amp; }
    void setFriction(qreal f)       { config.friction = f; }
    void setTension(qreal t)        { config.tension = t; }
    void setMaxCursorMove(qreal move) { config.maxCursorMove = move; }

public Q_SLOTS:
    void tick_();

public:
    std::vector<std::vector<Point>>& getLines() { return lines; }

    void setLines(int width, int height);
    void movePoints(qint64 time);
    QPointF moved(const Point& point, bool withCursor = true);
    void updateMouse(int x, int y);

    Noise* noiseGenerator = nullptr;
    std::vector<std::vector<Point>> lines;
    MouseState mouse;
    Config config;
    QElapsedTimer timeElapsed;
};

#endif // WAVESWIDGET_H