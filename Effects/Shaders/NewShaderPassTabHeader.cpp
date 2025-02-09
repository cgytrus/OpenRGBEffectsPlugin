#include "NewShaderPassTabHeader.h"
#include "ui_NewShaderPassTabHeader.h"

NewShaderPassTabHeader::NewShaderPassTabHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewShaderPassTabHeader)
{
    ui->setupUi(this);

    SetDynamicStrings();
}

NewShaderPassTabHeader::~NewShaderPassTabHeader()
{
    delete ui;
}

void NewShaderPassTabHeader::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        SetDynamicStrings();
    }
}

void NewShaderPassTabHeader::SetDynamicStrings()
{
    ui->type->clear();
    ui->type->addItems({tr("Texture"),
                        tr("Audio"),
                        tr("Buffer")});
}

void NewShaderPassTabHeader::on_add_clicked()
{
    emit Added(static_cast<ShaderPass::Type>(ui->type->currentIndex()));
}
