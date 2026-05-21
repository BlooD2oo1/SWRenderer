#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <algorithm>

#define SAFE_DELETE(p)       { if(p) { delete	(p);   (p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=nullptr; } }

