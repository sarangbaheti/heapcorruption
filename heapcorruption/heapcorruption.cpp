// DllNotification.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <winternl.h>

#include <stdio.h>

#define NOTIFICATION_REASON_LOADED 1
#define NOTIFICATION_REASON_UNLOADED 2

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA
{
    ULONG Flags;                    //Reserved.
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA
{
    ULONG Flags;                    //Reserved.
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA
{
    LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;

typedef const union _LDR_DLL_NOTIFICATION_DATA *PCLDR_DLL_NOTIFICATION_DATA;

typedef VOID CALLBACK PLDR_DLL_NOTIFICATION_FUNCTION
(
    ULONG NotificationReason,
    PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    PVOID Context
);


typedef NTSTATUS(WINAPI *LdrRegisterDllNotificationFn)
(
    ULONG Flags,
    PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
    PVOID Context,
    PVOID *Cookie
    );


static VOID CALLBACK dllNotificationFunction
(
    ULONG NotificationReason,
    PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    PVOID Context
)
{
    //printf("length of string is- %d chars, and Length in Bytes- %d\n", (int)wcslen(NotificationData->Loaded.FullDllName->Buffer), 
    //                                          NotificationData->Loaded.FullDllName->Length);
    char utf8Str[3 * _MAX_PATH + 1] = "\0";

    //  one could argue agains not use wcslen but
    //  Length/sizeof(USHORT) or Length/sizeof(WCHAR)
    //  for this blogpost that is immaterial
    //  the fix here is to 
    //  replace:
    //      NotificationData->Loaded.FullDllName->Length
    //  by
    //  (int)wcslen(NotificationData->Loaded.FullDllName->Buffer),
    //  or 
    //  NotificationData->Loaded.FullDllName->Length/2
    //
    int nBytes = WideCharToMultiByte(CP_UTF8, 0, 
                                     NotificationData->Loaded.FullDllName->Buffer, 
                                     NotificationData->Loaded.FullDllName->Length,
                                     utf8Str, (int)sizeof(utf8Str) - 1, nullptr, nullptr);
    
    if (nBytes > 0)
    {
        utf8Str[nBytes] = '\0';
        _strlwr_s(utf8Str);
    }

    printf("reason- %s, dll- %s\n", (NotificationReason == NOTIFICATION_REASON_LOADED ? "loaded" : "unloaded"), utf8Str);
}

int main()
{
    NTSTATUS status = 1;
    HMODULE ntdll = GetModuleHandle(L"NTDLL.DLL");
    if (ntdll != NULL)
    {
        LdrRegisterDllNotificationFn fn = (LdrRegisterDllNotificationFn)GetProcAddress(ntdll, "LdrRegisterDllNotification");
        if (fn != NULL)
        {
            PVOID cookie;
            status = fn(0, dllNotificationFunction, nullptr, &cookie);
        }
    }

    printf("LdrRegisterDllNotification status is-%d\n", status);

    WCHAR* dllsToLoad = L".\\ClassLibrary.dll";

    HANDLE handle = ::LoadLibraryW(dllsToLoad);
    if (handle == NULL)
        printf("loading library failed\n");

    return 0;
}
