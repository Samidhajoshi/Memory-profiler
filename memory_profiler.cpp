include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include"memory_profiler.h"

void displayBanner() {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "                                                            \n";
    std::cout << "       C++ MEMORY PROFILER & LEAK DETECTOR                 \n";
    std::cout << "                                                            \n";
    std::cout << "          Analyze Memory Usage & Find Leaks                \n";
    std::cout << "                                                            \n";
    std::cout << "============================================================\n";
    std::cout << "\n";
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename.c_str());
    return file.good();
}

std::string getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of(".");
    if (dotPos != std::string::npos) {
        return filename.substr(dotPos);
    }
    return "";
}

int main() {
    displayBanner();
    
    std::string filename;
    std::cout << "[*] Enter the C++ file to analyze: ";
    std::getline(std::cin, filename);
    
    // Trim whitespace
    size_t start = filename.find_first_not_of(" \t\n\r");
    size_t end = filename.find_last_not_of(" \t\n\r");
    if (start != std::string::npos && end != std::string::npos) {
        filename = filename.substr(start, end - start + 1);
    }
    
    // Check if file exists
    if (!fileExists(filename)) {
        std::cout << "\n[ERROR] File '" << filename << "' not found!\n";
        std::cout << "        Please check the filename and try again.\n\n";
        return 1;
    }
    
    // Check file extension
    std::string ext = getFileExtension(filename);
    if (ext != ".cpp" && ext != ".cc" && ext != ".cxx" && ext != ".c++") {
        std::cout << "\n[WARNING] File doesn't have a C++ extension (.cpp, .cc, .cxx)\n";
        std::cout << "          Proceeding anyway...\n\n";
    }
    
    std::cout << "\n[OK] File found: " << filename << "\n";
    std::cout << "\n[*] Instrumenting code with memory profiler...\n";
    
    // Create instrumented version
    std::string instrumentedFile = "instrumented_" + filename;
    std::ifstream inFile(filename.c_str());
    std::ofstream outFile(instrumentedFile.c_str());
    
    // Add profiler header at the beginning
    outFile << "#include \"memory_profiler.h\"\n";
    
    // Copy original file content
    std::string line;
    bool hasMain = false;
    std::vector<std::string> fileContent;
    
    while (std::getline(inFile, line)) {
        // Check if this file already has the profiler
        if (line.find("#include \"memory_profiler.h\"") != std::string::npos) {
            std::cout << "\nWarning: File already includes memory_profiler.h\n";
            std::cout << "   Using file as-is...\n";
            inFile.close();
            outFile.close();
            remove(instrumentedFile.c_str());
            instrumentedFile = filename;
            hasMain = true;
            break;
        }
        
        if (line.find("int main") != std::string::npos) {
            hasMain = true;
        }
        
        fileContent.push_back(line);
    }
    
    if (instrumentedFile != filename) {
        // Write all content except the last closing brace of main
        bool inMain = false;
        int braceCount = 0;
        
        for (size_t i = 0; i < fileContent.size(); i++) {
            std::string& currentLine = fileContent[i];
            
            if (currentLine.find("int main") != std::string::npos) {
                inMain = true;
            }
            
            // Count braces when in main
            if (inMain) {
                for (size_t j = 0; j < currentLine.length(); j++) {
                    if (currentLine[j] == '{') braceCount++;
                    if (currentLine[j] == '}') braceCount--;
                }
            }
            
            // If we're at the final return statement in main, add profiler code before it
            if (inMain && braceCount == 1 && currentLine.find("return") != std::string::npos) {
                outFile << "    // Memory Profiler Report Generation\n";
                outFile << "    MemoryProfiler::getInstance().detectLeaks();\n";
                outFile << "    MemoryProfiler::getInstance().printSummary();\n";
                outFile << "    MemoryProfiler::getInstance().generateHTMLReport();\n";
                outFile << "    \n";
            }
            
            outFile << currentLine << "\n";
        }
        
        inFile.close();
        outFile.close();
    }
    
    if (!hasMain) {
        std::cout << "\n  Warning: No main() function found in the file.\n";
        std::cout << "   The profiler needs a main() function to generate reports.\n\n";
        return 1;
    }
    
    // Compile the instrumented file
    std::cout << "\n Compiling instrumented code...\n";
    std::string outputExe = "profiled_program";
    std::string compileCmd = "g++ \"" + instrumentedFile + "\" -o " + outputExe;
    
    int compileResult = system(compileCmd.c_str());
    
    if (compileResult != 0) {
        std::cout << "\n Compilation failed!\n";
        std::cout << "   Please check the error messages above.\n\n";
        return 1;
    }
    
    std::cout << " Compilation successful!\n";
    std::cout << "\n Running memory analysis...\n";
    
    // Run the program
#ifdef _WIN32
    system(outputExe.c_str());
#else
    system(("./" + outputExe).c_str());
#endif
    std::cout << "\n Analysis Complete!\n";
    std::cout << "\n To view the detailed report, run:\n";
    std::cout << "   start memory_report.html    (Windows)\n";
    std::cout << "   open memory_report.html     (macOS)\n";
    std::cout << "   xdg-open memory_report.html (Linux)\n\n";
    
    // Clean up instrumented file if it was created
    if (instrumentedFile != filename) {
        remove(instrumentedFile.c_str());
    }
    
    return 0;
}
