#include "spin_opengl_widget.h"

#include <QDebug>
#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
SpinOpenGLWidget::SpinOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(320, 240);
    setFocusPolicy(Qt::StrongFocus);
}

SpinOpenGLWidget::~SpinOpenGLWidget()
{
    cleanup();
}

void SpinOpenGLWidget::cleanup()
{
    if (!context()) {
        return;
    }

    makeCurrent();

    vao.destroy();
    vbo.destroy();
    program.reset();

    ready = false;
    geometryDirty = true;

    doneCurrent();

    disconnect(context(),
               &QOpenGLContext::aboutToBeDestroyed,
               this,
               &SpinOpenGLWidget::cleanup);
}

void SpinOpenGLWidget::addLine(
    const QVector3D &from,
    const QVector3D &to,
    const QVector3D &color)
{
    vertices.push_back({
        {from.x(), from.y(), from.z()},
        {color.x(), color.y(), color.z()}
    });

    vertices.push_back({
        {to.x(), to.y(), to.z()},
        {color.x(), color.y(), color.z()}
    });
}

void SpinOpenGLWidget::setFrame(
    const UrFaustSim::SimulationConfig &config,
    const UrFaustSim::SimulationFrame &frame)
{
    vertices.clear();
    geometryDirty = true;

    // Пять отрезков на стрелку, по две вершины.
    constexpr std::size_t verticesPerSpin = 10;

    const auto maxSpins =
        static_cast<std::size_t>(
            std::numeric_limits<int>::max()) /
        (verticesPerSpin * sizeof(Vertex));

    if (frame.spinDirections.size() > maxSpins) {
        qWarning() << "Too many spins for the OpenGL buffer.";
        update();
        return;
    }

    const auto lx = static_cast<std::size_t>(config.lx);

    const auto ly =
        config.dimension == UrFaust::Dimension::one
            ? std::size_t{1}
            : static_cast<std::size_t>(config.ly);

    const auto lz =
        config.dimension == UrFaust::Dimension::three
            ? static_cast<std::size_t>(config.lz)
            : std::size_t{1};

    if (lx == 0 || ly == 0 || lz == 0) {
        update();
        return;
    }

    vertices.reserve(
        frame.spinDirections.size() * verticesPerSpin);

    // Центрируем решётку и приводим её размер
    // к удобному для отображения масштабу.
    const float scale =
        2.0f / static_cast<float>(std::max({lx, ly, lz}));

    const QVector3D center(
        0.5f * (static_cast<float>(lx) - 1.0f),
        0.5f * (static_cast<float>(ly) - 1.0f),
        0.5f * (static_cast<float>(lz) - 1.0f));

    const float arrowLength = 0.75f * scale;
    const float headLength = 0.25f * arrowLength;
    const float headWidth = 0.12f * arrowLength;

    const std::size_t yzStride = ly * lz;

    for (std::size_t i = 0;
         i < frame.spinDirections.size();
         ++i) {
        // Порядок узлов: z меняется быстрее y и x.
        const std::size_t x = i / yzStride;
        const std::size_t remainder = i % yzStride;
        const std::size_t y = remainder / lz;
        const std::size_t z = remainder % lz;

        const QVector3D base =
            (QVector3D(
                 static_cast<float>(x),
                 static_cast<float>(y),
                 static_cast<float>(z)) - center) * scale;

        const auto &spin = frame.spinDirections[i];

        const QVector3D direction = QVector3D(
                                        static_cast<float>(spin[0]),
                                        static_cast<float>(spin[1]),
                                        static_cast<float>(spin[2])).normalized();

        const QVector3D tip =
            base + arrowLength * direction;

        const QVector3D reference =
            std::abs(direction.z()) < 0.9f
                ? QVector3D(0.0f, 0.0f, 1.0f)
                : QVector3D(0.0f, 1.0f, 0.0f);

        const QVector3D side =
            QVector3D::crossProduct(
                direction, reference).normalized();

        const QVector3D otherSide =
            QVector3D::crossProduct(
                direction, side).normalized();

        const QVector3D headBase =
            tip - headLength * direction;

        // Цвет также зависит от направления.
        const QVector3D color(
            0.575f + 0.375f * direction.x(),
            0.575f + 0.375f * direction.y(),
            0.575f + 0.375f * direction.z());

        addLine(base, tip, color);

        addLine(tip, headBase + headWidth * side, color);
        addLine(tip, headBase - headWidth * side, color);

        addLine(tip, headBase + headWidth * otherSide, color);
        addLine(tip, headBase - headWidth * otherSide, color);
    }

    update();
}

void SpinOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    const auto actualFormat = context()->format();

    qDebug().nospace()
        << "OpenGL context: "
        << actualFormat.majorVersion()
        << "."
        << actualFormat.minorVersion();

    connect(context(),
            &QOpenGLContext::aboutToBeDestroyed,
            this,
            &SpinOpenGLWidget::cleanup,
            Qt::DirectConnection);

    program = std::make_unique<QOpenGLShaderProgram>();

    const char *vertexShader = R"(#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 mvp;

out vec3 vertexColor;

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
    vertexColor = color;
}
)";

    const char *fragmentShader = R"(#version 330 core
in vec3 vertexColor;

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor, 1.0);
}
)";

    if (!program->addShaderFromSourceCode(
            QOpenGLShader::Vertex, vertexShader) ||
        !program->addShaderFromSourceCode(
            QOpenGLShader::Fragment, fragmentShader) ||
        !program->link()) {
        qWarning() << "OpenGL shader error:"
                   << program->log();
        return;
    }

    if (!vao.create() || !vbo.create()) {
        qWarning() << "Could not create OpenGL buffers.";
        return;
    }

    vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    program->bind();
    vao.bind();
    vbo.bind();

    program->enableAttributeArray(0);
    program->setAttributeBuffer(
        0,
        GL_FLOAT,
        static_cast<int>(offsetof(Vertex, position)),
        3,
        static_cast<int>(sizeof(Vertex)));

    program->enableAttributeArray(1);
    program->setAttributeBuffer(
        1,
        GL_FLOAT,
        static_cast<int>(offsetof(Vertex, color)),
        3,
        static_cast<int>(sizeof(Vertex)));

    vao.release();
    vbo.release();
    program->release();

    ready = true;
    geometryDirty = true;
}

void SpinOpenGLWidget::paintGL()
{
    glClearColor(0.08f, 0.10f, 0.14f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glLineWidth(1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!ready || vertices.empty()) {
        return;
    }

    QMatrix4x4 projection;
    projection.perspective(
        45.0f,
        static_cast<float>(width()) /
            static_cast<float>(std::max(1, height())),
        0.1f,
        100.0f);

    const float yawRadians = qDegreesToRadians(yaw);
    const float pitchRadians = qDegreesToRadians(pitch);

    const QVector3D eye(
        cameraDistance *
            std::cos(pitchRadians) * std::cos(yawRadians),

        cameraDistance *
            std::cos(pitchRadians) * std::sin(yawRadians),

        cameraDistance * std::sin(pitchRadians));

    QMatrix4x4 view;
    view.lookAt(
        eye,
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 0.0f, 1.0f));
    program->bind();
    vao.bind();
    vbo.bind();

    if (geometryDirty) {
        vbo.allocate(
            vertices.data(),
            static_cast<int>(
                vertices.size() * sizeof(Vertex)));

        geometryDirty = false;
    }

    program->setUniformValue("mvp", projection * view);

    glDrawArrays(
        GL_LINES,
        0,
        static_cast<GLsizei>(vertices.size()));

    vao.release();
    vbo.release();
    program->release();
}
void SpinOpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        lastMousePosition = event->position();

        event->accept();
        return;
    }

    QOpenGLWidget::mousePressEvent(event);
}

void SpinOpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }

    const QPointF delta =
        event->position() - lastMousePosition;

    lastMousePosition = event->position();

    yaw = std::remainder(
        yaw - static_cast<float>(delta.x()) * 0.4f,
        360.0f);

    pitch = std::clamp(
        pitch + static_cast<float>(delta.y()) * 0.4f,
        -85.0f,
        85.0f);

    update();
    event->accept();
}

void SpinOpenGLWidget::wheelEvent(QWheelEvent *event)
{
    const int delta = event->angleDelta().y();

    if (delta == 0) {
        event->ignore();
        return;
    }

    const float steps =
        static_cast<float>(delta) / 120.0f;

    const float factor =
        static_cast<float>(std::pow(0.9f, steps));

    cameraDistance = std::clamp(
        cameraDistance * factor,
        2.0f,
        20.0f);

    update();
    event->accept();
}