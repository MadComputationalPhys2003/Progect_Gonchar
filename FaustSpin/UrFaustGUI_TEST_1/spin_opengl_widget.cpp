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
#include <QSvgGenerator>
#include <QSaveFile>
#include <QPolygonF>
#include <QVector4D>
#include <utility>
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
void SpinOpenGLWidget::addCone(
    const QVector3D &tip,
    const QVector3D &direction,
    float length,
    float radius,
    const QVector3D &color)
{
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

    const QVector3D base = tip - length * direction;

    const QVector3D light =
        QVector3D(-0.4f, -0.6f, 1.0f).normalized();

    const auto addTriangle =
        [&](const QVector3D &a,
            const QVector3D &b,
            const QVector3D &c)
    {
        const QVector3D normal =
            QVector3D::crossProduct(
                (b - a) / length,
                (c - a) / length).normalized();

        const float brightness =
            0.35f + 0.65f * std::max(
                        0.0f,
                        QVector3D::dotProduct(normal, light));

        const QVector3D faceColor = brightness * color;

        for (const QVector3D &point : {a, b, c}) {
            vertices.push_back({
                {point.x(), point.y(), point.z()},
                {faceColor.x(), faceColor.y(), faceColor.z()}
            });
        }
    };

    constexpr float fullCircle = 6.28318530718f;

    for (int segment = 0;
         segment < coneSegments;
         ++segment) {
        const float angle0 =
            fullCircle * static_cast<float>(segment) /
            static_cast<float>(coneSegments);

        const float angle1 =
            fullCircle * static_cast<float>(segment + 1) /
            static_cast<float>(coneSegments);

        const QVector3D rim0 =
            base + radius *
                       (std::cos(angle0) * side +
                        std::sin(angle0) * otherSide);

        const QVector3D rim1 =
            base + radius *
                       (std::cos(angle1) * side +
                        std::sin(angle1) * otherSide);

        // Боковая грань.
        addTriangle(tip, rim0, rim1);

        // Закрываем основание конуса.
        addTriangle(base, rim1, rim0);
    }
}
void SpinOpenGLWidget::setFrame(
    const UrFaustSim::SimulationConfig &config,
    const UrFaustSim::SimulationFrame &frame)
{
    vertices.clear();
    spinVertexCount = 0;
    latticeVertexCount = 0;
    nodeVertexCount = 0;
    coneVertexCount = 0;
    geometryDirty = true;
    // Пять отрезков на стрелку, по две вершины.
    constexpr std::size_t verticesPerNode =
        2 + 6 + 1 + 6 * coneSegments;

    const auto maxSpins =
        static_cast<std::size_t>(
            std::numeric_limits<int>::max()) /
        (verticesPerNode * sizeof(Vertex));

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
        frame.spinDirections.size() * verticesPerNode);

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
    std::vector<QVector3D> nodePositions;
    nodePositions.reserve(frame.spinDirections.size());
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
        nodePositions.push_back(base);
        const auto &spin = frame.spinDirections[i];

        const QVector3D direction = QVector3D(
                                        static_cast<float>(spin[0]),
                                        static_cast<float>(spin[1]),
                                        static_cast<float>(spin[2])).normalized();

        const QVector3D tip =
            base + arrowLength * direction;

        const QVector3D headBase =
            tip - headLength * direction;

        const QVector3D color(
            0.575f + 0.375f * direction.x(),
            0.575f + 0.375f * direction.y(),
            0.575f + 0.375f * direction.z());

        addLine(base, headBase, color);
    }
    spinVertexCount = static_cast<int>(vertices.size());

    const QVector3D latticeColor(0.30f, 0.38f, 0.46f);

    const auto addGridEdge =
        [&](std::size_t from, std::size_t to)
    {
        if (to < nodePositions.size()) {
            addLine(
                nodePositions[from],
                nodePositions[to],
                latticeColor);
        }
    };

    // Соединяем соседние узлы внутри отображаемой решётки.
    for (std::size_t i = 0; i < nodePositions.size(); ++i) {
        const std::size_t x = i / yzStride;
        const std::size_t remainder = i % yzStride;
        const std::size_t y = remainder / lz;
        const std::size_t z = remainder % lz;

        if (x + 1 < lx) {
            addGridEdge(i, i + yzStride);
        }

        if (y + 1 < ly) {
            addGridEdge(i, i + lz);
        }

        if (z + 1 < lz) {
            addGridEdge(i, i + 1);
        }
    }

    latticeVertexCount =
        static_cast<int>(vertices.size()) - spinVertexCount;

    // По одной светлой точке на каждый узел.
    for (const QVector3D &base : nodePositions) {
        vertices.push_back({
            {base.x(), base.y(), base.z()},
            {0.95f, 0.95f, 0.95f}
        });
    }

    nodeVertexCount = static_cast<int>(nodePositions.size());
    const int coneStart = static_cast<int>(vertices.size());

    for (std::size_t i = 0; i < nodePositions.size(); ++i) {
        const auto &spin = frame.spinDirections[i];

        const QVector3D direction = QVector3D(
                                        static_cast<float>(spin[0]),
                                        static_cast<float>(spin[1]),
                                        static_cast<float>(spin[2])).normalized();

        const QVector3D tip =
            nodePositions[i] + arrowLength * direction;

        const QVector3D color(
            0.575f + 0.375f * direction.x(),
            0.575f + 0.375f * direction.y(),
            0.575f + 0.375f * direction.z());

        addCone(
            tip,
            direction,
            headLength,
            headWidth,
            color);
    }

    coneVertexCount =
        static_cast<int>(vertices.size()) - coneStart;
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
uniform float pointSize;

