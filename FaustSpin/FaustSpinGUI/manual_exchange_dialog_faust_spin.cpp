#include "manual_exchange_dialog_faust_spin.h"
#include "ui_manual_exchange_dialog_faust_spin.h"

#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStaticText>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QVector3D>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

constexpr int matrixSize = 3;

QVector3D sitePosition(const FaustSpin::InitLattice &lattice,
                       FaustSpin::Dimension dimension, std::size_t site)
{
    switch (dimension) {
    case FaustSpin::Dimension::one:
        return {static_cast<float>(site), 0, 0};
    case FaustSpin::Dimension::two: {
        const auto [x, y] = lattice.get_lattice2D().get_coords(site);
        return {static_cast<float>(x), static_cast<float>(y), 0};
    }
    case FaustSpin::Dimension::three: {
        const auto [x, y, z] = lattice.get_lattice3D().get_coords(site);
        return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
    }
    }
    throw std::logic_error("Unknown lattice dimension");
}

QString formatSite(std::size_t site, const QVector3D &position)
{
    return QStringLiteral("%1 (%2, %3, %4)")
    .arg(static_cast<qulonglong>(site))
        .arg(position.x(), 0, 'f', 0)
        .arg(position.y(), 0, 'f', 0)
        .arg(position.z(), 0, 'f', 0);
}

QString bondTitle(std::size_t i, std::size_t j)
{
    return QStringLiteral("J<sub>%1 %2</sub>")
    .arg(static_cast<qulonglong>(i))
        .arg(static_cast<qulonglong>(j));
}

QPointF obliquePosition(const QVector3D &position)
{
    return {position.x() + (75.0 / 180.0) * position.z(),
            -position.y() - (115.0 / 180.0) * position.z()};
}

QPointF outwardNormal(const QPointF &a, const QPointF &b, const QPointF &center)
{
    const QPointF delta = b - a;
    const qreal length = std::hypot(delta.x(), delta.y());
    QPointF normal = length > 0 ? QPointF(-delta.y(), delta.x()) / length : QPointF(0, -1);
    if (QPointF::dotProduct(normal, (a + b) / 2 - center) <= 0) {
        normal = -normal;
    }
    return normal;
}

} // namespace

class ManualExchangeDialogFaustSpin::LatticeView final : public QWidget
{
public:
    LatticeView(ManualExchangeDialogFaustSpin &editor, FaustSpin::Dimension dimension,
                QWidget *parent)
        : QWidget(parent), editor_(editor)
    {
        setMinimumSize(360, 320);
        setObjectName(QStringLiteral("latticeView"));
        setAccessibleName(tr("Exchange bonds"));
        setToolTip(tr("Click a bond to select its matrix. Drag to rotate. "
                      "Right-drag to pan. Scroll to zoom."));

        const auto &lattice = editor_.lattice_;
        sites_.reserve(lattice.getN());
        for (std::size_t i = 0; i < lattice.getN(); ++i) {
            sites_.push_back(sitePosition(lattice, dimension, i));
        }

        center_ = sites_.back() / 2.0f;
        const auto &pairs = lattice.getUniquePairs();
        showAllBondLabels_ = dimension != FaustSpin::Dimension::three && pairs.size() <= 24;
        bonds_.reserve(pairs.size());
        for (const auto &[i, j] : pairs) {
            const QVector3D &a = sites_.at(i);
            const QVector3D &b = sites_.at(j);
            Bond bond{i, j, (a + b) / 2.0f, (b - a).lengthSquared() > 1.0f};
            if (bond.periodic) {
                const QPointF start = obliquePosition(a);
                const QPointF end = obliquePosition(b);
                const QPointF delta = end - start;
                const QPointF bend = outwardNormal(start, end, obliquePosition(center_)) *
                                     (80.0 / 180.0 + 0.2 * std::hypot(delta.x(), delta.y()));
                bond.control += QVector3D(bend.x(), -bend.y(), 0);
            }
            bonds_.push_back(bond);
        }
    }

    const QVector3D &site(std::size_t index) const
    {
        return sites_.at(index);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);
        painter.setRenderHint(QPainter::Antialiasing);
        QFont labelFont = font();
        labelFont.setPointSize(12);
        labelFont.setBold(true);
        painter.setFont(labelFont);

        QMatrix4x4 rotation;
        rotation.rotate(pitch_, 1, 0, 0);
        rotation.rotate(yaw_, 0, 1, 0);
        const qreal side = std::min(width(), height());
        const qreal scale = zoom_ * std::max<qreal>(side - 60, 1) /
                            (2.0 * center_.length() + 1.15);
        const QPointF origin = QPointF(width() / 2.0, height() / 2.0) + pan_;
        const auto project = [&](const QVector3D &position) {
            const QVector3D rotated = rotation.map(position - center_);
            return Projection{origin + obliquePosition(rotated) * scale, rotated.z()};
        };

