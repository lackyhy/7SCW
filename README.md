# 🖥️ 7 System Control Windows - 7SCW

**Version:** 13.4 | **Created by:** LCKY and Akane | **GitHub:** https://github.com/lackyhy/7SCW

A powerful Windows console application that provides comprehensive system management capabilities, advanced file operations, startup management, security monitoring, and a custom bash-like terminal with encryption, hash generation, and network testing features.

## ✨ Features

### 🔧 Core System Tools
- **File Manager** - Advanced file and directory management with search capabilities and disk management
- **Startup Management** - Comprehensive startup location monitoring and management
- **User Management** - System user listing and management
- **TEMP Files Cleanup** - Intelligent cleanup of temporary files across multiple locations
- **System Information** - Detailed system information display
- **Command Line Access** - Direct access to CMD and PowerShell
- **Cookie Cleaner** - Multi-browser cookie cleanup (Chrome, Firefox, Edge, Opera, Yandex)
- **Script Activator** - Execute custom scripts and batch files from `__script_` folder

### 🚀 Startup Management System
Advanced startup monitoring and management tools:

#### 📂 Startup Folders
- **User Startup Folder** - View and manage current user startup items
- **All Users Startup Folder** - Manage system-wide startup items
- **Interactive Commands:**
  - `open_user_startup` - Open current user startup folder
  - `open_all_startup` - Open all users startup folder

#### 🗂️ Registry Management
- **HKCU Run/RunOnce** - Current user startup registry entries
- **HKLM Run/RunOnce** - System-wide startup registry entries
- **Interactive Commands:**
  - `addhkcu_run <name> <value>` - Add HKCU startup entry
  - `addhklm_run <name> <value>` - Add HKLM startup entry
  - `deletehkcu_run <name>` - Delete HKCU startup entry
  - `deletehklm_run <name>` - Delete HKLM startup entry

#### ⏰ Task Scheduler
- View and manage scheduled startup tasks
- Comprehensive task listing and analysis

#### 🔒 Shell/Userinit Security
- **Shell Value Monitoring** - Check for malicious shell modifications
- **Userinit Value Monitoring** - Monitor userinit.exe integrity
- **Security Commands:**
  - `restore_shell` - Restore Shell to explorer.exe
  - `restore_userinit` - Restore Userinit to userinit.exe
  - `check_malware` - Check for suspicious values

#### 🔄 Restore Functions
- **Complete Startup Restoration** - Restore all startup settings to original state
- **Selective Restoration** - Restore specific startup components

### 🖥️ Custom Terminal (Bash-like)
Accessible via **Right Arrow** key from main menu, featuring:

#### 🔑 Hash Generation
- **`create_hash -l <length> -q <quantity> -f <file>`** - Generate random hashes
- **`create_hash -s`** - Use standard settings (length 27)
- **`create_hash -h`** - Show detailed help

#### 🌐 Network Management & Testing
Comprehensive network tools accessible via `network` command:

- **Connection Testing:**
  - Basic network connectivity test
  - Extended ping test (25 popular sites)
  - Custom ping to specific URLs
  
- **Network Information:**
  - Show saved networks
  - Show available networks
  - Network configuration details

- **Speed Testing:**
  - Download speed simulation
  - Upload speed simulation  
  - Ping latency testing with statistics (min/max/avg)

#### 🔐 File Encryption
- **`encrypt -pass <password> <file>`** - Encrypt files with password
- **`deencrypt <file>`** - Decrypt files (prompts for password)

#### 🐚 Standard Commands
- **File Operations:** `ls`, `dir`, `cd`, `pwd`, `mkdir`, `rm`, `rmdir`, `cp`, `mv`, `cat`
- **System Info:** `whoami`, `hostname`, `systeminfo`, `processes`, `services`
- **Network:** `netstat`, `ipconfig`, `network` (opens network menu)
- **Utilities:** `clear`, `date`, `echo`, `help`

### 📁 Advanced File Management
- **Directory Navigation** - Intuitive folder browsing with history
- **File Operations** - Copy, move, delete, rename, create
- **Search Engine** - Powerful file search with multiple filters
- **Disk Management** - Drive information and cleanup tools
- **Access Control** - File permission checking and management

### 🔒 Advanced Security & Monitoring
Accessible via **Left Arrow** key from main menu, featuring:

