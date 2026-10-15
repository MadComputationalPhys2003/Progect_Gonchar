#include "manual_direction_dialog.h"
#include "ui_manual_direction_dialog.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

ManualDirectionDialog::ManualDirectionDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManualDirectionDialog)
{
    ui->setupUi(this);

    ui->directionsTableWidget->setColumnCount(4);

    ui->directionsTableWidget->setHorizontalHeaderLabels(
        QStringList{
            QStringLiteral("Index"),
            QStringLiteral("nx"),
            QStringLiteral("ny"),
            QStringLiteral("nz")
        }
        );

    ui->directionsTableWidget->setSortingEnabled(false);

    ui->directionsTableWidget->setDragDropMode(
        QAbstractItemView::NoDragDrop
        );

    ui->directionsTableWidget->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::SelectedClicked |
        QAbstractItemView::EditKeyPressed
        );

    ui->directionsTableWidget->verticalHeader()->hide();

    ui->directionsTableWidget->horizontalHeader()
        ->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    for (int column = 1; column < 4; ++column) {
        ui->directionsTableWidget->horizontalHeader()
        ->setSectionResizeMode(column, QHeaderView::Stretch);
    }
}

ManualDirectionDialog::~ManualDirectionDialog()
{
    delete ui;
}

void ManualDirectionDialog::setDirections(
    std::size_t count,
    const std::vector<Vector3D_Decart::Vector3D_Dec>& previous
    )
{
    if (count == 0 ||
        count > static_cast<std::size_t>(
            std::numeric_limits<int>::max())) {
        throw std::invalid_argument(
            "Invalid number of ions for the directions table"
            );
    }

    ui->directionsTableWidget->clearContents();
    ui->directionsTableWidget->setRowCount(
        static_cast<int>(count)
        );

    const bool reusePrevious = previous.size() == count;

    const Vector3D_Decart::Vector3D_Dec defaultDirection(
        1.0, 0.0, 0.0
        );

    for (int row = 0; row < static_cast<int>(count); ++row) {
        auto* indexItem = new QTableWidgetItem(
            QString::number(row)
            );

        // Индекс можно выделить, но нельзя изменить.
        indexItem->setFlags(
            Qt::ItemIsEnabled | Qt::ItemIsSelectable
            );

        ui->directionsTableWidget->setItem(
            row, 0, indexItem
            );

        const Vector3D_Decart::Vector3D_Dec& direction =
            reusePrevious
                ? previous[static_cast<std::size_t>(row)]
                : defaultDirection;

        for (int component = 0; component < 3; ++component) {
            ui->directionsTableWidget->setItem(
                row,
                component + 1,
                new QTableWidgetItem(
                    QString::number(
                        direction[
                            static_cast<std::size_t>(component)
            ],
                        'g',
                        17
                        )
                    )
                );
        }
    }
}

const std::vector<Vector3D_Decart::Vector3D_Dec>&
ManualDirectionDialog::getDirections() const
{
    return directionsValue;
}

void ManualDirectionDialog::accept()
{
    // Завершаем редактирование активной ячейки.
    ui->directionsTableWidget->setFocus();

    const int count = ui->directionsTableWidget->rowCount();

    if (count == 0) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid directions"),
            QStringLiteral("The directions table is empty.")
            );
        return;
    }

    std::vector<Vector3D_Decart::Vector3D_Dec> result;
    result.reserve(static_cast<std::size_t>(count));

    for (int row = 0; row < count; ++row) {
        std::array<double, 3> values{};

        for (int component = 0; component < 3; ++component) {
            const int column = component + 1;

            const QTableWidgetItem* item =
                ui->directionsTableWidget->item(row, column);

            bool valid = false;

            const double value = item != nullptr
                                     ? item->text().trimmed().toDouble(&valid)
                                     : 0.0;

            if (!valid || !std::isfinite(value)) {
                ui->directionsTableWidget->setCurrentCell(
                    row, column
                    );

                QMessageBox::warning(
                    this,
                    QStringLiteral("Invalid direction"),
                    QStringLiteral(
                        "Ion %1: component %2 must be a finite "
                        "number. Use a decimal point."
                        ).arg(row).arg(component + 1)
                    );
                return;
            }

            values[static_cast<std::size_t>(component)] = value;
        }

        const double scale = std::max({
            std::abs(values[0]),
            std::abs(values[1]),
            std::abs(values[2])
        });

        if (scale == 0.0) {
            ui->directionsTableWidget->setCurrentCell(row, 1);

            QMessageBox::warning(
                this,
                QStringLiteral("Invalid direction"),
                QStringLiteral(
                    "Ion %1: a zero vector has no direction."
                    ).arg(row)
                );
            return;
        }

        // Масштабируем перед нормировкой, чтобы избежать
        // переполнения при больших компонентах.
        const double x = values[0] / scale;
        const double y = values[1] / scale;
        const double z = values[2] / scale;

        const double length = std::hypot(x, y, z);

        result.emplace_back(
            x / length,
            y / length,
            z / length
            );
    }

    // Сохраняем только полностью проверенный набор.
    directionsValue = std::move(result);

    QDialog::accept();
}