        projectedSites_.clear();
        projectedSites_.reserve(sites_.size());
        for (const auto &site : sites_) {
            projectedSites_.push_back(project(site));
        }

        paths_.clear();
        paths_.reserve(bonds_.size());
        for (const auto &bond : bonds_) {
            const auto &a = projectedSites_[bond.i];
            const auto &b = projectedSites_[bond.j];
            const auto control = project(bond.control);
            QPainterPath path(a.point);
            if (bond.periodic) {
                path.quadTo(control.point, b.point);
            } else {
                path.lineTo(b.point);
            }
            paths_.push_back({std::move(path), a.depth, control.depth, b.depth});
        }

        const int selected = editor_.currentBond_;
        const QColor orange(QStringLiteral("#d65a00"));
        const QColor gray(QStringLiteral("#718096"));
        const auto drawBond = [&](int index, const QColor &color, qreal width) {
            painter.setPen(QPen(color, width, bonds_[index].periodic ? Qt::DashLine : Qt::SolidLine));
            painter.drawPath(paths_[index].curve);
        };

        for (std::size_t k = 0; k < paths_.size(); ++k) {
            if (static_cast<int>(k) != selected) {
                drawBond(static_cast<int>(k), gray, 1.5);
            }
        }
        if (selected >= 0) {
            drawBond(selected, orange, 4.0);
        }

        for (std::size_t i = 0; i < projectedSites_.size(); ++i) {
            const bool endpoint = selected >= 0 &&
                                  (i == bonds_[selected].i || i == bonds_[selected].j);
            const QPointF point = projectedSites_[i].point;
            const QColor color = endpoint ? orange : QColor(Qt::black);
            painter.setPen(color);
            painter.setBrush(color);
            painter.drawEllipse(point, 6.0, 6.0);
            const auto text = QString::number(static_cast<qulonglong>(i));
            painter.setPen(Qt::black);
            painter.drawText(point + QPointF(-painter.fontMetrics().horizontalAdvance(text) / 2.0,
                                             7.0 + painter.fontMetrics().ascent()), text);
        }

        const auto drawLabel = [&](int index, const QColor &color) {
            const auto &bond = bonds_[index];
            const auto a = projectedSites_[bond.i].point;
            const auto b = projectedSites_[bond.j].point;
            const auto middle = paths_[index].curve.pointAtPercent(0.5);
            const auto location = middle + outwardNormal(a, b, origin) * 20;
            QStaticText text(bondTitle(bond.i, bond.j));
            text.setTextFormat(Qt::RichText);
            text.prepare(QTransform(), labelFont);
            const QSizeF size = text.size();
            const QRectF bounds(location - QPointF(size.width() / 2, size.height() / 2), size);
            painter.fillRect(bounds, Qt::white);
            painter.setPen(color);
            painter.drawStaticText(bounds.topLeft(), text);
        };
        if (showAllBondLabels_) {
            for (std::size_t k = 0; k < bonds_.size(); ++k) {
                if (static_cast<int>(k) != selected) {
                    drawLabel(static_cast<int>(k), Qt::black);
                }
            }
        }
        if (selected >= 0) {
            drawLabel(selected, orange);
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
            dragStart_ = lastMouse_ = event->position();
            dragging_ = false;
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (!(event->buttons() & (Qt::LeftButton | Qt::RightButton))) {
            return;
        }
        const QPointF position = event->position();
        if (!dragging_ &&
            (position - dragStart_).manhattanLength() < QApplication::startDragDistance()) {
            return;
        }
        dragging_ = true;
        const QPointF change = position - lastMouse_;
        lastMouse_ = position;
        if (event->buttons() & Qt::RightButton) {
            pan_ += change;
        } else {
            yaw_ = std::remainder(yaw_ - 0.4 * change.x(), 360.0);
            pitch_ = std::clamp(pitch_ + 0.4 * change.y(), -90.0, 90.0);
        }
        update();
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && !dragging_) {
            pickBond(event->position());
        }
    }

    void wheelEvent(QWheelEvent *event) override
    {
        const int delta = event->angleDelta().y();
        if (delta == 0) {
            event->ignore();
            return;
        }
        const qreal next = std::clamp(zoom_ * std::pow(1.15, delta / 120.0), 0.25, 32.0);
        const QPointF center(width() / 2.0, height() / 2.0);
        const QPointF cursor = event->position() - center;
        pan_ = cursor + (pan_ - cursor) * (next / zoom_);
        zoom_ = next;
        update();
        event->accept();
    }

