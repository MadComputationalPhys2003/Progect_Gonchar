#include "manual_single_ion_anisotropy_dialog.h"
#include "ui_manual_single_ion_anisotropy_dialog.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QKeyEvent>
#include <QTableWidget>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>
namespace {
class MatrixCellEdit final : public QLineEdit
{
public:
    MatrixCellEdit(QTableWidget *table, int row, int column)
        : QLineEdit(table), table_(table), row_(row), column_(column)
    {
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        const auto modifiers = event->modifiers();
        if (modifiers != Qt::NoModifier && modifiers != Qt::KeypadModifier) {
            QLineEdit::keyPressEvent(event);
            return;
        }

        int row = row_;
        int column = column_;

        switch (event->key()) {
        case Qt::Key_Up:    --row;    break;
        case Qt::Key_Down:  ++row;    break;
        case Qt::Key_Left:  --column; break;
        case Qt::Key_Right: ++column; break;
        default:
            QLineEdit::keyPressEvent(event);
            return;
        }

        if (auto *next = table_->cellWidget(row, column)) {
            table_->setCurrentCell(row, column);
            next->setFocus(Qt::OtherFocusReason);
        }

        event->accept();
    }

private:
    QTableWidget *table_;
    int row_;
    int column_;
};
} // namespace
ManualSingleIonAnisotropyDialog::ManualSingleIonAnisotropyDialog(
    std::vector<FM::Matrix> matrices, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::ManualSingleIonAnisotropyDialog>())
    , matrices_(std::move(matrices))
{
    if (matrices_.empty() ||
        matrices_.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::length_error("Invalid number of ions for the manual editor");
    }

    ui->setupUi(this);
    setWindowTitle(tr("Manual Single Ion Anisotropy"));

    auto *table = ui->tableWidget;
    const QStringList axes{"x", "y", "z"};
    table->setRowCount(3);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels(axes);
    table->setVerticalHeaderLabels(axes);
    table->setSortingEnabled(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            auto *entry = new MatrixCellEdit(table, row, column);
            entry->setFrame(false);
            entry->setAlignment(Qt::AlignCenter);
            table->setCellWidget(row, column, entry);
            entries_[3 * row + column] = entry;
        }
    }

    ui->anisotropyIonSpinBox->setRange(0, static_cast<int>(matrices_.size()) - 1);
    ui->anisotropyIonSpinBox->setKeyboardTracking(false);
    connect(ui->anisotropyIonSpinBox, &QSpinBox::valueChanged, this, [this](int index) {
        if (saveMatrix()) {
            showIon(index);
        } else {
            const QSignalBlocker blocker(ui->anisotropyIonSpinBox);
            ui->anisotropyIonSpinBox->setValue(ion_);
        }
    });

    showIon(0);
}

ManualSingleIonAnisotropyDialog::~ManualSingleIonAnisotropyDialog() = default;

std::vector<FM::Matrix> ManualSingleIonAnisotropyDialog::takeMatrices()
{
    return std::move(matrices_);
}

void ManualSingleIonAnisotropyDialog::accept()
{
    if (saveMatrix()) {
        QDialog::accept();
    }
}

void ManualSingleIonAnisotropyDialog::showIon(int index)
{
    ion_ = index;
    const auto &matrix = matrices_[ion_];
    for (std::size_t k = 0; k < entries_.size(); ++k) {
        entries_[k]->setText(QString::number(
            matrix[k], 'g', std::numeric_limits<double>::max_digits10));
    }
}

bool ManualSingleIonAnisotropyDialog::saveMatrix()
{
    std::array<double, 9> values{};
    for (std::size_t k = 0; k < entries_.size(); ++k) {
        auto *entry = entries_[k];
        bool ok = false;
        values[k] = entry->text().trimmed().toDouble(&ok);
        if (!ok || !std::isfinite(values[k])) {
            QMessageBox::warning(this, windowTitle(),
                                 tr("Enter a finite number. Use a decimal point."));
            entry->setFocus();
            entry->selectAll();
            return false;
        }
    }

    auto &matrix = matrices_[ion_];
    for (std::size_t k = 0; k < values.size(); ++k) {
        matrix[k] = values[k];
    }
    return true;
}