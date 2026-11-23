#pragma once

#include "Faust math/Vector3D_Decart.h"

#include <QDialog>
#include <QString>

#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

namespace Ui {
class SpinParametersDialog;
}

class SpinParametersDialog : public QDialog
{
    Q_OBJECT

public:
    enum class DirectionMode { parallel, random, file, manual };
    enum class LengthInput { manual, random, file };

    using Vector3 = Vector3D_Decart::Vector3D_Dec;

    struct Settings {
        DirectionMode directionMode{DirectionMode::parallel};
        Vector3 parallelDirection{1.0, 0.0, 0.0};
        std::optional<int> directionSeed;
        std::vector<Vector3> ionDirections;
        QString directionFile;

        LengthInput lengthInput{LengthInput::manual};
        std::variant<double, std::vector<double>> spinLength{1.0};
        double randomMinimum{0.5};
        double randomMaximum{1.5};
        QString lengthFile;
    };

    explicit SpinParametersDialog(QWidget *parent = nullptr);
    SpinParametersDialog(std::size_t ionCount, const Settings &previous,
                         QWidget *parent = nullptr);
    ~SpinParametersDialog() override;

    const Settings &settings() const;

    void accept() override;

private:
    DirectionMode directionMode() const;
    LengthInput lengthInput() const;
    bool differentLengths() const;
    bool directionsReady() const;
    bool lengthsReady() const;
    std::size_t requireIonCount() const;

    void updateControls();
    void configureDirections();
    void configureLengths();

    Ui::SpinParametersDialog *ui;
    std::size_t ionCount_;
    Settings settings_;
};