private:
    struct Bond {
        std::size_t i, j;
        QVector3D control;
        bool periodic;
    };
    struct Projection {
        QPointF point;
        float depth;
    };
    struct Path {
        QPainterPath curve;
        float startDepth, controlDepth, endDepth;
    };

    void pickBond(const QPointF &click)
    {
        constexpr qreal hitRadius = 7.0;
        qreal bestDistance = hitRadius;
        qreal bestDepth = -std::numeric_limits<qreal>::infinity();
        int best = -1;
        for (std::size_t k = 0; k < paths_.size(); ++k) {
            const auto &path = paths_[k];
            if (!path.curve.controlPointRect().adjusted(-hitRadius, -hitRadius,
                                                        hitRadius, hitRadius).contains(click)) {
                continue;
            }
            const int steps = bonds_[k].periodic ? 24 : 1;
            const QPointF start = projectedSites_[bonds_[k].i].point;
            const QPointF end = projectedSites_[bonds_[k].j].point;
            QPointF a = start;
            for (int step = 1; step <= steps; ++step) {
                const QPointF b = path.curve.isEmpty() ? end
                                                       : path.curve.pointAtPercent(static_cast<qreal>(step) / steps);
                const QPointF delta = b - a;
                const qreal lengthSquared = QPointF::dotProduct(delta, delta);
                const qreal t = lengthSquared > 0
                                    ? std::clamp(QPointF::dotProduct(click - a, delta) / lengthSquared, 0.0, 1.0)
                                    : (path.endDepth > path.startDepth ? 1.0 : 0.0);
                const QPointF offset = click - (a + delta * t);
                const qreal distance = std::hypot(offset.x(), offset.y());
                if (distance <= hitRadius && distance <= bestDistance + 0.01) {
                    const qreal u = (step - 1 + t) / steps;
                    const qreal depth = std::lerp(std::lerp(path.startDepth, path.controlDepth, u),
                                                  std::lerp(path.controlDepth, path.endDepth, u), u);
                    if (distance < bestDistance - 0.01 || depth > bestDepth) {
                        best = static_cast<int>(k);
                        bestDistance = distance;
                        bestDepth = depth;
                    }
                }
                a = b;
            }
        }
        if (best >= 0) {
            editor_.selectBond(best);
        }
    }

    ManualExchangeDialogFaustSpin &editor_;
    std::vector<QVector3D> sites_;
    std::vector<Bond> bonds_;
    std::vector<Projection> projectedSites_;
    std::vector<Path> paths_;
    QVector3D center_;
    QPointF dragStart_;
    QPointF lastMouse_;
    QPointF pan_;
    double yaw_ = 0.0;
    double pitch_ = 0.0;
    double zoom_ = 1.0;
    bool dragging_ = false;
    bool showAllBondLabels_ = false;
};

class ManualExchangeDialogFaustSpin::MatrixItemDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        editor_ = QStyledItemDelegate::createEditor(parent, option, index);
        return editor_.data();
    }

    void destroyEditor(QWidget *editor, const QModelIndex &index) const override
    {
        if (editor_ == editor) {
            editor_.clear();
        }
        QStyledItemDelegate::destroyEditor(editor, index);
    }

    void finishEditing()
    {
        if (editor_) {
            // A click on a bond can change the selected matrix before focus leaves the cell.
            emit commitData(editor_.data());
            if (editor_) {
                emit closeEditor(editor_.data(), QAbstractItemDelegate::NoHint);
            }
        }
    }

private:
    mutable QPointer<QWidget> editor_;
};

