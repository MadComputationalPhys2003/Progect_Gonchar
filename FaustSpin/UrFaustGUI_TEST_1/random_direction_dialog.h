#pragma once

#include <QDialog>
#include <optional>

namespace Ui {
class RandomDirectionDialog;
}

class RandomDirectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RandomDirectionDialog(QWidget* parent = nullptr);
    ~RandomDirectionDialog() override;

    void setSeed(const std::optional<int>& seed);
    std::optional<int> getSeed() const;

public slots:
    void accept() override;

private:
    Ui::RandomDirectionDialog* ui;
    std::optional<int> seedValue;
};