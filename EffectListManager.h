/*---------------------------------------------------------*\
| EffectListManager.h                                       |
|                                                           |
|   Single-instance class to manage registration of effects |
|   and effects list                                        |
|                                                           |
|   Adam Honse (CalcProgrammer1)                10 Aug 2024 |
|                                                           |
|   This file is part of the OpenRGB Effects Plugin project |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <functional>
#include <string>
#include "RGBEffect.h"

class EffectListManager
{
public:
    static EffectListManager * get();

    EffectListManager();
    ~EffectListManager();
    
    std::map<std::string, std::vector<std::string>>     GetCategorizedEffects();
    std::function<RGBEffect*()>                         GetEffectConstructor(std::string name);
    std::size_t                                         GetEffectsListSize();

    void                                                RegisterEffect(std::string, std::string, std::function<RGBEffect*()>);

private:
    static EffectListManager*                           instance;

    std::map<std::string, std::function<RGBEffect*()>>  effects_constructors;
    std::map<std::string, std::vector<std::string>>     categorized_effects;
};