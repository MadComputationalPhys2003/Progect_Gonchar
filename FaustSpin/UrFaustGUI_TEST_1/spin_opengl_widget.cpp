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
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QColor>
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
    const float aspect =
        static_cast<float>(std::max(1, width())) /
        static_cast<float>(std::max(1, height()));

    QMatrix4x4 projection;

    if (orthographicView) {
        const float halfHeight =
            cameraDistance *
            std::tan(qDegreesToRadians(22.5f));

        const float halfWidth = halfHeight * aspect;

        projection.ortho(
            -halfWidth, halfWidth,
            -halfHeight, halfHeight,
            0.1f, 100.0f);
    }
    else {
        projection.perspective(
            45.0f, aspect, 0.1f, 100.0f);
    }

    const float yawRadians = qDegreesToRadians(yaw);
    const float pitchRadians = qDegreesToRadians(pitch);

    const float cosYaw = std::cos(yawRadians);
    const float sinYaw = std::sin(yawRadians);
    const float cosPitch = std::cos(pitchRadians);
    const float sinPitch = std::sin(pitchRadians);

    const QVector3D eye(
        cameraDistance * cosPitch * cosYaw,
        cameraDistance * cosPitch * sinYaw,
        cameraDistance * sinPitch);

    const QVector3D cameraUp(
        -sinPitch * cosYaw,
        -sinPitch * sinYaw,
        cosPitch);

    QMatrix4x4 view;
    view.lookAt(
        eye,
        QVector3D(0.0f, 0.0f, 0.0f),
        cameraUp);

    QPainter painter(this);
    painter.beginNativePainting();

    glClearColor(0.08f, 0.10f, 0.14f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glLineWidth(1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (ready && !vertices.empty()) {
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

        program->setUniformValue(
            "mvp",
            projection * view);

        glDrawArrays(
            GL_LINES,
            0,
            static_cast<GLsizei>(vertices.size()));

        vao.release();
        vbo.release();
        program->release();
    }

    painter.endNativePainting();

    drawOrientationAxes(painter, view);
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
        -90.0f,
        90.0f);

    orthographicView = false;

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

void SpinOpenGLWidget::setViewAlongX()
{
    yaw = 0.0f;
    pitch = 0.0f;
    orthographicView = true;
    update();
}

void SpinOpenGLWidget::setViewAlongY()
{
    yaw = -90.0f;
    pitch = 0.0f;
    orthographicView = true;
    update();
}

void SpinOpenGLWidget::setViewAlongZ()
{
    yaw = -90.0f;
    pitch = 90.0f;
    orthographicView = true;
    update();
}

void SpinOpenGLWidget::resetView()
{
    yaw = -52.0f;
    pitch = 29.0f;
    cameraDistance = 5.2f;
    orthographicView = false;
    update();
}

void SpinOpenGLWidget::drawOrientationAxes(
    QPainter &painter,
    const QMatrix4x4 &view)
{
    painter.save();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    const QPointF origin(60.0, height() - 60.0);
    constexpr qreal axisLength = 34.0;

    const auto drawAxis = [&](const QVector3D &axis,
                              const QColor &color,
                              const QString &label)
    {
        const QVector3D rotated = view.mapVector(axis);

        const QPointF offset(
            axisLength * rotated.x(),
            -axisLength * rotated.y());

        const QPointF end = origin + offset;
        const qreal length =
            std::hypot(offset.x(), offset.y());

        painter.setPen(QPen(color, 2.0));
        painter.setBrush(color);

        // Ось, направленная вдоль взгляда,
        // проецируется в точку.
        if (length < 1.0) {
            painter.drawEllipse(origin, 3.0, 3.0);
            painter.drawText(
                origin + QPointF(6.0, 14.0),
                label);
            return;
        }

        painter.drawLine(origin, end);

        const QPointF direction = offset / length;
        const QPointF normal(
            -direction.y(),
            direction.x());

        painter.drawLine(
            end,
            end - direction * 6.0 + normal * 3.0);

        painter.drawLine(
            end,
            end - direction * 6.0 - normal * 3.0);

        painter.drawText(
            end + QPointF(5.0, -5.0),
            label);
    };

    drawAxis(
        QVector3D(1.0f, 0.0f, 0.0f),
        QColor(240, 80, 80),
        QStringLiteral("X"));

    drawAxis(
        QVector3D(0.0f, 1.0f, 0.0f),
        QColor(90, 220, 100),
        QStringLiteral("Y"));

    drawAxis(
        QVector3D(0.0f, 0.0f, 1.0f),
        QColor(100, 160, 255),
        QStringLiteral("Z"));

    painter.restore();
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