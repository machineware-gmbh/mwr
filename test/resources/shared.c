#ifdef _MSC_VER
#define API __declspec(dllexport)
#else
#define API
#endif

API int global = 42;

API int function(int arg) {
    return arg + global;
}

// gcc -fPIC -shared -Wl,-soname,shared.so -o shared.so shared.c
// clang -arch arm64 -dynamiclib -o shared-arm64.dylib shared.c
// cl.exe /LD shared.c /link /OUT:shared-x86.dll
