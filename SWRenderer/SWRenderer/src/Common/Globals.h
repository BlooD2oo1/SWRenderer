#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <vector>

#include "Graphics/Math.h"

#define SAFE_DELETE(p)       { if(p) { delete	(p);   (p)=nullptr; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=nullptr; } }

