#ifndef SPIN_OPENGL_WIDGET_H
#define SPIN_OPENGL_WIDGET_H

#include "UrFaustSimulation_1.h"
#include <QPointF>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QVector3D>

#include <memory>
#include <vector>
class QMouseEvent;
class QWheelEvent;
class QPainter;
class QMatrix4x4;
class SpinOpenGLWidget : public QOpenGLWidget,
                         protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit SpinOpenGLWidget(QWidget *parent = nullptr);
    ~SpinOpenGLWidget() override;

    void setFrame(
        const UrFaustSim::SimulationConfig &config,
        const UrFaustSim::SimulationFrame &frame);
    void setViewAlongX();
    void setViewAlongY();
    void setViewAlongZ();
    void resetView();
protected:
    void initializeGL() override;
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    struct Vertex {
        float position[3];
        float color[3];
    };

    std::vector<Vertex> vertices;
    QPointF lastMousePosition;

    float yaw = -52.0f;
    float pitch = 29.0f;
    float cameraDistance = 5.2f;
    QOpenGLBuffer vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject vao;
    std::unique_ptr<QOpenGLShaderProgram> program;
    bool orthographicView = false;
    bool ready = false;
    bool geometryDirty = true;

    void addLine(
        const QVector3D &from,
        const QVector3D &to,
        const QVector3D &color);

    void cleanup();
    void drawOrientationAxes(
        QPainter &painter,
        const QMatrix4x4 &view);
};

#endif // SPIN_OPENGL_WIDGET_H