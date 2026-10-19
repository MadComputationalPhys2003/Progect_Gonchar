#include "manual_exchange_integral_dialog.h"
#include "ui_manual_exchange_integral_dialog.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QHeaderView>
#include <QList>
#include <QLocale>
#include <QMessageBox>
#include <QModelIndex>
#include <QPainter>
#include <QPalette>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QItemSelectionModel>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

class ExchangeMatrixDelegate final : public QStyledItemDelegate
{
public:
    explicit ExchangeMatrixDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override
    {
        painter->save();

        QStyleOptionViewItem background(option);
        initStyleOption(&background, index);
        background.text.clear();

        QStyle* style = option.widget
                            ? option.widget->style()
                            : QApplication::style();

        style->drawControl(
            QStyle::CE_ItemViewItem,
            &background,
            painter,
            option.widget
            );

        const QVariantList values =
            index.data(Qt::UserRole).toList();

        if (values.size() == 9) {
            painter->setFont(
                QFontDatabase::systemFont(
                    QFontDatabase::FixedFont
                    )
                );

            painter->setPen(
                option.palette.color(
                    option.state.testFlag(QStyle::State_Selected)
                        ? QPalette::HighlightedText
                        : QPalette::Text
                    )
                );

            const QRectF area(
                option.rect.adjusted(8, 6, -8, -6)
                );

            const double cellWidth = area.width() / 3.0;
            const double cellHeight = area.height() / 3.0;

            for (int element = 0; element < 9; ++element) {
                const int row = element / 3;
                const int column = element % 3;

                const QRectF cell(
                    area.left() + column * cellWidth,
                    area.top() + row * cellHeight,
                    cellWidth,
                    cellHeight
                    );

                painter->drawText(
                    cell,
                    Qt::AlignCenter,
                    QString::number(
                        values[element].toDouble(),
                        'g',
                        6
                        )
                    );
            }
        }

        painter->restore();
    }
};

} // namespace

ManualExchangeIntegralDialog::ManualExchangeIntegralDialog(
    QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ManualExchangeIntegralDialog)
{
    ui->setupUi(this);

    ui->exchangeIntegralDoubleSpinBox->setLocale(
        QLocale::c()
        );

    ui->exchangeStackedWidget->setCurrentWidget(
        ui->constantExchangePage
        );

    ui->matrixInstructionLabel->setText(
        QStringLiteral(
            "Double-click a matrix to edit its elements. "
            "Use a decimal point."
            )
        );

    QTableWidget* table = ui->exchangeMatricesTableWidget;

    table->setColumnCount(4);

    table->setHorizontalHeaderLabels(
        QStringList{
            QStringLiteral("Index"),
            QStringLiteral("Ion"),
            QStringLiteral("Slot"),
            QStringLiteral("Matrix")
        }
        );

    table->setSortingEnabled(false);

    table->setDragDropMode(
        QAbstractItemView::NoDragDrop
        );

    table->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    table->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    table->setSelectionMode(
        QAbstractItemView::SingleSelection
        );

    table->setMinimumWidth(520);

    table->verticalHeader()->hide();
    table->verticalHeader()->setDefaultSectionSize(100);

    for (int column = 0; column < 3; ++column) {
        table->horizontalHeader()->setSectionResizeMode(
            column,
            QHeaderView::ResizeToContents
            );
    }

    table->horizontalHeader()->setSectionResizeMode(
        3,
        QHeaderView::Stretch
        );

    table->setItemDelegateForColumn(
        3,
        new ExchangeMatrixDelegate(table)
        );

    connect(
        table,
        &QTableWidget::cellDoubleClicked,
        this,
        &ManualExchangeIntegralDialog::editMatrix
        );
    ui->matrixSearchValueSpinBox->setRange(
        0,
        std::numeric_limits<int>::max()
        );

    ui->findMatrixButton->setAutoDefault(false);
    ui->findNextMatrixButton->setAutoDefault(false);

    connect(
        ui->findMatrixButton,
        &QPushButton::clicked,
        this,
        [this]() {
            findMatrix(false);
        }
        );

    connect(
        ui->findNextMatrixButton,
        &QPushButton::clicked,
        this,
        [this]() {
            findMatrix(true);
        }
        );
}

ManualExchangeIntegralDialog::~ManualExchangeIntegralDialog()
{
    delete ui;
}

void ManualExchangeIntegralDialog::setExchangeIntegral(
    double value)
{
    if (!std::isfinite(value) ||
        value < ui->exchangeIntegralDoubleSpinBox->minimum() ||
        value > ui->exchangeIntegralDoubleSpinBox->maximum()) {
        throw std::invalid_argument(
            "Exchange integral is outside the allowed range"
            );
    }

    ui->exchangeIntegralDoubleSpinBox->setValue(value);

    ui->exchangeStackedWidget->setCurrentWidget(
        ui->constantExchangePage
        );
}