ManualExchangeDialogFaustSpin::ManualExchangeDialogFaustSpin(
    const FaustSpin::InitLattice &lattice, FaustSpin::Dimension dimension,
    const std::vector<FM::Matrix> &previous, double defaultExchange, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::ManualExchangeDialogFaustSpin>())
    , lattice_(lattice)
{
    const auto count = lattice_.getUniquePairs().size();
    if (count == 0 || count > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("Unsupported number of exchange bonds");
    }
    if (!std::isfinite(defaultExchange)) {
        throw std::invalid_argument("Exchange integral must be finite");
    }
    if (previous.empty()) {
        FM::Matrix initial(3, 3);
        initial[0] = initial[4] = initial[8] = defaultExchange;
        matrices_.assign(count, initial);
    } else {
        if (previous.size() != count) {
            throw std::invalid_argument("Exchange matrix count does not match the lattice");
        }
        for (const auto &matrix : previous) {
            if (matrix.get_rows() != 3 || matrix.get_cols() != 3) {
                throw std::invalid_argument("Exchange matrix must be 3x3");
            }
            for (std::size_t k = 0; k < 9; ++k) {
                if (!std::isfinite(matrix[k])) {
                    throw std::invalid_argument("Exchange matrix elements must be finite");
                }
            }
        }
        matrices_ = previous;
    }

    ui->setupUi(this);
    auto *table = ui->exchangeMatrixTable;
    const QStringList axes{QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z")};
    table->setRowCount(matrixSize);
    table->setColumnCount(matrixSize);
    table->setHorizontalHeaderLabels(axes);
    table->setVerticalHeaderLabels(axes);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSortingEnabled(false);
    for (int row = 0; row < matrixSize; ++row) {
        for (int column = 0; column < matrixSize; ++column) {
            auto *item = new QTableWidgetItem;
            item->setTextAlignment(Qt::AlignCenter);
            table->setItem(row, column, item);
        }
    }
    matrixEditor_ = new MatrixItemDelegate(table);
    table->setItemDelegate(matrixEditor_);

    view_ = new LatticeView(*this, dimension, ui->latticeViewHost);
    auto *viewLayout = ui->latticeViewHost->layout();
    if (!viewLayout) {
        viewLayout = new QVBoxLayout(ui->latticeViewHost);
        viewLayout->setContentsMargins(0, 0, 0, 0);
    }
    viewLayout->addWidget(view_);

    ui->exchangeBondIndex->setRange(0, static_cast<int>(count) - 1);
    ui->exchangeBondIndex->setKeyboardTracking(false);
    ui->exchangeBondCoordinates->setTextFormat(Qt::RichText);
    ui->exchangeBondCoordinates->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->exchangeInputError->setWordWrap(true);
    ui->exchangeInputError->hide();

    connect(ui->exchangeBondIndex, &QSpinBox::valueChanged,
            this, &ManualExchangeDialogFaustSpin::selectBond);
    selectBond(0);
}

ManualExchangeDialogFaustSpin::~ManualExchangeDialogFaustSpin() = default;

const std::vector<FM::Matrix> &ManualExchangeDialogFaustSpin::getMatrices() const
{
    return matrices_;
}

bool ManualExchangeDialogFaustSpin::saveCurrentMatrix()
{
    if (currentBond_ < 0) {
        return true;
    }
    matrixEditor_->finishEditing();
    auto *table = ui->exchangeMatrixTable;
    std::array<double, matrixSize * matrixSize> values{};
    for (int row = 0; row < matrixSize; ++row) {
        for (int column = 0; column < matrixSize; ++column) {
            bool ok = false;
            const double value = table->item(row, column)->text().toDouble(&ok);
            if (!ok || !std::isfinite(value)) {
                ui->exchangeInputError->setText(
                    tr("Enter a finite number in J%1%2. Use a decimal point.")
                        .arg(table->verticalHeaderItem(row)->text(),
                             table->horizontalHeaderItem(column)->text()));
                ui->exchangeInputError->show();
                table->setCurrentCell(row, column);
                table->setFocus();
                return false;
            }
            values[row * matrixSize + column] = value;
        }
    }

    for (std::size_t k = 0; k < values.size(); ++k) {
        matrices_[currentBond_][k] = values[k];
    }
    ui->exchangeInputError->hide();
    return true;
}

void ManualExchangeDialogFaustSpin::selectBond(int index)
{
    if (index == currentBond_) {
        return;
    }
    const QSignalBlocker blocker(ui->exchangeBondIndex);
    if (!saveCurrentMatrix()) {
        ui->exchangeBondIndex->setValue(currentBond_);
        return;
    }

    currentBond_ = index;
    ui->exchangeBondIndex->setValue(index);
    const auto &[i, j] = lattice_.getUniquePairs()[index];
    ui->exchangeBondCoordinates->setText(
        QStringLiteral("<span style=\"font-size:22pt; font-weight:bold; color:#d65a00;\">%1</span>"
                       "<br>%2")
            .arg(bondTitle(i, j),
                 tr("i = %1<br>j = %2").arg(formatSite(i, view_->site(i)),
                                            formatSite(j, view_->site(j)))));
    const auto &matrix = matrices_[index];
    auto *table = ui->exchangeMatrixTable;
    for (int row = 0; row < matrixSize; ++row) {
        for (int column = 0; column < matrixSize; ++column) {
            table->item(row, column)->setText(QString::number(
                matrix[row * matrixSize + column], 'g', std::numeric_limits<double>::max_digits10));
        }
    }
    view_->update();
}

void ManualExchangeDialogFaustSpin::accept()
{
    if (saveCurrentMatrix()) {
        QDialog::accept();
    }
}

