/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <max.h>
#include <3dsmaxport.h>
#include "resource.h"

extern HINSTANCE hInstance;

TCHAR* GetString(int id);