double ManualExchangeIntegralDialog::getExchangeIntegral() const
{
    return ui->exchangeIntegralDoubleSpinBox->value();
}

void ManualExchangeIntegralDialog::setMatrices(
    std::size_t ionCount,
    std::size_t neighborCount,
    const std::vector<FM::Matrix>& previous,
    double defaultExchange)
{
    if (ionCount == 0 ||
        (neighborCount != 2 &&
         neighborCount != 4 &&
         neighborCount != 6)) {
        throw std::invalid_argument(
            "Invalid ion or neighbor count"
            );
    }

    if (ionCount >
        std::numeric_limits<std::size_t>::max() / neighborCount) {
        throw std::length_error(
            "Exchange matrix count is too large"
            );
    }

    const std::size_t matrixCount =
        ionCount * neighborCount;

    if (matrixCount >
        static_cast<std::size_t>(
            std::numeric_limits<int>::max()
            )) {
        throw std::length_error(
            "Too many matrices for the table"
            );
    }

    if (!std::isfinite(defaultExchange)) {
        throw std::invalid_argument(
            "Default exchange integral must be finite"
            );
    }

    std::vector<FM::Matrix> prepared;

    if (previous.size() == matrixCount) {
        prepared = previous;
    }
    else {
        prepared.assign(matrixCount, FM::Matrix(3, 3));

        for (FM::Matrix& matrix : prepared) {
            for (std::size_t row = 0; row < 3; ++row) {
                for (std::size_t column = 0; column < 3; ++column) {
                    matrix[row * 3 + column] =
                        row == column ? defaultExchange : 0.0;
                }
            }
        }
    }

    for (const FM::Matrix& matrix : prepared) {
        if (matrix.get_rows() != 3 ||
            matrix.get_cols() != 3) {
            throw std::invalid_argument(
                "Every exchange matrix must be 3x3"
                );
        }

        for (std::size_t element = 0; element < 9; ++element) {
            if (!std::isfinite(matrix[element])) {
                throw std::invalid_argument(
                    "Exchange matrix elements must be finite"
                    );
            }
        }
    }

    matricesValue = std::move(prepared);
    neighborCountValue = neighborCount;

    QTableWidget* table = ui->exchangeMatricesTableWidget;

    table->setUpdatesEnabled(false);
    table->clearContents();
    table->setRowCount(static_cast<int>(matrixCount));

    for (int row = 0; row < table->rowCount(); ++row) {
        const std::size_t index =
            static_cast<std::size_t>(row);

        const std::size_t identifiers[] = {
            index,
            index / neighborCountValue,
            index % neighborCountValue
        };

        for (int column = 0; column < 3; ++column) {
            auto* item = new QTableWidgetItem(
                QString::number(
                    static_cast<qulonglong>(
                        identifiers[column]
                        )
                    )
                );

            item->setFlags(
                Qt::ItemIsEnabled |
                Qt::ItemIsSelectable
                );

            table->setItem(row, column, item);
        }

        updateMatrixCell(row);
    }

    table->setUpdatesEnabled(true);

    ui->exchangeStackedWidget->setCurrentWidget(
        ui->matrixExchangePage
        );
}

const std::vector<FM::Matrix>&
ManualExchangeIntegralDialog::getMatrices() const
{
    return matricesValue;
}

void ManualExchangeIntegralDialog::updateMatrixCell(int row)
{
    QTableWidget* table = ui->exchangeMatricesTableWidget;

    QTableWidgetItem* item = table->item(row, 3);

    if (item == nullptr) {
        item = new QTableWidgetItem;

        item->setFlags(
            Qt::ItemIsEnabled |
            Qt::ItemIsSelectable
            );

        table->setItem(row, 3, item);
    }

    const FM::Matrix& matrix =
        matricesValue[static_cast<std::size_t>(row)];

    QVariantList values;
    values.reserve(9);

    for (std::size_t element = 0; element < 9; ++element) {
        values.append(QVariant(matrix[element]));
    }

    item->setData(Qt::UserRole, values);
}

