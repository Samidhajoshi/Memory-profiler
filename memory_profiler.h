// ==================== MEMORY_PROFILER.H ====================
// Save this as: memory_profiler.h

#ifndef MEMORY_PROFILER_H
#define MEMORY_PROFILER_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <algorithm>

struct AllocationInfo {
    void* address;
    size_t size;
    std::string file;
    int line;
    std::string timestamp;
    int threadId;
    bool freed;
    
    AllocationInfo() : address(NULL), size(0), line(0), threadId(0), freed(false) {}
};

class MemoryProfiler {
private:
    std::map<void*, AllocationInfo> allocations;
    
    size_t totalAllocations;
    size_t totalDeallocations;
    size_t currentMemoryUsage;
    size_t peakMemoryUsage;
    
    std::map<std::string, size_t> allocationSites;
    std::vector<AllocationInfo> leakList;
    
    std::string getCurrentTimestamp() {
        time_t now = time(0);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return std::string(buffer);
    }
    
    MemoryProfiler() : totalAllocations(0), totalDeallocations(0), 
                       currentMemoryUsage(0), peakMemoryUsage(0) {}
    
public:
    static MemoryProfiler& getInstance() {
        static MemoryProfiler instance;
        return instance;
    }
    
    void recordAllocation(void* ptr, size_t size, const char* file, int line) {
        AllocationInfo info;
        info.address = ptr;
        info.size = size;
        info.file = file ? file : "unknown";
        info.line = line;
        info.timestamp = getCurrentTimestamp();
        info.threadId = 1;
        info.freed = false;
        
        allocations[ptr] = info;
        totalAllocations++;
        currentMemoryUsage += size;
        if (currentMemoryUsage > peakMemoryUsage) {
            peakMemoryUsage = currentMemoryUsage;
        }
        
        std::string site = std::string(file ? file : "unknown") + ":" + 
                          static_cast<std::ostringstream&>(std::ostringstream() << line).str();
        allocationSites[site]++;
    }
    
    void recordDeallocation(void* ptr) {
        std::map<void*, AllocationInfo>::iterator it = allocations.find(ptr);
        if (it != allocations.end()) {
            currentMemoryUsage -= it->second.size;
            it->second.freed = true;
            totalDeallocations++;
        }
    }
    
    void detectLeaks() {
        leakList.clear();
        
        for (std::map<void*, AllocationInfo>::iterator it = allocations.begin(); 
             it != allocations.end(); ++it) {
            if (!it->second.freed) {
                leakList.push_back(it->second);
            }
        }
    }
    
