This project is a full-featured Memory Profiler written in C++ that instruments user code, overrides global new/delete, tracks allocations, detects memory leaks, and automatically generates a beautiful interactive HTML report.

It works across any C++ program with a main() function and provides high-level insights into:

Total allocations / deallocations

Current & peak memory usage

Leak details (address, size, timestamp, location, thread ID)

Top allocation hotspots

Severity visualization

Optimization tips

This tool helps students, developers, and testers understand real-time heap usage and catch leaks early.