void ManualExchangeIntegralDialog::editMatrix(
    int row,
    int column)
{
    if (column != 3 ||
        row < 0 ||
        static_cast<std::size_t>(row) >= matricesValue.size()) {
        return;
    }

    auto* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    const std::size_t index =
        static_cast<std::size_t>(row);

    dialog->setWindowTitle(
        QStringLiteral("Matrix %1 — Ion %2, Slot %3")
            .arg(static_cast<qulonglong>(index))
            .arg(static_cast<qulonglong>(
                index / neighborCountValue
                ))
            .arg(static_cast<qulonglong>(
                index % neighborCountValue
                ))
        );

    auto* layout = new QVBoxLayout(dialog);
    auto* cells = new QTableWidget(3, 3, dialog);

    const QStringList axes{
        QStringLiteral("x"),
        QStringLiteral("y"),
        QStringLiteral("z")
    };

    cells->setHorizontalHeaderLabels(axes);
    cells->setVerticalHeaderLabels(axes);

    cells->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    cells->verticalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    cells->setSortingEnabled(false);
    cells->setDragDropMode(QAbstractItemView::NoDragDrop);

    const FM::Matrix& matrix = matricesValue[index];

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            cells->setItem(
                r,
                c,
                new QTableWidgetItem(
                    QString::number(
                        matrix[static_cast<std::size_t>(r * 3 + c)],
                        'g',
                        std::numeric_limits<double>::max_digits10
                        )
                    )
                );
        }
    }

    layout->addWidget(cells);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok |
            QDialogButtonBox::Cancel,
        dialog
        );

    layout->addWidget(buttons);

    connect(
        buttons,
        &QDialogButtonBox::rejected,
        dialog,
        &QDialog::reject
        );

    connect(
        buttons,
        &QDialogButtonBox::accepted,
        dialog,
        [this, dialog, cells, row]() {
            // Завершаем редактирование текущей ячейки.
            cells->setFocus();

            FM::Matrix result(3, 3);

            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; ++c) {
                    QTableWidgetItem* item =
                        cells->item(r, c);

                    bool valid = false;
                    double value = 0.0;

                    if (item != nullptr) {
                        value = item->text()
                        .trimmed()
                            .toDouble(&valid);
                    }

                    if (!valid || !std::isfinite(value)) {
                        QMessageBox::warning(
                            dialog,
                            QStringLiteral("Invalid matrix element"),
                            QStringLiteral(
                                "Row %1, column %2: "
                                "enter a finite number "
                                "using a decimal point."
                                )
                                .arg(r + 1)
                                .arg(c + 1)
                            );

                        cells->setCurrentCell(r, c);

                        if (item != nullptr) {
                            cells->editItem(item);
                        }

                        return;
                    }

                    result[
                        static_cast<std::size_t>(r * 3 + c)
                    ] = value;
                }
            }

            matricesValue[
                static_cast<std::size_t>(row)
            ] = std::move(result);

            updateMatrixCell(row);
            dialog->accept();
        }
        );

    dialog->resize(540, 260);
    dialog->open();
}

void ManualExchangeIntegralDialog::accept()
{
    if (ui->exchangeStackedWidget->currentWidget() ==
        ui->constantExchangePage) {
        ui->exchangeIntegralDoubleSpinBox->interpretText();

        if (!std::isfinite(getExchangeIntegral())) {
            QMessageBox::warning(
                this,
                QStringLiteral("Invalid exchange integral"),
                QStringLiteral(
                    "Exchange integral must be finite."
                    )
                );

            return;
        }
    }
    else {
        if (matricesValue.empty()) {
            QMessageBox::warning(
                this,
                QStringLiteral("Exchange matrices"),
                QStringLiteral("The matrix list is empty.")
                );

            return;
        }

        for (const FM::Matrix& matrix : matricesValue) {
            for (std::size_t element = 0; element < 9; ++element) {
                if (!std::isfinite(matrix[element])) {
                    QMessageBox::warning(
                        this,
                        QStringLiteral("Exchange matrices"),
                        QStringLiteral(
                            "Every matrix element must be finite."
                            )
                        );

                    return;
                }
            }
        }
    }

    QDialog::accept();
}
void ManualExchangeIntegralDialog::findMatrix(bool nextMatch)
{
    QTableWidget* table = ui->exchangeMatricesTableWidget;
    const int rowCount = table->rowCount();

    if (rowCount == 0) {
        QMessageBox::information(
            this,
            QStringLiteral("Find matrix"),
            QStringLiteral("The matrix list is empty.")
            );
        return;
    }

    const int column =
        ui->matrixSearchFieldComboBox->currentIndex();

    if (column < 0 || column > 2) {
        return;
    }

    ui->matrixSearchValueSpinBox->interpretText();

    const QString target = QString::number(
        ui->matrixSearchValueSpinBox->value()
        );

    int row = 0;

    if (nextMatch && table->currentRow() >= 0) {
        row = table->currentRow() + 1;

        if (row >= rowCount) {
            row = 0;
        }
    }

    for (int checked = 0; checked < rowCount; ++checked) {
        const QTableWidgetItem* item =
            table->item(row, column);

        if (item != nullptr && item->text() == target) {
            table->setCurrentCell(
                row,
                3,
                QItemSelectionModel::ClearAndSelect |
                    QItemSelectionModel::Rows
                );

            table->scrollToItem(
                table->item(row, 3),
                QAbstractItemView::PositionAtCenter
                );

            table->setFocus();
            return;
        }

        ++row;

        if (row >= rowCount) {
            row = 0;
        }
    }

    QMessageBox::information(
        this,
        QStringLiteral("Find matrix"),
        QStringLiteral("No matrix found for %1 = %2.")
            .arg(ui->matrixSearchFieldComboBox->currentText())
            .arg(target)
        );
}