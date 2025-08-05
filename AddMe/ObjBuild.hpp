/*
SPDX-License-Identifier: MIT
Copyright (c) 2025 Wdboyes13 
Full License can be found at THE END of this file
*/

#pragma once

// C++ Headers
#include <vector> // std::vector
#include <iostream> // std::cerr, std::cout, std::endl
#include <string> // std::string
#include <sstream> // std::istringstream
#include <filesystem> // std::filesystem::create_directory, std::filesystem::create_directories, std::filesystem::path

// C Headers
#include <cstdlib> // exit, getenv, size_t, system
#include <cstring> // strcmp
#include <cctype> // isprint

// Platform Headers
#ifdef _WIN32 
#include <windows.h> // SetCurrentDirectory
#else 
#include <unistd.h> // chdir
#endif

// These are the macros to make using ObjBuild a lot cleaner
//      It includes automatically initializing classes +
//      Easily accesing the instance +
//      The `int main(int argc, char* argv[])` function

#define B_MakeBuild \
    int main(int argc, char* argv[]) { \
    std::unique_ptr<ObjBuild> ObjB = std::make_unique<ObjBuild>(argc, argv);

#define B_DoBuild ObjB->DoBuild(); }
#define B_AddDepLib ObjB->AddDepLib
#define B_AddIncludePath ObjB->AddIncludePath
#define B_AddExecutable ObjB->AddExecutable
#define B_AddDynamicLib ObjB->AddDynamicLib
#define B_AddStaticLib ObjB->AddStaticLib
#define B_AddCompileOpt ObjB->AddCompileOpt
#define B_ProjectType ObjB->ProjectType
#define B_AddLinkOpt ObjB->AddLinkOpt
#define B_AddLibPath ObjB->AddLibPath 
#define B_Message(Message) std::cout << Message << std::endl  
#define B_Error(Message) \
        std::cerr << Message << std::endl; \
        exit(1);

#define B_CC ObjB->CC; // C Compiler
#define B_CXX ObjB->CXX; // C++ Compiler
#define B_Apple ObjB->Apple // Platform Apple
#define B_Windows ObjB->Windows// Platform Windows
#define B_Linux ObjB->Linux// Platform Linux
#define B_IsMSVC ObjB->IsMSVC

// Start of actual code

class ObjBuild {
typedef struct Target { // Target type, does NOT contain compilation info
    std::string name;
    std::vector<std::string> files;
} Target;

private:
    bool HasHead = false;
    bool IsDone = false;
    bool IsCXX = false;
    bool IsC = false;

    std::vector<std::string> SplitBySpace(const std::string& input) { 
                                                    // Split a std::string into a std::vector<std::string>
        std::vector<std::string> result;
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) {
            result.push_back(token);
        }
        return result;
    }

    bool EndsWith(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() &&
            str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
    std::string Join(const std::vector<std::string>& vec, const std::string& delim) { 
                                                    // Join an std::vector<std::string> into a single std::string
        std::string result;
        for (size_t i = 0; i < vec.size(); ++i) {
            result += vec[i];
            if (i != vec.size() - 1) {
                result += delim;
            }
        }
        return result;
    }
    void SetPlatform(){ // Set platform based on compiler macros
        #ifdef __APPLE__ 
        Apple = true;
        #endif 
        #ifdef _WIN32
        Windows = true;
        #endif
        #ifdef __gnu_linux__
        Linux = true;
        #endif
        #ifdef __linux__ 
        Linux = true;
        #endif
    }

    void CheckBeforeAdd(){ // Check that build system is initialized, and building has not ran
        if (!HasHead){
            std::cerr << "Did not initialize system yet" << std::endl;
            exit(1);
        } else if (IsDone){
            std::cerr << "Tried to add Target and/or Option after Configure Finished" << std::endl;
            exit(1);
        }
    }


    std::string StripBadChars(const std::string& input) {
        std::string clean; // temp var
        for (char ch : input) { // Loop through var and remove non-printable chars (\0 \n \t \r etc.)
            if (isprint(static_cast<unsigned char>(ch))) clean += ch;
        }

        return clean;
    }

    void InsertCMD(std::string &CurrCmd){
        if (IsCXX){                            // Check if project has been set as C++
            CurrCmd.insert(0, CXX);
        } else if (IsC) {                      // Check if project has been set as C
            CurrCmd.insert(0, CC);
        } else {
            std::cerr << "Project type not set" << std::endl; // Error if project type hasnt been set
            exit(1);
        }
    }

    void CompileAll(Target CurrentTarget, std::string CompCmd, std::vector<std::string> &Objs){
        for (int cidx = 0; cidx < CurrentTarget.files.size(); cidx++){ // Main Compile Loop
            // Vars for compiling
            auto CCompCmd = CompCmd;
            auto Filename = CurrentTarget.files.at(cidx);
            int ExtPos;
            InsertCMD(CCompCmd);

            // Make compile output name
            std::filesystem::path oname = Filename;
            if (!IsMSVC){
                oname.replace_extension(".o");
            } else {
                oname.replace_extension(".obj");
            }

            if (Windows){
                Filename.insert(0, "..\\");
            } else {
                Filename.insert(0, "../");
            }
            if (!oname.parent_path().empty()){ // Create parent directories
                std::filesystem::create_directories(oname.parent_path());
            }

            // Add filename, output name, and append output name to Objs to link
            if (!IsMSVC){
                CCompCmd.append(" -c " + Filename + " -o " + oname.c_str() + " ");
            } else if (IsMSVC && IsCXX){
                CCompCmd.append(" /EHsc /c " + Filename + " /Fo:" + oname.c_str() + " ");
            } else {
                CCompCmd.append(" /c " + Filename + " /Fo:" + oname.c_str() + " ");
            }
            Objs.push_back(oname);
            // Execute cleaned compile command
            std::cout << "COMPILE: " << Filename << std::endl;
            system(StripBadChars(CCompCmd).c_str());
        }
    }
public:

std::vector<std::string> linkopts; // Linker options
std::vector<std::string> compileopts; // Compiler options

std::vector<Target> libsotb; // .dylib's / .so's / .dll's to build
std::vector<Target> libatb; // .a's to build
std::vector<Target> exectb; // executables to build

std::string CC; // C Compiler
std::string CXX; // C++ Compiler

bool Apple = false; // Platform Apple
bool Windows = false; // Platform Windows
bool Linux = false; // Platform Linux
bool IsMSVC = false;


 void CreateBuild() { // Init ObjBuild
        // Check if on MSVC
        #ifdef _MSC_VER
            IsMSVC = true;
        #endif
        // Retrieve environment variables
        const char* ccEnv = getenv("CC");
        const char* cxxEnv = getenv("CXX");
		const char* ldEnv = getenv("LDFLAGS");
        const char* cflags = getenv("CFLAGS");
        const char* cppflags = getenv("CPPFLAGS");
        std::string cEnv; 

        // Append CFLAGS and CPPFLAGS to temp var cEnv
        if (cflags) cEnv.append(cflags);
        if (cppflags) cEnv.append(cppflags);

        // Make sure Compilers are set
        if (!ccEnv || !cxxEnv) {
            std::cerr << "CC and CXX environment variables MUST be set" << std::endl;
        }

        // Set global compiler string
        CC = std::string(ccEnv);
        CXX = std::string(cxxEnv);
        
        // Append to it for safety
        CC.append(" ");
        CXX.append(" ");

        // If LDFLAGS then 
        //      1. Strip any bad chars 
        //      2. Split By Space
        //      3. Append to `linkopts` std::vector
        if (ldEnv) {
            auto cleanLdEnv = StripBadChars(ldEnv);
            auto ldextra = SplitBySpace(cleanLdEnv);
            linkopts.insert(linkopts.end(), ldextra.begin(), ldextra.end());
        }

        // If CFLAGS / CPPFLAGS then 
        //      1. Strip any bad chars 
        //      2. Split By Space
        //      3. Append to `compileopts` std::vector
        if (!cEnv.empty()) {
            auto cleanCEnv = StripBadChars(cEnv);
            auto cextra = SplitBySpace(cleanCEnv);
            compileopts.insert(compileopts.end(), cextra.begin(), cextra.end());
        }

        // Set platform bool's
        SetPlatform();
        // Set that this function ran succesfully
        HasHead = true;
}

ObjBuild(int argc, char *argv[]) { // Initializer
    if (argc <= 2) {
        if (strcmp(argv[1], "--clean") == 0){
            std::cout << "Cleaning" << std::endl;
            std::filesystem::remove_all("OBuild");
            exit(0);
        }
    }
    std::cout << "Initializing ObjBuild" << std::endl;

    this->CreateBuild();

    std::cout << "Detected C Compiler " << CC << std::endl;
    std::cout << "Detected C++ Compiler " << CXX << std::endl;
    if (Windows) {
        std::cout << "Detected OS Windows" << std::endl;
    }
    else if (Apple) {
         std::cout << "Detected OS macOS" << std::endl;
    }
    else if (Linux) {
         std::cout << "Detected OS Linux" << std::endl;
    }

    std::cout << "Your configuration will now be processed" << std::endl;
}

 void ProjectType(std::string Type){ // Set Project Type
    CheckBeforeAdd();
    if (CC.empty() || CXX.empty()){
        std::cout << "Compiler Is Empty - Was CreateBuild() called with CC and CXX set in environment??" << std::endl;
        exit(1);
    }
    if (Type == "C"){
        IsC = true;
    } 
    else if (Type == "CXX"){
        IsCXX = true;
    }
    else {
        std::cerr << "Unknown Project Type" << std::endl;
        exit(1);
    }
}

 void AddDepLib(std::string libname){ // Adds a dependent library libcurl
    CheckBeforeAdd();
    if (!IsMSVC){
        linkopts.push_back("-l" + libname);
    } else {
        compileopts.push_back(libname + ".lib");
    }
}
 void AddLibPath(std::string libpath){ // Adds a library path
    CheckBeforeAdd();
    if (!Windows && libpath.at(0) != '/') libpath.insert(0, "../");
    if (Windows && libpath.at(1) != ':' && libpath.at(2) != '\\') libpath.insert(0, "..\\");
    if (!IsMSVC){
        linkopts.push_back("-L" + libpath);
    } else {
        linkopts.push_back("/LIBPATH:" + libpath);
    }
}
 void AddIncludePath(std::string incpath){ // Adds an include path include
    CheckBeforeAdd();
    if (!Windows && incpath.at(0) != '/') incpath.insert(0, "../");
    if (Windows && incpath.at(1) != ':' && incpath.at(2) != '\\') incpath.insert(0, "..\\");
    if (!IsMSVC){
        compileopts.push_back("-I" + incpath);
    } else {
        compileopts.push_back("/I " + incpath);
    }
}
 void AddExecutable(std::string name, std::vector<std::string> files){ // Adds an executable to build
    CheckBeforeAdd();
    exectb.push_back({name, files});
}
 void AddDynamicLib(std::string name, std::vector<std::string> files) { // Adds a dynamic library to build
    CheckBeforeAdd();
    libsotb.push_back({name, files});
}
 void AddStaticLib(std::string name, std::vector<std::string> files){ // Adds a static library to build
    CheckBeforeAdd();
    libatb.push_back({name, files});
}
void AddCompileOpt(std::string compileopt){ // Adds compile options
    CheckBeforeAdd();
    compileopts.push_back(compileopt);
}
void AddLinkOpt(std::string linkopt){ // Adds Link Options
    CheckBeforeAdd();
    if (!IsMSVC){
        linkopts.push_back("-Wl," + linkopt);
    } else {
        linkopts.push_back(linkopt);
    }
}

void DoBuild() { // Finishes Build
    IsDone = true; // Set that everything is done
    std::string ObjPath;
    if (Windows) {
        ObjPath = "Obuild\\";
    } else {
        ObjPath = "Obuild/";
    }
    std::filesystem::create_directory(ObjPath);
    #ifdef _WIN32 
    if (!SetCurrentDirectory(ObjPath)){
        std::cerr << "Failed to switch to " << ObjPath << std::endl;
        exit(1);
    }
    #else 
    if (chdir(ObjPath.c_str()) != 0){
        std::cerr << "Failed to switch to " << ObjPath << std::endl;
        exit(1);
    }
    #endif
    std::filesystem::create_directory("out");
    for (int idx = 0; idx < exectb.size(); idx++){ // Loop for executable building

        // Vars for executable building
        auto CurrentTarget = exectb.at(idx);
        std::string CompCmd;
        std::string LinkCmd;
        std::vector<std::string> Objs;
        
        // Add in `compileopts` to the right std::string
        CompCmd.append(Join(compileopts, " "));

        // Compile All
        CompileAll(CurrentTarget, CompCmd, Objs);

        // Append -o flags and add in Object filenames
        LinkCmd.append(" " + Join(Objs, " ") + " ");
        if (!IsMSVC) LinkCmd.append(" -o out/" + CurrentTarget.name + " ");
        else LinkCmd.append(" /Fe:out/" + CurrentTarget.name);
        InsertCMD(LinkCmd);
        if (Windows) LinkCmd.append(".exe "); // If we're on windows, make it a .exe
        // Insert Linker Commands
        if (IsMSVC) LinkCmd.append(" /link ");
        LinkCmd.append(Join(linkopts, " "));
        // Execute cleaned final linking command
        std::cout << "LINK EXECUTABLE: " << CurrentTarget.name << std::endl;
        system(StripBadChars(LinkCmd).c_str());
    }
    for (int idx = 0; idx < libsotb.size(); idx++){

        // Vars for library building
        auto CurrentTarget = libsotb.at(idx);
        std::string CompCmd;
        std::string LinkCmd;
        std::vector<std::string> Objs;
        
        // Add in and `compileopts` to the right std::string
        CompCmd.append(Join(compileopts, " "));

        if (Apple){ // For macOS, Compilation of a Dynamic Library needs `-fPIC`
                    // For macOS, Linking of a Dynamic Library need 
                    //      `-dynamiclib` and is Prefix with `lib` and File Extension is `.dylib`
            LinkCmd.append(" -dynamiclib  -o out/lib" + CurrentTarget.name + ".dylib ");
            CompCmd.append(" -fPIC ");
        }
        if (Linux){ // For Linux, Compilation of a Shared Object needs `-fPIC`
                    // For Linux, Linking of a Dynamic Library need 
                    //      `-shared` and is Prefix with `lib` and File Extension is `.so`
            LinkCmd.append(" -shared -o out/lib" + CurrentTarget.name + ".so ");
            CompCmd.append(" -fPIC ");
        }
        if (Windows){ // For Windows, Linking of a Dynamic Link Library needs `-shared` on things like Cygwin & MinGW
                      //        Output is file extension is `.dll`
                      //        Microsoft's Dynamic Link Libraries consist of an 
                      //                Export Lib (.dll) and Import Lib (.a / .lib)
                      //        To get an import library, we use the linker flag `--out-implib` and we prefix the
                      //                Import Library with prefix `lib` and use the File Extension `.dll.a`, unless on MSVC then we use
                      //                        File Extension `.lib` and no prefix
            if (!IsMSVC){
                LinkCmd.append(" -shared -o " + CurrentTarget.name + ".dll -Wl,--out-implib,lib" + CurrentTarget.name + ".dll.a ");
            } else {
                LinkCmd.append(" /LD /Fe:out/" + CurrentTarget.name + ".dll ");
            }
        }

        // Compile All
        CompileAll(CurrentTarget, CompCmd, Objs);

        // Append Object filenames to link command
        LinkCmd.append(" " + Join(Objs, " ") + " ");
        // Insert Linker
        InsertCMD(LinkCmd);
        // Execute cleaned final linking command

        // Insert Linker Commands
        if (IsMSVC) LinkCmd.append(" /link /IMPLIB:" + CurrentTarget.name + ".lib ");
        LinkCmd.append(Join(linkopts, " "));
        std::cout << "LINK DYNAMIC LIB: " << CurrentTarget.name << std::endl;
        system(StripBadChars(LinkCmd).c_str());
    }
    for (int idx = 0; idx < libatb.size(); idx++){

        // Vars for library building, note that LinkCmd is set immediatly, this is because
        //      building a static library uses `ar` not the linker, which `ar` is much simpler
        auto CurrentTarget = libatb.at(idx);
        std::string CompCmd;
        std::vector<std::string> Objs;
        std::string LinkCmd;
        if (!IsMSVC) LinkCmd = "ar rcs out/lib" + CurrentTarget.name + ".a ";
        else LinkCmd = "lib /OUT:out/" + CurrentTarget.name + ".lib ";

        // Add in compile options
        CompCmd.append(Join(compileopts, " "));
        
        // Compile all
        CompileAll(CurrentTarget, CompCmd, Objs);

        // Append Object filenames to link command
        LinkCmd.append(" " + Join(Objs, " ") + " ");

        // Execute cleaned link command
        std::cout << "LINK STATIC LIB: " << CurrentTarget.name << std::endl;
        system(StripBadChars(LinkCmd).c_str());
    }
}};

/*
Copyright (c) 2025 Wdboyes13  
  
Permission is hereby granted, free of charge, to any person obtaining a copy of this software (the " Software"),   
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is   
furnished to do so, subject to the following conditions:    
  
The above copyright notice and this permission notice (including the next paragraph)  
shall be included in all copies or substantial portions of the Software.  

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE   
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR  
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR   
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/