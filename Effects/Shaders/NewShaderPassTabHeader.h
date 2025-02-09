#ifndef NEWSHADERPASSTABHEADER_H
#define NEWSHADERPASSTABHEADER_H

#include <QWidget>
#include "ShaderPass.h"

namespace Ui {
class NewShaderPassTabHeader;
}

class NewShaderPassTabHeader : public QWidget
{
    Q_OBJECT

public:
    explicit NewShaderPassTabHeader(QWidget *parent = nullptr);
    ~NewShaderPassTabHeader();

private slots:
    void changeEvent(QEvent *event) override;
    void on_add_clicked();

private:
    Ui::NewShaderPassTabHeader *ui;

    void SetDynamicStrings();

signals:
    void Added(ShaderPass::Type);
};

#endif // NEWSHADERPASSTABHEADER_H