out vec3 vertexColor;

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
    gl_PointSize = pointSize;
    vertexColor = color;
}
)";

    const char *fragmentShader = R"(#version 330 core
in vec3 vertexColor;

uniform bool drawNodes;

out vec4 fragmentColor;

void main()
{
    if (drawNodes) {
        vec2 offset = gl_PointCoord - vec2(0.5);

        if (dot(offset, offset) > 0.25) {
            discard;
        }
    }

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
    svgProjection = projection;
    svgView = view;

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

        program->setUniformValue("drawNodes", 0);

        program->setUniformValue(
            "pointSize",
            6.0f * static_cast<float>(devicePixelRatioF()));

        glDrawArrays(
            GL_LINES,
            spinVertexCount,
            latticeVertexCount);

        glDrawArrays(
            GL_LINES,
            0,
            spinVertexCount);

        glDrawArrays(
            GL_TRIANGLES,
            spinVertexCount + latticeVertexCount + nodeVertexCount,
            coneVertexCount);

        program->setUniformValue("drawNodes", 1);

        glEnable(GL_PROGRAM_POINT_SIZE);

        glDrawArrays(
            GL_POINTS,
            spinVertexCount + latticeVertexCount,
            nodeVertexCount);

        glDisable(GL_PROGRAM_POINT_SIZE);

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
bool SpinOpenGLWidget::saveSvg(
    const QString &fileName,
    QString &errorMessage)
{
    errorMessage.clear();

    // Обновляем изображение и матрицы текущей камеры.
    repaint();

    if (!ready || vertices.empty()) {
        errorMessage =
            QStringLiteral("There is no rendered frame to save.");
        return false;
    }

    struct SvgPrimitive {
        QPolygonF points;
        QColor color;
        float depth = 0.0f;
    };

    std::vector<SvgPrimitive> primitives;

    primitives.reserve(
        static_cast<std::size_t>(
            (spinVertexCount + latticeVertexCount) / 2
            + coneVertexCount / 3
            + nodeVertexCount
            )
        );

    const QMatrix4x4 transform =
        svgProjection * svgView;

    const qreal imageWidth = width();
    const qreal imageHeight = height();

    const auto appendPrimitives =
        [&](int start, int count, int verticesPerPrimitive)
    {
        for (int first = start;
             first < start + count;
             first += verticesPerPrimitive) {

            SvgPrimitive primitive;
            bool visible = true;

            const Vertex &firstVertex =
                vertices[static_cast<std::size_t>(first)];

            primitive.color = QColor::fromRgbF(
                firstVertex.color[0],
                firstVertex.color[1],
                firstVertex.color[2]
                );

            for (int offset = 0;
                 offset < verticesPerPrimitive;
                 ++offset) {

                const Vertex &vertex = vertices[
                    static_cast<std::size_t>(first + offset)
                ];

                const QVector4D clip = transform * QVector4D(
                                           vertex.position[0],
                                           vertex.position[1],
                                           vertex.position[2],
                                           1.0f
                                           );

                if (clip.w() <= 0.0f) {
                    visible = false;
                    break;
                }

                const float inverseW = 1.0f / clip.w();

                const qreal x =
                    (clip.x() * inverseW + 1.0f)
                    * imageWidth * 0.5;

                const qreal y =
                    (1.0f - clip.y() * inverseW)
                    * imageHeight * 0.5;

                primitive.points.append(QPointF(x, y));
                primitive.depth += clip.z() * inverseW;
            }

            if (visible) {
                primitive.depth /=
                    static_cast<float>(verticesPerPrimitive);

                primitives.push_back(std::move(primitive));
            }
        }
    };

    // Линии решётки.
    appendPrimitives(
        spinVertexCount,
        latticeVertexCount,
        2
        );

    // Стержни стрелок.
    appendPrimitives(0, spinVertexCount, 2);

    // Треугольные грани конусов.
    appendPrimitives(
        spinVertexCount + latticeVertexCount + nodeVertexCount,
        coneVertexCount,
        3
        );

    // Узлы решётки.
    appendPrimitives(
        spinVertexCount + latticeVertexCount,
        nodeVertexCount,
        1
        );

    // Сначала рисуем удалённые элементы.
    std::stable_sort(
        primitives.begin(),
        primitives.end(),
        [](const SvgPrimitive &left,
           const SvgPrimitive &right)
        {
            return left.depth > right.depth;
        }
        );

    QSaveFile output(fileName);

    if (!output.open(QIODevice::WriteOnly)) {
        errorMessage = output.errorString();
        return false;
    }

    QSvgGenerator generator(
        QSvgGenerator::SvgVersion::Svg11
        );

    generator.setOutputDevice(&output);
    generator.setSize(size());
    generator.setViewBox(
        QRect(0, 0, width(), height())
        );
    generator.setResolution(logicalDpiX());
    generator.setTitle(
        QStringLiteral("FaustSpin — current view")
        );

    QPainter painter;

    if (!painter.begin(&generator)) {
        errorMessage =
            QStringLiteral("Cannot start SVG export.");
        return false;
    }

    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(
        QRectF(0.0, 0.0, imageWidth, imageHeight),
        QColor::fromRgbF(0.08, 0.10, 0.14)
        );

    const qreal lineWidth =
        1.0 / devicePixelRatioF();

    for (const SvgPrimitive &primitive : primitives) {
        if (primitive.points.size() == 1) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(primitive.color);

            painter.drawEllipse(
                primitive.points[0],
                3.0,
                3.0
                );
        }
        else if (primitive.points.size() == 2) {
            painter.setBrush(Qt::NoBrush);
            painter.setPen(
                QPen(
                    primitive.color,
                    lineWidth,
                    Qt::SolidLine,
                    Qt::FlatCap
                    )
                );

            painter.drawLine(
                primitive.points[0],
                primitive.points[1]
                );
        }
        else {
            painter.setPen(Qt::NoPen);
            painter.setBrush(primitive.color);
            painter.drawPolygon(primitive.points);
        }
    }

    drawOrientationAxes(painter, svgView);

    if (!painter.end()) {
        errorMessage =
            QStringLiteral("Cannot finish SVG export.");
        return false;
    }

    if (!output.commit()) {
        errorMessage = output.errorString();
        return false;
    }

    return true;
}