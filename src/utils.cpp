/*
logbk - A simple C++ log backup program
Copyright (C) 2026  NakamuraSama072

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "../include/utils.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <ctime>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <filesystem>

// Check if current user is root.
bool is_root() {
    return geteuid() == 0;
}

// Check if a file or directory exists.
bool file_or_dir_exists(const std::string &path) {
    return access(path.c_str(), F_OK) == 0;
}

// Check if the path is a directory
bool is_directory(const std::string &path) {
    struct stat path_stat{};
    if (stat(path.c_str(), &path_stat) != 0) return false;
    return S_ISDIR(path_stat.st_mode);
}

// Check if the path is actually a regular file
bool is_regular_file(const std::string &path) {
    struct stat path_stat{};
    if (stat(path.c_str(), &path_stat) != 0) return false;
    return S_ISREG(path_stat.st_mode);
}

// Get current timestamp of system
std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    // Failed to get timestamp
    if (!now.time_since_epoch().count()) {
        std::cerr << "WARNING: Failed to get current system time." << 
                     " Is your system clock service running?\n";
        return "NO_TIME_AVAILABLE";
    }
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm local_time;
    // failed to convert to local time
    if (localtime_r(&now_time, &local_time) == nullptr) {
        std::cerr << "WARNING: Failed to convert time to local time.\n";
        return "CONVERSION_FAILED";
    }

    std::ostringstream oss;
    // format the time to readable format (YYYYMMDD-HH24MISS)
    oss << std::put_time(&local_time, "%Y%m%d-%H%M%S");
    return oss.str();
}

// Convert file size (in Byte) into that with human-readable units
std::string format_file_size(unsigned long size_in_bytes) {
    // TODO: Rewrite using std::array or other safe data types in STL
    const std::string size_units[] = {"B", "KiB",
                                      "MiB", "GiB",
                                      "TiB", "PiB"};
    unsigned short unit_index = 0;
    auto formatted_size = static_cast<double>(size_in_bytes);

    // Divide by 1024 until no longer divisible
    // However maximum supports to PiB
    while (formatted_size >= 1024 && unit_index < 5) {
        unit_index++;
        formatted_size = formatted_size / 1024;
    }
    // round up value (safer way)
    formatted_size = std::round(formatted_size * 100 + 0.5) / 100.0;
    std::string readable_size = std::to_string(formatted_size) +
                                " " + size_units[unit_index];
    return readable_size;
}

// Create a tar archive according to given path, file name and destination
bool create_tar_archive(const std::string &source_path, 
                        const std::string &file_name, 
                        const std::string &archive_path) {
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "ERROR: Failed to fork process.\n";
        return false;
    }
    if (pid == 0) { // child process (because parent process is root)
        execlp("tar", "tar", "-cJf", archive_path.c_str(), 
               "-C", source_path.c_str(), file_name.c_str(), nullptr);
        exit(EXIT_FAILURE);
    }
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && (WEXITSTATUS(status) == 0);
}

// Filter a file full of path and return a vector of file
std::optional<std::vector<std::string>> split_from_file(const std::string& loglist_path_str) {
    std::ifstream ifs(loglist_path_str, std::ios::binary);
    // If failed to open the file
    if (!ifs.is_open()) {
        std::cerr << "ERROR: Unable to open file " <<
            loglist_path_str << ". Is your path list broken?\n";
        return std::nullopt;
    }

    // result, each line, instringstream
    std::vector<std::string> loglist;
    std::string line;

    unsigned int line_no = 0;

    // Parse from the file, using stringstream
    while (std::getline(ifs, line)) {
        line_no++;

        std::istringstream iss(line);
        std::string file_token;

        // We should skip empty lines
        if (line.empty()) {
            std::cerr << "WARNING: Empty line detected at line " << line_no << "\n";
            continue;
        }

        // validating path (the file can be separated by whitespace or TAB)
        while (iss >> file_token) {
            //std::filesystem::path path(file_token);

            // check existence (use existing UNIX-API functions)
            if (!file_or_dir_exists(file_token)) {
                std::cerr << "WARNING: File '" << file_token
                << "' at line " << line_no << " does not exist. Skipping it.\n";
                continue;
            }
            // add it to the list
            loglist.push_back(file_token);
        }
    }
    // Nothing detected actually (all values do not exist)
    if (loglist.empty()) {
        std::cerr << "ERROR: No valid files found from " << loglist_path_str << ".\n";
        return std::nullopt;
    }
    ifs.close();
    return loglist;
}