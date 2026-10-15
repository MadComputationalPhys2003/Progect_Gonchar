#pragma once

#include <QDialog>
#include "Vector3D_Decart.h"

namespace Ui {
class ParallelDirectionDialog;
}

class ParallelDirectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParallelDirectionDialog(QWidget* parent = nullptr);
    ~ParallelDirectionDialog() override;

    void setDirection(
        const Vector3D_Decart::Vector3D_Dec& direction);

    const Vector3D_Decart::Vector3D_Dec& getDirection() const;

public slots:
    void accept() override;

private:
    Ui::ParallelDirectionDialog* ui;

    Vector3D_Decart::Vector3D_Dec directionValue{
        1.0, 0.0, 0.0
    };
};