#include "NewShaderPassTabHeader.h"
#include "ui_NewShaderPassTabHeader.h"

NewShaderPassTabHeader::NewShaderPassTabHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NewShaderPassTabHeader)
{
    ui->setupUi(this);

    ui->type->addItems({"Texture", "Audio", "Buffer"});
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
    }
}

void NewShaderPassTabHeader::on_add_clicked()
{
    emit Added(static_cast<ShaderPass::Type>(ui->type->currentIndex()));
}
