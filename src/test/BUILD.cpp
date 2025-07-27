#include "../ObjBuild.hpp"
B_MakeBuild // Init ObjBuild
B_ProjectType("C"); // Set project type to C

B_AddLibPath("lib"); // Adds library search path `lib`  
B_AddDepLib("curl"); // Adds a dependent library `libcurl`
B_AddIncludePath("include"); // Adds an include search path `include`

B_AddExecutable("MyExec", {"File1.c", "File2.c", "File3.c"}); // Adds an executable to build
                                                                            // Using files File1.c File2.c File3.c

B_AddDynamicLib("MyLib", {"File1d.c", "File2d.c", "File3d.c"}); // ----------|
                                                            // Adds a dynamic library to build     <-|
                                                            // On macOS makes "libMyLib.dylib"
                                                    // On Windows makes "MyLib.dll" + "libMyLib.dll.a" (Import Library)
                                                            // On Linux makes "libMyLib.so"
                                                            //
                                                            // Using files File1d.c File2d.c File3d.c

B_AddStaticLib("MyLib2", {"File1a.c", "File2a.c", "File3a.c"}); 
                                                            // Adds a static library to build "libMyLib2.a"
                                                            // Using files File1a.c File2a.c File3a.c

if (!B_IsMSVC){ // True if compiler is MSVC 
    B_AddCompileOpt({"-O2", "-fopenmp"}); // Adds flags to the Compiler  
} else {
    B_AddCompileOpt({"/O2", "/openmp"}); // MSVC Specific flags
}


if (B_Apple){ // True if on Apple platform
    B_AddLinkOpt("-lomp"); // Add an option for the linker `-lomp`
} else if (B_Windows) { // True on Windows platform
    B_Error("Windows not supported"); // Output an error and exit if on windows
} else if (B_Linux) { // True on Linux Platform
    B_Message("You are on Linux"); // Output a message
}

B_DoBuild // Executes Build