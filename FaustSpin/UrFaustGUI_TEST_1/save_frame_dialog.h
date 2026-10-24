#ifndef SAVE_FRAME_DIALOG_H
#define SAVE_FRAME_DIALOG_H

#include <QColor>
#include <QDialog>
#include <QPointer>

    class SpinOpenGLWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class SaveFrameDialog;
}
QT_END_NAMESPACE

class SaveFrameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SaveFrameDialog(QWidget* parent = nullptr);
    ~SaveFrameDialog() override;

    void setFrameSource(
        SpinOpenGLWidget* widget,
        int frameIndex
        );

public slots:
    void accept() override;

private:
    Ui::SaveFrameDialog* ui;

    QPointer<SpinOpenGLWidget> frameWidget;
    int frameIndex = 0;

    QColor backgroundColor =
        QColor::fromRgbF(0.08, 0.10, 0.14);

    void chooseBackgroundColor();
    void updateBackgroundPreview();
};

#endif // SAVE_FRAME_DIALOG_H