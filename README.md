# logbk

> ⚠️ **Important Note**
> 
> The project is currently in **alpha stage** and has not been fully tested yet.
> Therefore, bugs and unexpected behavior are possible.
> If you encounter any, please open an issue.

## What is logbk?

**logbk** is a small command-line tool for **archiving Linux log files**.

It is written in **C++** and designed with the **KISS principle (Keep It Simple, Stupid)** in mind.
The program follows a **process-oriented approach**, focusing on simplicity and minimal dependencies.

Its main purpose is to **archive log files, compress them, and clean the original logs automatically**.

Documentation in other languages:

[简体中文](./docs/README_zh-cn.md) | [繁體中文](./docs/README_zh-tw.md) | [日本語](./docs/README_ja.md)

---

## Requirements

To build and run `logbk`, you will need:

* **Operating System:** Linux (kernel 6.x recommended)
* **Compiler:**

    * GCC/G++ 6.1 or later
    * Clang 3.4 or later
* **C++ Standard:** C++ 17
* **Build System:** CMake 3.10 or later

---

## Build and Run

Clone the repository and build the program using CMake:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Then run it with root privileges:

```bash
sudo ./logbk
```

---

## Using Release Files

Official releases may provide one or more of the following formats:

* **Single precompiled binary (exeucutable) file**
* **`.deb` packages** for Debian-based distributions
* **`.rpm` packages** for RPM-based distributions

The provided **`.rpm` packages are expected to be compatible with** distributions such as **openSUSE Leap** and other RPM-based server environments.

A **`.tar.zst` package is intentionally not available by default**, as **Arch Linux is generally not recommended for server deployments**. Users of Arch-based systems can still build the program manually if needed.

---


## Usage

⚠️ **Root privileges are required** to run this program because it may access system log files.

Basic syntax:

```bash
logbk <log_file> [backup_directory]
logbk help
```

### Arguments

* **log_file**
  Path to the log file you want to archive and clean.

* **backup_directory** *(optional)*
  Directory where backups will be stored.
  Default location:

```
/root/syslogbackup
```

---

## Examples

Backup the system log using the default backup directory:

```bash
logbk /var/log/syslog
```

Backup `auth.log` to a custom directory:

```bash
logbk /var/log/auth.log -d /data/backup/log
```

Backup according to a list of log files:
```bash
logbk -f log_list.txt -d /data/backup/log
```

Show help information:

```bash
logbk help
```

---

## Notes

* Backups are stored as **`.tar.xz` archives**
* The original log file is **truncated (cleared) after the backup**
* Backup filenames include a **timestamp** in the format:

```
YYYYMMDD-HHMMSS
```

Example:

```
syslog-20260313-154210.tar.xz
```

---

## Platform Notes

Although **logbk** is designed primarily for **Linux systems**, some administrators may attempt to build or run it on other platforms such as **macOS**, **FreeBSD**, or other Unix-like operating systems.

In many cases the program may compile successfully using a compatible C++17 toolchain, but **native functionality is only intended and tested for Linux**. Behavior on non-Linux platforms is **not guaranteed**.

Particularly on **Windows**, the program may not work natively and would typically require compatibility layers or environments such as WSL. Such setups are **not officially supported**.

---

## Disclaimer

This software is provided **as-is**, without any warranty.

The developer **is not responsible for any data loss, service interruption, or system damage** that may occur from:

* running the program on **unsupported platforms**
* **improper usage**
* modifying the program or build process
* using **unofficial builds**

System administrators are expected to **review the source code and test the software in a safe environment** before deploying it on production systems.

---
