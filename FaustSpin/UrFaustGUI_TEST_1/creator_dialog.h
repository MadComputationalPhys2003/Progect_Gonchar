#ifndef CREATOR_DIALOG_H
#define CREATOR_DIALOG_H

#include <QDialog>

namespace Ui {
class CreatorDialog;
}

class CreatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreatorDialog(QWidget *parent = nullptr);
    ~CreatorDialog();

private:
    Ui::CreatorDialog *ui;
};

#endif // CREATOR_DIALOG_H
