#include <Windows.h>

//  sample impl from:
//  https://msdn.microsoft.com/en-us/library/aa370448(v=vs.85).aspx
//

BOOLEAN WINAPI DllMain(IN HINSTANCE hDllHandle,
    IN DWORD     nReason,
    IN LPVOID    Reserved)
{
    BOOLEAN bSuccess = TRUE;


    //  Perform global initialization.

    switch (nReason)
    {
    case DLL_PROCESS_ATTACH:

        //  For optimization.

        DisableThreadLibraryCalls(hDllHandle);

        break;

    case DLL_PROCESS_DETACH:

        break;
    }

    return bSuccess;

}

