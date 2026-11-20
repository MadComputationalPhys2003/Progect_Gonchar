#include "exchange_cheat_sheet_dialog.h"
#include "ui_exchange_cheat_sheet_dialog.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QColor>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFontDatabase>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidgetItem>
#include <QTimer>
#include <QTransform>
#include <QWheelEvent>

#include <cmath>
#include <limits>
#include <stdexcept>

ExchangeCheatSheetDialog::ExchangeCheatSheetDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ExchangeCheatSheetDialog)
    , scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    ui->latticeGraphicsView->setScene(scene);
    ui->latticeGraphicsView->setRenderHint(QPainter::Antialiasing);
    ui->latticeGraphicsView->setBackgroundBrush(QBrush(Qt::white));
    ui->latticeGraphicsView->setDragMode(
        QGraphicsView::ScrollHandDrag
        );
    ui->latticeGraphicsView->setTransformationAnchor(
        QGraphicsView::AnchorUnderMouse
        );

    ui->latticeGraphicsView->viewport()->installEventFilter(this);

    ui->latticeGraphicsView->setToolTip(
        QStringLiteral(
            "Click a bond to select its matrix.\n"
            "Mouse wheel: zoom. Drag the background: move.\n"
            "Dashed curves: periodic boundary connections.\n"
            "3D lattices are shown as an oblique projection."
            )
        );

    // Крупная подпись выбранной обменной матрицы.
    QFont pairFont = ui->pairInfoLabel->font();
    pairFont.setPointSize(22);
    pairFont.setBold(true);

    ui->pairInfoLabel->setFont(pairFont);
    ui->pairInfoLabel->setTextFormat(Qt::RichText);
    ui->pairInfoLabel->setMinimumSize(190, 60);

    ui->matrixNumberSpinBox->setMinimumWidth(90);
    ui->matrixNumberSpinBox->setEnabled(false);
    ui->fitViewButton->setEnabled(false);

    auto* table = ui->matrixPreviewTableWidget;

    table->setRowCount(3);
    table->setColumnCount(3);

    const QStringList axes{
        QStringLiteral("x"),
        QStringLiteral("y"),
        QStringLiteral("z")
    };

    table->setHorizontalHeaderLabels(axes);
    table->setVerticalHeaderLabels(axes);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);

    table->setFont(
        QFontDatabase::systemFont(QFontDatabase::FixedFont)
        );

    table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );
    table->verticalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);

    connect(
        ui->buttonBox,
        &QDialogButtonBox::rejected,
        this,
        &QDialog::reject
        );

    connect(
        ui->fitViewButton,
        &QPushButton::clicked,
        this,
        &ExchangeCheatSheetDialog::fitView
        );

    connect(
        ui->matrixNumberSpinBox,
        &QSpinBox::valueChanged,
        this,
        &ExchangeCheatSheetDialog::showMatrix
        );

    connect(
        scene,
        &QGraphicsScene::selectionChanged,
        this,
        [this]() {
            const auto selected = scene->selectedItems();

            if (selected.isEmpty()) {
                return;
            }

            const int number = selected.front()->data(0).toInt();

            if (ui->matrixNumberSpinBox->value() == number) {
                showMatrix(number);
            }
            else {
                ui->matrixNumberSpinBox->setValue(number);
            }
        }
        );
}

ExchangeCheatSheetDialog::~ExchangeCheatSheetDialog()
{
    delete ui;
}

void ExchangeCheatSheetDialog::setData(
    std::uint64_t lx,
    std::uint64_t ly,
    std::uint64_t lz,
    const std::vector<std::pair<std::size_t, std::size_t>>& pairs,
    const std::vector<FM::Matrix>& matrices)
{
    const std::array<std::uint64_t, 3> size{{lx, ly, lz}};

    std::size_t count = 1;

    for (std::uint64_t length : size) {
        if (length == 0 ||
            length > std::numeric_limits<std::size_t>::max() / count) {
            throw std::invalid_argument(
                "Invalid lattice dimensions"
                );
        }

        count *= static_cast<std::size_t>(length);
    }

    if (pairs.empty() ||
        pairs.size() != matrices.size() ||
        pairs.size() > static_cast<std::size_t>(
            std::numeric_limits<int>::max())) {
        throw std::invalid_argument(
            "Invalid pair or matrix count"
            );
    }

    for (std::size_t k = 0; k < pairs.size(); ++k) {
        if (pairs[k].first >= pairs[k].second ||
            pairs[k].second >= count) {
            throw std::invalid_argument(
                "Ion pair is outside the lattice"
                );
        }

        if (matrices[k].get_rows() != 3 ||
            matrices[k].get_cols() != 3) {
            throw std::invalid_argument(
                "Exchange matrix must be 3x3"
                );
        }

        for (std::size_t element = 0; element < 9; ++element) {
            if (!std::isfinite(matrices[k][element])) {
                throw std::invalid_argument(
                    "Exchange matrix must be finite"
                    );
            }
        }
    }

    dimensions = size;
    nodeCount = count;
    pairsValue = pairs;
    matricesValue = matrices;

    drawScene();

    {
        const QSignalBlocker blocker(ui->matrixNumberSpinBox);

        ui->matrixNumberSpinBox->setRange(
            0,
            static_cast<int>(pairs.size()) - 1
            );

        ui->matrixNumberSpinBox->setValue(0);
        ui->matrixNumberSpinBox->setEnabled(true);
    }

    ui->fitViewButton->setEnabled(true);

    showMatrix(0);

    QTimer::singleShot(
        0,
        this,
        [this]() {
            fitView();
        }
        );
}

