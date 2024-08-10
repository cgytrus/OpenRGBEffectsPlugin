/*---------------------------------------------------------*\
| EffectListManager.cpp                                     |
|                                                           |
|   Single-instance class to manage registration of effects |
|   and effects list                                        |
|                                                           |
|   Adam Honse (CalcProgrammer1)                10 Aug 2024 |
|                                                           |
|   This file is part of the OpenRGB Effects Plugin project |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "EffectListManager.h"

EffectListManager* EffectListManager::instance;

EffectListManager *EffectListManager::get()
{
    if(!instance)
    {
        instance = new EffectListManager();
    }

    return instance;
}

EffectListManager::EffectListManager()
{

}

EffectListManager::~EffectListManager()
{

}

std::map<std::string, std::vector<std::string>> EffectListManager::GetCategorizedEffects()
{
    return(categorized_effects);
}

std::function<RGBEffect*()> EffectListManager::GetEffectConstructor(std::string name)
{
    return(effects_constructors[name]);
}

std::size_t EffectListManager::GetEffectsListSize()
{
    return(effects_constructors.size());
}

void EffectListManager::RegisterEffect(std::string name, std::string category, std::function<RGBEffect*()> constructor)
{    
    effects_constructors[name] = constructor;

    if(!categorized_effects.count(category))
    {
        std::vector<std::string> effects;
        categorized_effects[category] = effects;
    }

    categorized_effects[category].push_back(name);
}