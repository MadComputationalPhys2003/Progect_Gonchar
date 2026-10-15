#ifndef MANUAL_DIRECTION_DIALOG_H
#define MANUAL_DIRECTION_DIALOG_H

#include <QDialog>
#include <cstddef>
#include <vector>

#include "../FaustSpin/Vector3D_Decart.h"

namespace Ui {
class ManualDirectionDialog;
}

class ManualDirectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManualDirectionDialog(QWidget *parent = nullptr);
    ~ManualDirectionDialog() override;

    void setDirections(
        std::size_t count,
        const std::vector<Vector3D_Decart::Vector3D_Dec>& previous
        );

    const std::vector<Vector3D_Decart::Vector3D_Dec>&
    getDirections() const;

public slots:
    void accept() override;

private:
    Ui::ManualDirectionDialog *ui;

    std::vector<Vector3D_Decart::Vector3D_Dec> directionsValue;
};

#endif // MANUAL_DIRECTION_DIALOG_H