std::array<std::uint64_t, 3>
ExchangeCheatSheetDialog::coords(std::size_t index) const
{
    const std::uint64_t value =
        static_cast<std::uint64_t>(index);

    const std::uint64_t plane =
        dimensions[1] * dimensions[2];

    return {{
        value / plane,
        (value % plane) / dimensions[2],
        value % dimensions[2]
    }};
}

QPointF ExchangeCheatSheetDialog::position(
    std::size_t index) const
{
    const auto c = coords(index);

    // X направлена вправо, Y вверх.
    // Третья координата задаёт наклонную проекцию.
    return QPointF(
        180.0 * static_cast<double>(c[0]) +
            75.0 * static_cast<double>(c[2]),

        -180.0 * static_cast<double>(c[1]) -
            115.0 * static_cast<double>(c[2])
        );
}

bool ExchangeCheatSheetDialog::crossesBoundary(
    std::size_t i,
    std::size_t j) const
{
    const auto a = coords(i);
    const auto b = coords(j);

    for (std::size_t axis = 0; axis < 3; ++axis) {
        const std::uint64_t distance =
            a[axis] > b[axis]
                ? a[axis] - b[axis]
                : b[axis] - a[axis];

        if (dimensions[axis] > 2 &&
            distance == dimensions[axis] - 1) {
            return true;
        }
    }

    return false;
}

void ExchangeCheatSheetDialog::drawScene()
{
    const QSignalBlocker blocker(scene);

    scene->clear();

    bonds.clear();
    bondLabels.clear();
    nodes.clear();
    periodicBonds.clear();

    QFont labelFont = font();
    labelFont.setPointSize(12);
    labelFont.setBold(true);

    const QPointF center =
        position(nodeCount - 1) * 0.5;

    // Обменные связи.
    for (std::size_t k = 0; k < pairsValue.size(); ++k) {
        const auto& pair = pairsValue[k];

        const QPointF a = position(pair.first);
        const QPointF b = position(pair.second);

        const QPointF middle = (a + b) * 0.5;
        const QPointF delta = b - a;

        const double length =
            std::hypot(delta.x(), delta.y());

        QPointF normal = length > 0.0
                             ? QPointF(
                                   -delta.y() / length,
                                   delta.x() / length
                                   )
                             : QPointF(0.0, -1.0);

        const QPointF outward = middle - center;

        if (QPointF::dotProduct(normal, outward) <= 0.0) {
            normal = -normal;
        }

        const bool periodic =
            crossesBoundary(pair.first, pair.second);

        QPainterPath path(a);

        if (periodic) {
            path.quadTo(
                middle + normal * (80.0 + length * 0.2),
                b
                );
        }
        else {
            path.lineTo(b);
        }

        auto* bond = scene->addPath(path);

        bond->setFlag(QGraphicsItem::ItemIsSelectable);
        bond->setData(0, static_cast<int>(k));
        bond->setCursor(Qt::PointingHandCursor);
        bond->setZValue(-1.0);

        bond->setToolTip(
            QStringLiteral("Matrix %1: J(%2 %3)")
                .arg(static_cast<qulonglong>(k))
                .arg(static_cast<qulonglong>(pair.first))
                .arg(static_cast<qulonglong>(pair.second))
            );

        bonds.push_back(bond);
        periodicBonds.push_back(periodic);

        auto* label = scene->addText(QString(), labelFont);

        label->setHtml(
            QStringLiteral(
                "<span style=\"background-color:white;\">"
                "J<sub>%1%2</sub></span>"
                )
                .arg(static_cast<qulonglong>(pair.first))
                .arg(static_cast<qulonglong>(pair.second))
            );

        // Подписи сохраняют читаемый размер при масштабировании.
        label->setFlag(
            QGraphicsItem::ItemIgnoresTransformations
            );

        label->setPos(
            path.pointAtPercent(0.5) + normal * 20.0
            );

        const QRectF bounds = label->boundingRect();

        label->setTransform(
            QTransform::fromTranslate(
                -bounds.width() * 0.5,
                -bounds.height() * 0.5
                )
            );

        label->setZValue(3.0);
        label->setAcceptedMouseButtons(Qt::NoButton);

        bondLabels.push_back(label);
    }

    // Узлы и их индексы.
    for (std::size_t i = 0; i < nodeCount; ++i) {
        auto* dot = scene->addEllipse(
            -6.0,
            -6.0,
            12.0,
            12.0,
            QPen(Qt::black),
            QBrush(Qt::black)
            );

        dot->setPos(position(i));
        dot->setZValue(2.0);

        dot->setFlag(
            QGraphicsItem::ItemIgnoresTransformations
            );

        dot->setAcceptedMouseButtons(Qt::NoButton);

        nodes.push_back(dot);

        auto* label = new QGraphicsTextItem(dot);

        label->setFont(labelFont);

        label->setPlainText(
            QString::number(static_cast<qulonglong>(i))
            );

        label->setDefaultTextColor(Qt::black);

        label->setPos(
            -label->boundingRect().width() * 0.5,
            7.0
            );

        label->setAcceptedMouseButtons(Qt::NoButton);
    }

    scene->setSceneRect(
        scene->itemsBoundingRect().adjusted(
            -50.0, -50.0, 50.0, 50.0
            )
        );
}

