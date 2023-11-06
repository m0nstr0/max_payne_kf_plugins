/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MaxPayneKFExporter.h"
#include "MPKFExporterDesc.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID /*lpvReserved*/)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        MaxSDK::Util::UseLanguagePackLocale();
        // Hang on to this DLL's instance handle.
        hInstance = hinstDLL;
        DisableThreadLibraryCalls(hInstance);
        // DO NOT do any initialization here. Use LibInitialize() instead.
    }
    return TRUE;
}

__declspec(dllexport) const TCHAR* LibDescription() 
{    
    return _T("Max Payne KF2 Export Plugin");
}

__declspec(dllexport) int LibNumberClasses() 
{ 
    return 1; 
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i) 
{
    switch (i) {
    case 0: return GetMPKFExporterDesc();
    default: return nullptr; 
    }
}

__declspec(dllexport) int LibInitialize(void)
{
    return TRUE;
}


__declspec(dllexport) int LibShutdown(void)
{
    return TRUE;
}

__declspec(dllexport) ULONG LibVersion() 
{ 
    return VERSION_3DSMAX; 
}

TCHAR* GetString(int id)
{
    static TCHAR buf[256];
    if (hInstance)
        return LoadString(hInstance, id, buf, _countof(buf)) ? buf : NULL;
    return NULL;
}

bool IsASCII(const MCHAR* Str, size_t Length)
{
    size_t StrIndex = 0;
    while (Str && Str[StrIndex] != '\0') {
        if (Length > 0 && StrIndex >= Length) {
            return true;
        }
        if (isascii(Str[StrIndex]) == 0) {
            return false;
        }
        StrIndex++;
    }
    return true;
}
