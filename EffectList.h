#ifndef EFFECTLIST_H
#define EFFECTLIST_H

#include <set>

#include <QWidget>
#include <QMenu>
#include "RGBEffect.h"
#include "EffectSearch.h"

namespace Ui {
class EffectList;
}

class EffectList : public QWidget
{
    Q_OBJECT

public:
    explicit EffectList(QWidget *parent = nullptr);
    ~EffectList();

    void ShowStartStopButton(bool);    
    void EnableStartStopButton(bool);

    void AddMenu(QMenu*);
    void AddAction(QAction*);
    void AddEffectsMenus();
    void ResetMenus();

signals:
    void EffectAdded(RGBEffect*);
    void ToggleAllEffectsState();

private slots:
    void changeEvent(QEvent *event) override;
    void on_start_stop_all_button_clicked();
    void AddEffect(std::string);

private:
    Ui::EffectList *        ui;
    QMenu *                 main_menu;
    EffectSearch*           effect_search;
    std::vector<QMenu*>     sub_menus;
    std::vector<QAction*>   sub_actions;
};

#endif // EFFECTLIST_H