void ExchangeCheatSheetDialog::showMatrix(int number)
{
    if (number < 0 ||
        static_cast<std::size_t>(number) >= pairsValue.size()) {
        return;
    }

    const std::size_t selected =
        static_cast<std::size_t>(number);

    const auto& pair = pairsValue[selected];

    const QColor orange(QStringLiteral("#d65a00"));

    // На небольших плоских схемах показываем все подписи.
    // В 3D — подпись выбранной связи.
    const bool showAll =
        dimensions[2] == 1 && bonds.size() <= 24;

    const QSignalBlocker blocker(scene);

    scene->clearSelection();

    for (std::size_t k = 0; k < bonds.size(); ++k) {
        const bool current = k == selected;

        QPen pen(
            current
                ? orange
                : QColor(QStringLiteral("#718096")),
            current ? 4.0 : 1.5
            );

        pen.setStyle(
            periodicBonds[k]
                ? Qt::DashLine
                : Qt::SolidLine
            );

        bonds[k]->setPen(pen);
        bonds[k]->setZValue(current ? 0.0 : -1.0);

        bondLabels[k]->setDefaultTextColor(
            current ? orange : QColor(Qt::black)
            );

        bondLabels[k]->setVisible(current || showAll);
    }

    for (std::size_t i = 0; i < nodes.size(); ++i) {
        const bool endpoint =
            i == pair.first || i == pair.second;

        const QColor color =
            endpoint ? orange : QColor(Qt::black);

        nodes[i]->setBrush(QBrush(color));
        nodes[i]->setPen(QPen(color));
    }

    ui->pairInfoLabel->setText(
        QStringLiteral("J<sub>%1%2</sub>")
            .arg(static_cast<qulonglong>(pair.first))
            .arg(static_cast<qulonglong>(pair.second))
        );

    const FM::Matrix& matrix = matricesValue[selected];

    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            auto* item =
                ui->matrixPreviewTableWidget->item(row, column);

            if (item == nullptr) {
                item = new QTableWidgetItem;

                item->setTextAlignment(Qt::AlignCenter);

                ui->matrixPreviewTableWidget->setItem(
                    row,
                    column,
                    item
                    );
            }

            item->setText(
                QString::number(
                    matrix[static_cast<std::size_t>(
                        row * 3 + column
                        )],
                    'g',
                    std::numeric_limits<double>::max_digits10
                    )
                );

            item->setToolTip(item->text());
        }
    }
}

void ExchangeCheatSheetDialog::fitView()
{
    if (pairsValue.empty()) {
        return;
    }

    ui->latticeGraphicsView->fitInView(
        scene->sceneRect(),
        Qt::KeepAspectRatio
        );

    ui->latticeGraphicsView->scale(0.9, 0.9);
}

bool ExchangeCheatSheetDialog::eventFilter(
    QObject* watched,
    QEvent* event)
{
    if (watched == ui->latticeGraphicsView->viewport() &&
        event->type() == QEvent::Wheel) {
        const auto* wheel =
            static_cast<QWheelEvent*>(event);

        const double factor =
            std::pow(1.0015, wheel->angleDelta().y());

        const double scale =
            ui->latticeGraphicsView->transform().m11();

        if ((factor > 1.0 && scale < 8.0) ||
            (factor < 1.0 && scale > 0.03)) {
            ui->latticeGraphicsView->scale(factor, factor);
        }

        return true;
    }

    return QDialog::eventFilter(watched, event);
}
