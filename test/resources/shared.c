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

// cl.exe /c shared.c
// link.exe /DLL /OUT:shared.dll shared.obj
