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
#else
#define NULL        0
#include <fstream>
#endif




#endif // SYS_CONFIG_H