    void generateHTMLReport(const std::string& filename = "memory_report.html") {
        detectLeaks();
        
        std::ofstream file(filename.c_str());
        
        file << "<!DOCTYPE html>\n";
        file << "<html lang=\"en\">\n";
        file << "<head>\n";
        file << "    <meta charset=\"UTF-8\">\n";
        file << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        file << "    <title>Memory Profiler Report</title>\n";
        file << "    <style>\n";
        file << "        * { margin: 0; padding: 0; box-sizing: border-box; }\n";
        file << "        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;\n";
        file << "               background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n";
        file << "               padding: 20px; min-height: 100vh; }\n";
        file << "        .container { max-width: 1400px; margin: 0 auto; }\n";
        file << "        .header { background: white; padding: 30px; border-radius: 15px;\n";
        file << "                  box-shadow: 0 10px 30px rgba(0,0,0,0.2); margin-bottom: 20px; text-align: center; }\n";
        file << "        .header h1 { color: #667eea; font-size: 2.5em; margin-bottom: 10px; }\n";
        file << "        .header .subtitle { color: #666; font-size: 1.1em; }\n";
        file << "        .stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));\n";
        file << "                      gap: 20px; margin-bottom: 20px; }\n";
        file << "        .stat-card { background: white; padding: 25px; border-radius: 15px;\n";
        file << "                     box-shadow: 0 5px 15px rgba(0,0,0,0.1); transition: transform 0.3s; }\n";
        file << "        .stat-card:hover { transform: translateY(-5px); box-shadow: 0 10px 25px rgba(0,0,0,0.2); }\n";
        file << "        .stat-card .icon { font-size: 2em; margin-bottom: 10px; }\n";
        file << "        .stat-card .label { color: #666; font-size: 0.9em; margin-bottom: 5px; }\n";
        file << "        .stat-card .value { font-size: 2em; font-weight: bold; color: #333; }\n";
        file << "        .leak-alert { background: #fff3cd; border-left: 5px solid #ffc107;\n";
        file << "                      padding: 20px; border-radius: 10px; margin-bottom: 20px; }\n";
        file << "        .leak-critical { background: #f8d7da; border-left: 5px solid #dc3545; }\n";
        file << "        .section { background: white; padding: 30px; border-radius: 15px;\n";
        file << "                   box-shadow: 0 5px 15px rgba(0,0,0,0.1); margin-bottom: 20px; }\n";
        file << "        .section h2 { color: #667eea; margin-bottom: 20px; padding-bottom: 10px;\n";
        file << "                      border-bottom: 3px solid #667eea; }\n";
        file << "        table { width: 100%; border-collapse: collapse; margin-top: 15px; }\n";
        file << "        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }\n";
        file << "        th { background: #667eea; color: white; font-weight: 600; }\n";
        file << "        tr:hover { background: #f5f5f5; }\n";
        file << "        .progress-bar { width: 100%; height: 30px; background: #e0e0e0;\n";
        file << "                        border-radius: 15px; overflow: hidden; margin: 10px 0; }\n";
        file << "        .progress-fill { height: 100%; background: linear-gradient(90deg, #667eea, #764ba2);\n";
        file << "                         display: flex; align-items: center; justify-content: center;\n";
        file << "                         color: white; font-weight: bold; transition: width 0.3s; }\n";
        file << "        .badge { display: inline-block; padding: 5px 10px; border-radius: 20px;\n";
        file << "                 font-size: 0.85em; font-weight: 600; }\n";
        file << "        .badge-danger { background: #dc3545; color: white; }\n";
        file << "        .badge-warning { background: #ffc107; color: #333; }\n";
        file << "        .badge-success { background: #28a745; color: white; }\n";
        file << "    </style>\n";
        file << "</head>\n";
        file << "<body>\n";
        file << "    <div class=\"container\">\n";
        file << "        <div class=\"header\">\n";
        file << "            <h1>🔍 Memory Profiler Report</h1>\n";
        file << "            <p class=\"subtitle\">Comprehensive Memory Analysis & Leak Detection</p>\n";
        file << "            <p class=\"subtitle\">Generated: " << getCurrentTimestamp() << "</p>\n";
        file << "        </div>\n";
        
        // Statistics Cards
        file << "        <div class=\"stats-grid\">\n";
        file << "            <div class=\"stat-card\"><div class=\"icon\">📊</div>\n";
        file << "                <div class=\"label\">Total Allocations</div>\n";
        file << "                <div class=\"value\">" << totalAllocations << "</div></div>\n";
        file << "            <div class=\"stat-card\"><div class=\"icon\">✅</div>\n";
        file << "                <div class=\"label\">Total Deallocations</div>\n";
        file << "                <div class=\"value\">" << totalDeallocations << "</div></div>\n";
        file << "            <div class=\"stat-card\"><div class=\"icon\">💾</div>\n";
        file << "                <div class=\"label\">Current Usage</div>\n";
        file << "                <div class=\"value\">" << (currentMemoryUsage / 1024.0) << " KB</div></div>\n";
        file << "            <div class=\"stat-card\"><div class=\"icon\">📈</div>\n";
        file << "                <div class=\"label\">Peak Usage</div>\n";
        file << "                <div class=\"value\">" << (peakMemoryUsage / 1024.0) << " KB</div></div>\n";
        file << "            <div class=\"stat-card\"><div class=\"icon\">⚠️</div>\n";
        file << "                <div class=\"label\">Memory Leaks</div>\n";
        file << "                <div class=\"value\">" << leakList.size() << "</div></div>\n";
        file << "            <div class=\"stat-card\"><div class=\"icon\">🎯</div>\n";
        file << "                <div class=\"label\">Active Allocations</div>\n";
        file << "                <div class=\"value\">" << (totalAllocations - totalDeallocations) << "</div></div>\n";
        file << "        </div>\n";
        
        // Leak Alert
        if (!leakList.empty()) {
            size_t leakedBytes = 0;
            for (size_t i = 0; i < leakList.size(); i++) {
                leakedBytes += leakList[i].size;
            }
            
            std::string alertClass = leakedBytes > 10240 ? "leak-alert leak-critical" : "leak-alert";
            file << "        <div class=\"" << alertClass << "\">\n";
            file << "            <h3>⚠️ Memory Leak Detected!</h3>\n";
            file << "            <p><strong>" << leakList.size() << "</strong> allocations were not freed, totaling <strong>" 
                 << (leakedBytes / 1024.0) << " KB</strong> of leaked memory.</p>\n";
            file << "        </div>\n";
        } else {
            file << "        <div class=\"leak-alert\" style=\"background: #d4edda; border-color: #28a745;\">\n";
            file << "            <h3>✅ No Memory Leaks Detected!</h3>\n";
            file << "            <p>All allocations have been properly freed.</p>\n";
            file << "        </div>\n";
        }
        
        // Memory Leaks Table
        if (!leakList.empty()) {
            file << "        <div class=\"section\">\n";
            file << "            <h2>🔴 Detected Memory Leaks</h2>\n";
            file << "            <table><thead><tr><th>Address</th><th>Size</th><th>Location</th>\n";
            file << "                <th>Timestamp</th><th>Thread ID</th><th>Severity</th></tr></thead><tbody>\n";
            
            for (size_t i = 0; i < leakList.size(); i++) {
                const AllocationInfo& leak = leakList[i];
                std::string severity = leak.size > 1024 ? "Major" : "Minor";
                std::string badgeClass = leak.size > 1024 ? "badge-danger" : "badge-warning";
                
                file << "                    <tr><td><code>" << leak.address << "</code></td>\n";
                file << "                        <td>" << leak.size << " bytes</td>\n";
                file << "                        <td>" << leak.file << ":" << leak.line << "</td>\n";
                file << "                        <td>" << leak.timestamp << "</td><td>" << leak.threadId << "</td>\n";
                file << "                        <td><span class=\"badge " << badgeClass << "\">" 
                     << severity << "</span></td></tr>\n";
            }
            
            file << "                </tbody></table></div>\n";
        }
        
        // Allocation Sites
        file << "        <div class=\"section\"><h2>📍 Top Allocation Sites</h2>\n";
        file << "            <table><thead><tr><th>Location</th><th>Allocation Count</th>\n";
        file << "                <th>Frequency</th></tr></thead><tbody>\n";
        
        std::vector<std::pair<std::string, size_t> > sortedSites(allocationSites.begin(), allocationSites.end());
        
        for (size_t i = 0; i < sortedSites.size(); i++) {
            for (size_t j = 0; j < sortedSites.size() - 1 - i; j++) {
                if (sortedSites[j].second < sortedSites[j + 1].second) {
                    std::pair<std::string, size_t> temp = sortedSites[j];
                    sortedSites[j] = sortedSites[j + 1];
                    sortedSites[j + 1] = temp;
                }
            }
        }
        
        size_t displayCount = sortedSites.size() < 10 ? sortedSites.size() : 10;
        for (size_t i = 0; i < displayCount; i++) {
            double percentage = (sortedSites[i].second * 100.0) / totalAllocations;
            file << "                    <tr><td><code>" << sortedSites[i].first << "</code></td>\n";
            file << "                        <td>" << sortedSites[i].second << "</td><td>\n";
            file << "                            <div class=\"progress-bar\" style=\"height: 20px;\">\n";
            file << "                                <div class=\"progress-fill\" style=\"width: " 
                 << percentage << "%; font-size: 0.8em;\">" << percentage << "%</div></div></td></tr>\n";
        }
        
        file << "                </tbody></table></div>\n";
        
        file << "        <div class=\"section\"><h2>💡 Optimization Tips</h2>\n";
        file << "            <ul style=\"line-height: 2;\">\n";
        file << "                <li>✓ Review allocation sites with high frequency for optimization opportunities</li>\n";
        file << "                <li>✓ Consider using object pools for frequently allocated/deallocated objects</li>\n";
        file << "                <li>✓ Use smart pointers to prevent leaks</li>\n";
        file << "                <li>✓ Profile your application regularly during development</li>\n";
        file << "                <li>✓ Enable compiler warnings for memory-related issues</li>\n";
        file << "            </ul></div></div></body></html>\n";
        
        file.close();
        std::cout << "\n HTML report generated: " << filename << std::endl;
        std::cout << "Open it with: start " << filename << "\n\n";
    }
    
