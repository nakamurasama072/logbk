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

#include "../include/bktool.h"
#include "../include/utils.h"

#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>

const std::string LOG_BACKUP_DIR = "/root/syslogbackup";
const mode_t DEFAULT_MODE = 0755;

// clear original log
bool log_cleanup(const std::string &path_to_log) {
    // ios::trunc means log_ostream will truncate (clean) the file
    std::ofstream log_ostream(path_to_log, std::ios::trunc);
    if (!log_ostream.is_open()) {
        std::cerr << "ERROR: Unable to open log file " 
                << path_to_log << " for cleaning.\n";
        return EXECUTION_FAILED;
    }
    log_ostream.close();
    return EXECUTION_SUCCESS;
}

// Create a directory with designated mode
bool create_directory(const std::string &dir_path, mode_t mode = DEFAULT_MODE) {
    // Check if a directory exists and it is a directory
    if (file_or_dir_exists(dir_path)) {
        if (!is_directory(dir_path)) {
            std::cerr << "ERROR: A file with the same name as the desired "
                      << "directory already exists: " << dir_path << "\n";
            return EXECUTION_FAILED;
        }
    }
    // We are calling the native UNIX-API to improve performance
    if (mkdir(dir_path.c_str(), mode) != 0) {
        std::cerr << "ERROR: Failed to create directory: " << dir_path << "\n";
        return EXECUTION_FAILED;
    }
    return EXECUTION_SUCCESS;
}

// Archive a series of log files
bool archive_logs(const std::vector<std::string>& path_to_logs,
                   const std::string &custom_backup_dir) {
    for (const auto& path_to_log : path_to_logs) {
        if (!archive_log(path_to_log, custom_backup_dir)) {
            // Found errors when archiving 1 file!
            std::cerr << "ERROR: Failed to archive log: " << path_to_log << "\n";
            std::cerr << "Execution will be terminated to prevent potential lost.\n";
            return EXECUTION_FAILED;
        }
    }
    return EXECUTION_SUCCESS;
}

// Main function, archive 1 log file
bool archive_log(const std::string &path_to_log,
                  const std::string &custom_backup_dir) {
    struct stat file_stat{};
    // Get information of the log file
    if (stat(path_to_log.c_str(), &file_stat) != 0) {
        std::cerr << "ERROR: Failed to retrieve information regarding " 
                  << path_to_log << ".\n";
        return EXECUTION_FAILED;
    }

    std::cout << "Log file " << path_to_log << " detected.\n";
    // Check if the file IS a file (not a path)
    if (!S_ISREG(file_stat.st_mode)) {
        std::cerr << "ERROR: " << path_to_log << " is not a regular file.\n";
        return EXECUTION_FAILED;
    }

    // Calculate file size to human-readable ones
    unsigned long file_size = file_stat.st_size;
    std::string readable_size = format_file_size(file_size);
    std::cout << "Size: " << readable_size << "\n";
    if (file_size == 0) {
        std::cout << "The log file appears to be empty. Backup is not necessary.\n";
        return EXECUTION_FAILED;
    }

    // Create backup directory
    std::cout << "Attempting to create/verify backup directory...\n";
    if (!create_directory(custom_backup_dir)) {
        std::cerr << "ERROR: Failed to create or verify backup directory.\n";
        return EXECUTION_FAILED;
    }
    // Create backup archive file's name
    size_t last_delimiter = path_to_log.find_last_of('/');
    std::string file_name = (last_delimiter != std::string::npos) ?
                            path_to_log.substr(last_delimiter + 1) : path_to_log;
    std::string file_dir = (last_delimiter != std::string::npos) ?
                            path_to_log.substr(0, last_delimiter) : ".";
    std::string timestamp = get_current_timestamp();
    std::string backup_filename = custom_backup_dir + "/"
                                  + file_name + "_" + timestamp + ".tar.xz";
    std::cout << "Attempting to create archives...\n";
    if (!create_tar_archive(file_dir, file_name, backup_filename)) {
        std::cerr << "ERROR: Failed to create archive.\n";
        if (file_or_dir_exists(backup_filename))
            remove(backup_filename.c_str());
        return EXECUTION_FAILED;
    }

    // Verify archive creation
    std::cout << "Verifying if archive is created...\n";
    if (!file_or_dir_exists(backup_filename)) {
        std::cerr << "ERROR: Archive file verification failed.\n";
        return EXECUTION_FAILED;
    }
    // Success, do some cleanups
    std::cout << "Archive created successfully at " << backup_filename << "\n";
    std::cout << "Cleaning original log file...\n";
    if (log_cleanup(path_to_log)) {
        std::cerr << "ERROR: Failed to clean log file, however archive was created successfully.\n";
        return EXECUTION_FAILED;
    }
    std::cout << "Job completed successfully.\n";
    return EXECUTION_SUCCESS;
}
