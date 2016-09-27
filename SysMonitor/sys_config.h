#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include "json.h"
using namespace Json;
#include <string>
using std::string;

#include <map>
#include <vector>
#include <list>
using namespace std;

#include "func.h"

#ifndef WIN32
#define  SSIZE_T unsigned int
#endif


#define NULL        0

#endif // SYS_CONFIG_H