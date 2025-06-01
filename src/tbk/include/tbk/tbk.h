#ifndef __TBK_H__
#define __TBK_H__
#include "tbk/core.h"
#include "tbk/log.h"
#include "tbk/posix.h"
#include "tbk/manager.h"
#include "tbk/setting.h"
namespace tbk{
void init(const std::string& _name="",const std::string& _namespace="");
} // namespace tbk
#endif // __TBK_H__