#### 🔍 File Integrity & Verification
- **File Hash Verification** - Check system file integrity with MD5/SHA checksums
- **System Integrity Check** - Comprehensive system health verification
- **Export Security Report** - Generate detailed security reports

#### 📊 Event Logs & Monitoring
- **Windows Event Logs** - Browse Application, Security, and System logs
- **Custom Log Search** - Advanced log filtering and search capabilities
- **Security Statistics** - System security overview and metrics
- **Real-time Monitoring** - Live system status monitoring

#### 🛡️ Security Features
- **Administrator Privilege Detection** - Automatic admin rights verification
- **Safe Mode Operation** - Limited functionality mode without admin rights
- **Safe Mode Features:**
  - Works in current terminal (no new window creation)
  - Full menu access with limited functionality
  - Status display in window title

### 📝 Logging System
- **Comprehensive Logging** - Enable with `--logs` flag for detailed operation tracking
- **Multiple Log Levels** - INFO, WARNING, ERROR, SUCCESS with color coding
- **Hidden Log Files** - Automatic log file hiding for security
- **Session Tracking** - Complete session logging with timestamps

## 🚀 Installation

### Prerequisites
- Windows 10/11 (64-bit/32-bit)
- C++ compiler (GCC, MinGW, or Visual Studio)
- Administrator privileges (for full functionality)

### Build Instructions

#### Method 1: CMake (Recommended)
```bash
# Clone the repository
git clone https://github.com/lackyhy/7SCW.git
cd 7SCW

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# The executable will be created as 7SCW.exe
```

#### Method 2: Direct GCC Compilation
```bash
# Navigate to project directory
cd 7SCW

# Compile all source files
g++ -std=c++20 -static-libgcc -static-libstdc++ \
    main.cpp \
    cpp_file/argv.cpp \
    cpp_file/terminal/terminal_commands.cpp \
    cpp_file/terminal/terminal.cpp \
    cpp_file/terminal/speed_test.cpp \
    cpp_file/startup/SHOW_ALL_STARTUP.cpp \
    cpp_file/startup/startup.cpp \
    cpp_file/startup/restoreStartupSettings.cpp \
    cpp_file/file_manager/file_manager.cpp \
    cpp_file/security/file_hash_verifier.cpp \
    cpp_file/security/log_viewer.cpp \
    cpp_file/security/advanced_security_menu.cpp \
    cpp_file/file/clear_temp_file.cpp \
    cpp_file/file/clear_cookie.cpp \
    cpp_file/activator/menu.cpp \
    cpp_file/activator/load_script.cpp \
    cpp_file/system_info/system_info.cpp \
    cpp_file/logs/logs.cpp \
    -luser32 -liphlpapi -lws2_32 -ladvapi32 -lwininet \
    -o 7SCW.exe
```

#### Method 3: Visual Studio (Windows)
```bash
# Open Developer Command Prompt
# Navigate to project directory
cd 7SCW

# Compile with MSVC
cl /std:c++20 main.cpp cpp_file\argv.cpp cpp_file\terminal\terminal_commands.cpp cpp_file\terminal\terminal.cpp cpp_file\terminal\speed_test.cpp cpp_file\startup\SHOW_ALL_STARTUP.cpp cpp_file\startup\startup.cpp cpp_file\startup\restoreStartupSettings.cpp cpp_file\file_manager\file_manager.cpp cpp_file\security\file_hash_verifier.cpp cpp_file\security\log_viewer.cpp cpp_file\security\advanced_security_menu.cpp cpp_file\file\clear_temp_file.cpp cpp_file\file\clear_cookie.cpp cpp_file\activator\menu.cpp cpp_file\activator\load_script.cpp cpp_file\system_info\system_info.cpp cpp_file\logs\logs.cpp /Fe:7SCW.exe user32.lib iphlpapi.lib ws2_32.lib advapi32.lib wininet.lib
```

#### Method 4: MinGW (Alternative)
```bash
# Using MinGW compiler
mingw32-g++ -std=c++20 main.cpp [all cpp files] -o 7SCW.exe -luser32 -liphlpapi -lws2_32 -ladvapi32 -lwininet
```

## 📖 Usage

### Command Line Arguments
- **`-safemod`** - Run in safe mode (works in current terminal without admin rights)
- **`-clear_tempfile`** - Clear temporary files and exit
- **`-clear_autorun`** - Restore startup settings and exit
- **`--logs`** - Enable comprehensive logging to logs.txt file

