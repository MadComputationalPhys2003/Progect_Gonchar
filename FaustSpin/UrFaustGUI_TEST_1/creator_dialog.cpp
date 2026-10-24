#include "creator_dialog.h"
#include "ui_creator_dialog.h"
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QString>

CreatorDialog::CreatorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreatorDialog)
{
    ui->setupUi(this);
    const auto setImage = [](QLabel* label, const QString& resourcePath)
    {
        label->clear();
        label->setAlignment(Qt::AlignCenter);
        label->setScaledContents(false);

        const QPixmap image(resourcePath);

        if (image.isNull()) {
            qWarning() << "Cannot load image:" << resourcePath;
            return;
        }

        label->setPixmap(
            image.scaled(
                label->contentsRect().size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    };

    // cat0.png — твоя фотография, alias в QRC: main_cat.
    setImage(ui->label_2, QStringLiteral(":/images/main_cat"));

    // cat1.jpg.
    setImage(ui->label_3, QStringLiteral(":/images/cat_1"));

    // cat4.png.
    setImage(ui->label, QStringLiteral(":/images/cat_4"));

    // cat5.png.
    setImage(ui->label_6, QStringLiteral(":/images/cat_5"));

    // cat2.png.
    setImage(ui->label_4, QStringLiteral(":/images/cat_2"));

    // cat3.png.
    setImage(ui->label_5, QStringLiteral(":/images/cat_3"));
}

CreatorDialog::~CreatorDialog()
{
    delete ui;
}
