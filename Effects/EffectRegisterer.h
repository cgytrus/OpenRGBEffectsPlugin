#ifndef EFFECTREGISTERER_H
#define EFFECTREGISTERER_H

#include "EffectListManager.h"

#define EFFECT_REGISTERER(_classname, _ui_name, _category, _constructor)                                \
    static class _register                                                                              \
    {                                                                                                   \
     public:                                                                                            \
       _register()                                                                                      \
       {                                                                                                \
           EffectListManager::get()->RegisterEffect(_classname, _ui_name, _category ,_constructor);     \
        }                                                                                               \
    } _registerer;                                                                                      \

#define REGISTER_EFFECT(T) T::_register T::_registerer;

#endif // EFFECTREGISTERER_H