### Main Menu Navigation
- **↑/↓ Arrows** - Navigate menu items
- **Enter** - Select option
- **Right Arrow** - Open custom terminal
- **Left Arrow** - Open Advanced Security Menu
- **Ctrl+C** - Exit application
- **'h'** - Show help menu
- **'q'** - Quit current operation

### Terminal Commands

#### 🔑 Hash Generation
```bash
# Generate 5 hashes of length 18
create_hash -l 18 -q 5

# Use standard settings (length 27) with 10 hashes
create_hash -s -q 10

# Save hashes to file
create_hash -l 20 -q 3 -f hashes.txt

# Show help
create_hash -h
```

#### 🌐 Network Management
```bash
# Open network management menu
network

# Test network connection
# (Available in network menu)

# Run speed test
# (Available in network menu)
```

#### 🔐 File Encryption
```bash
# Encrypt file with password
encrypt -pass mypassword secret.txt

# Decrypt file (will prompt for password)
deencrypt secret.txt.enc
```

#### 🍪 Cookie Cleanup
```bash
# Access cookie cleanup from main menu
# Supports: Chrome, Firefox, Edge, Opera, Yandex
# Automatically detects browser profiles and cleans cookies
```

#### 📝 Logging & Monitoring
```bash
# Enable logging when starting the program
7SCW.exe --logs

# View logs in real-time
# Logs are automatically saved to logs.txt (hidden file)
```

#### 🛡️ Security Features
```bash
# Access Advanced Security Menu (Left Arrow from main menu)
# Features:
# - File Hash Verification
# - System Integrity Check
# - Event Log Browsing
# - Security Statistics
# - Export Security Reports
```

#### 🐚 File Operations
```bash
# List directory contents
ls
dir

# Change directory
cd C:\Users\Username\Documents

# Create directory
mkdir new_folder

# Copy file
cp source.txt destination.txt

# Move/rename file
mv old_name.txt new_name.txt
```

## 🏗️ Project Structure

```
7SCW/
├── main.cpp                          # Main application file
├── CMakeLists.txt                    # CMake build configuration
├── CMakePresets.json                 # CMake presets configuration
├── cpp_file/                         # Source code files
│   ├── activator/
│   │   ├── load_script.cpp           # Script loading functionality
│   │   └── menu.cpp                 # Activator menu system
│   ├── argv.cpp                     # Command line argument processing
│   ├── file/
│   │   ├── clear_cookie.cpp         # Browser cookie cleanup
│   │   └── clear_temp_file.cpp      # Temporary file cleanup
│   ├── file_manager/
│   │   └── file_manager.cpp         # File management system
│   ├── logs/
│   │   └── logs.cpp                 # Logging system implementation
│   ├── security/
│   │   ├── advanced_security_menu.cpp # Security menu interface
│   │   ├── file_hash_verifier.cpp   # File integrity verification
│   │   └── log_viewer.cpp           # Event log viewer
│   ├── startup/
│   │   ├── restoreStartupSettings.cpp # Startup restoration
│   │   ├── SHOW_ALL_STARTUP.cpp     # Comprehensive startup viewer
│   │   └── startup.cpp              # Startup management
│   ├── system_info/
│   │   └── system_info.cpp          # System information display
│   └── terminal/
│       ├── speed_test.cpp           # Network speed testing
│       ├── terminal_commands.cpp    # Terminal command implementation
│       └── terminal.cpp             # Terminal interface
├── h_file/                          # Header files
│   ├── activator/                   # Activator headers
│   ├── argv.h                       # Command line arguments header
│   ├── file/                        # File operation headers
│   ├── file_manager/                # File manager headers
│   ├── logs/                        # Logging headers
│   ├── main.h                       # Main application header
│   ├── security/                    # Security feature headers
│   ├── startup/                     # Startup management headers
│   ├── system_info/                 # System info headers
│   └── terminal/                    # Terminal headers
├── build/                           # CMake build directory
├── out/                             # Output directory
└── README.md                        # This documentation file
```

## 🔧 Technical Details

### Architecture
- **Modular Design** - Separated components for terminal, startup, file management, security, and logging
- **Windows API Integration** - Native Windows system calls and registry access
- **Console Interface** - Rich text-based user interface with color support and Unicode
- **Error Handling** - Comprehensive error checking and user feedback
- **Cross-Platform Compatibility** - Designed for Windows 10/11 with 32/64-bit support

