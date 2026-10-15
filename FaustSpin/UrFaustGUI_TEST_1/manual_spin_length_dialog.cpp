#include "manual_spin_length_dialog.h"
#include "ui_manual_spin_length_dialog.h"

#include <QAbstractItemView>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QLocale>
#include <QMessageBox>
#include <QStackedWidget>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

ManualSpinLengthDialog::ManualSpinLengthDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManualSpinLengthDialog)
{
    ui->setupUi(this);

    ui->spinLengthDoubleSpinBox->setLocale(QLocale::c());

    ui->spinLengthStackedWidget->setCurrentWidget(
        ui->constantLengthPage
        );

    ui->spinLengthsTableWidget->setColumnCount(2);

    ui->spinLengthsTableWidget->setHorizontalHeaderLabels(
        QStringList{
            QStringLiteral("Index"),
            QStringLiteral("S")
        }
        );

    ui->spinLengthsTableWidget->setSortingEnabled(false);

    ui->spinLengthsTableWidget->setDragDropMode(
        QAbstractItemView::NoDragDrop
        );

    ui->spinLengthsTableWidget->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::SelectedClicked |
        QAbstractItemView::EditKeyPressed
        );

    ui->spinLengthsTableWidget->verticalHeader()->hide();

    ui->spinLengthsTableWidget->horizontalHeader()
        ->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    ui->spinLengthsTableWidget->horizontalHeader()
        ->setSectionResizeMode(1, QHeaderView::Stretch);
}

ManualSpinLengthDialog::~ManualSpinLengthDialog()
{
    delete ui;
}

void ManualSpinLengthDialog::setSpinLength(double value)
{
    if (!std::isfinite(value) ||
        value <= 0.0 ||
        value < ui->spinLengthDoubleSpinBox->minimum() ||
        value > ui->spinLengthDoubleSpinBox->maximum()) {
        throw std::invalid_argument(
            "Spin length must be positive and within the input range"
            );
    }

    ui->spinLengthDoubleSpinBox->setValue(value);

    ui->spinLengthStackedWidget->setCurrentWidget(
        ui->constantLengthPage
        );
}

double ManualSpinLengthDialog::getSpinLength() const
{
    return ui->spinLengthDoubleSpinBox->value();
}

void ManualSpinLengthDialog::setSpinLengths(
    std::size_t count,
    const std::vector<double>& previous,
    double defaultLength
    )
{
    if (count == 0 ||
        count > static_cast<std::size_t>(
            std::numeric_limits<int>::max())) {
        throw std::invalid_argument(
            "Invalid number of ions for the spin lengths table"
            );
    }

    if (!std::isfinite(defaultLength) || defaultLength <= 0.0) {
        throw std::invalid_argument(
            "Default spin length must be finite and positive"
            );
    }

    ui->spinLengthsTableWidget->clearContents();

    ui->spinLengthsTableWidget->setRowCount(
        static_cast<int>(count)
        );

    const bool reusePrevious = previous.size() == count;

    for (int row = 0; row < static_cast<int>(count); ++row) {
        auto* indexItem = new QTableWidgetItem(
            QString::number(row)
            );

        indexItem->setFlags(
            Qt::ItemIsEnabled | Qt::ItemIsSelectable
            );

        ui->spinLengthsTableWidget->setItem(
            row, 0, indexItem
            );

        const double value = reusePrevious
                                 ? previous[static_cast<std::size_t>(row)]
                                 : defaultLength;

        ui->spinLengthsTableWidget->setItem(
            row,
            1,
            new QTableWidgetItem(
                QString::number(
                    value,
                    'g',
                    std::numeric_limits<double>::max_digits10
                    )
                )
            );
    }

    ui->spinLengthStackedWidget->setCurrentWidget(
        ui->differentLengthPage
        );
}

const std::vector<double>&
ManualSpinLengthDialog::getSpinLengths() const
{
    return lengthsValue;
}

void ManualSpinLengthDialog::accept()
{
    if (ui->spinLengthStackedWidget->currentWidget() ==
        ui->constantLengthPage) {
        ui->spinLengthDoubleSpinBox->interpretText();

        const double value = getSpinLength();

        if (!std::isfinite(value) || value <= 0.0) {
            QMessageBox::warning(
                this,
                QStringLiteral("Invalid spin length"),
                QStringLiteral(
                    "Spin length must be finite and greater than zero."
                    )
                );
            return;
        }

        QDialog::accept();
        return;
    }

    ui->spinLengthsTableWidget->setFocus();

    const int count = ui->spinLengthsTableWidget->rowCount();

    if (count == 0) {
        QMessageBox::warning(
            this,
            QStringLiteral("Invalid spin lengths"),
            QStringLiteral("The spin lengths table is empty.")
            );
        return;
    }

    std::vector<double> result;
    result.reserve(static_cast<std::size_t>(count));

    for (int row = 0; row < count; ++row) {
        const QTableWidgetItem* item =
            ui->spinLengthsTableWidget->item(row, 1);

        bool valid = false;

        const double value = item != nullptr
                                 ? item->text().trimmed().toDouble(&valid)
                                 : 0.0;

        if (!valid || !std::isfinite(value) || value <= 0.0) {
            ui->spinLengthsTableWidget->setCurrentCell(row, 1);

            QMessageBox::warning(
                this,
                QStringLiteral("Invalid spin length"),
                QStringLiteral(
                    "Ion %1: enter a finite positive spin length "
                    "using a decimal point."
                    ).arg(row)
                );
            return;
        }

        result.push_back(value);
    }

    lengthsValue = std::move(result);

    QDialog::accept();
}