#ifndef ORACON_LANG_RUNTIME_BUILTINS_H
#define ORACON_LANG_RUNTIME_BUILTINS_H

#include "oracon/lang/interpreter/environment.h"

namespace oracon {
namespace lang {

void registerBuiltins(Environment& env);

} // namespace lang
} // namespace oracon

#endif // ORACON_LANG_RUNTIME_BUILTINS_H