### Security Features
- **XOR Encryption** - Simple but effective file encryption
- **Random Hash Generation** - Cryptographically secure random number generation
- **Access Control** - Administrator privilege verification and safe mode operation
- **Startup Security** - Malware detection in startup locations
- **File Integrity Verification** - MD5/SHA checksum validation
- **Event Log Monitoring** - Real-time system log analysis

### Performance
- **Efficient File Operations** - Optimized file I/O with buffering
- **Memory Management** - Proper resource cleanup and memory handling
- **Fast Search** - Efficient file search algorithms
- **Network Testing** - High-performance network diagnostics
- **Static Linking** - Self-contained executable with minimal dependencies

## 🐛 Known Issues

- Requires Administrator privileges for full functionality
- Some features may not work on older Windows versions
- File encryption uses basic XOR method (not suitable for high-security applications)
- Safe mode provides limited functionality
- Cookie cleanup requires browsers to be closed
- Script activator requires scripts to be placed in `__script_` folder
- Logging creates hidden files that may need manual cleanup

## 🔧 Troubleshooting

### Common Error: 0xc000007b

If you encounter the error **"The application was unable to start correctly (0xc000007b)"**:

#### Quick Fixes:
1. **Install Visual C++ Redistributable:**
   - Download [Microsoft Visual C++ Redistributable 2015-2022](https://aka.ms/vs/17/release/vc_redist.x64.exe)
   - Install and restart your computer

2. **Recompile with correct architecture:**
   ```bash
   # For 64-bit Windows:
   g++ -m64 main.cpp [all cpp files] -o 7SCW_64.exe
   
   # For 32-bit Windows:
   g++ -m32 main.cpp [all cpp files] -o 7SCW_32.exe
   ```

3. **Use static linking for maximum compatibility:**
   ```bash
   g++ -static main.cpp [all cpp files] -o 7SCW_static.exe
   ```

#### Check System Architecture:
```bash
# In Command Prompt:
systeminfo | findstr "System Type"
```

#### Alternative Compilers:
```bash
# Try MinGW:
mingw32-g++ main.cpp [all cpp files] -o 7SCW.exe

# Or Visual Studio Developer Command Prompt:
cl main.cpp [all cpp files] /Fe:7SCW.exe
```

### Safe Mode Usage
If you don't have administrator privileges:
```bash
7SCW.exe -safemod
```

This will run the program in safe mode with limited functionality but without requiring admin rights.

### Logging Issues
If logging is not working:
```bash
# Check if logs.txt exists (it's hidden by default)
dir /a logs.txt

# Enable logging explicitly
7SCW.exe --logs

# View logs
type logs.txt
```

### Cookie Cleanup Issues
If cookie cleanup fails:
1. Close all browser instances completely
2. Run the program as administrator
3. Check if browser profiles exist in expected locations
4. Try cleaning cookies for one browser at a time

### Script Activator Issues
If scripts don't execute:
1. Ensure scripts are in `__script_` folder
2. Check script file extensions (.cmd, .bat, .ps1)
3. Verify PowerShell execution policy if using .ps1 files
4. Run as administrator for system-level scripts

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **LCKY** - Original developer
- **Windows API** - For system integration capabilities
- **C++ Community** - For language and best practices

## 📞 Support

If you encounter any issues or have questions:
- Create an issue on GitHub
- Check the help documentation in the application (press 'h' in main menu)
- Review the terminal help commands (type 'help' in terminal)
- Enable logging with `--logs` flag for detailed error tracking
- Use safe mode with `-safemod` flag if admin rights are not available
- Check the Advanced Security Menu for system diagnostics

---

**⭐ Star this repository if you find it useful!**

**🔒 Security Note:** This tool requires administrator privileges for full functionality and should be used responsibly. Always backup important files before encryption/decryption operations or startup modifications. The logging system creates hidden files that may contain sensitive information.

**🛡️ Safe Mode:** Use `-safemod` flag to run with limited functionality without administrator privileges.

**📝 Logging:** Use `--logs` flag to enable comprehensive logging for troubleshooting and monitoring.

**🍪 Privacy:** Cookie cleanup feature removes browser tracking data - use responsibly and ensure browsers are closed before cleanup.
