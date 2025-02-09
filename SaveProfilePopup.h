#ifndef SAVEPROFILEPOPUP_H
#define SAVEPROFILEPOPUP_H

#include <QWidget>

namespace Ui {
class SaveProfilePopup;
}

class SaveProfilePopup : public QWidget
{
    Q_OBJECT

public:
    explicit SaveProfilePopup(QWidget *parent = nullptr);
    ~SaveProfilePopup();

    QString Filename();
    bool ShouldLoadAtStartup();
    bool ShouldSaveEffectsState();

    void SetFileName(QString);

signals:
    void Accept();
    void Reject();

private slots:
    void changeEvent(QEvent *event) override;
    void on_save_clicked();
    void on_cancel_clicked();

private:
    Ui::SaveProfilePopup *ui;
};

#endif // SAVEPROFILEPOPUP_H