    void printSummary() {
        std::cout << "\n========================================\n";
        std::cout << "    MEMORY PROFILER SUMMARY\n";
        std::cout << "========================================\n\n";
        std::cout << "Total Allocations:   " << totalAllocations << "\n";
        std::cout << "Total Deallocations: " << totalDeallocations << "\n";
        std::cout << "Current Usage:       " << (currentMemoryUsage / 1024.0) << " KB\n";
        std::cout << "Peak Usage:          " << (peakMemoryUsage / 1024.0) << " KB\n";
        std::cout << "Memory Leaks:        " << leakList.size() << "\n";
        std::cout << "========================================\n\n";
    }
};

// Override new/delete operators
void* operator new(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (ptr) {
        MemoryProfiler::getInstance().recordAllocation(ptr, size, file, line);
    }
    return ptr;
}

void* operator new[](size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    if (ptr) {
        MemoryProfiler::getInstance().recordAllocation(ptr, size, file, line);
    }
    return ptr;
}

void operator delete(void* ptr) throw() {
    if (ptr) {
        MemoryProfiler::getInstance().recordDeallocation(ptr);
        free(ptr);
    }
}

void operator delete[](void* ptr) throw() {
    if (ptr) {
        MemoryProfiler::getInstance().recordDeallocation(ptr);
        free(ptr);
    }
}

#define new new(__FILE__, __LINE__)

#endif 
