# 🖥️ Advanced System Management Tool - 7SCW

**Version:** 9.1.1 | **Created by:** LCKY | **GitHub:** https://github.com/lackyhy/7SCW


A powerful Windows console application that provides comprehensive system management capabilities, advanced file operations, startup management, and a custom bash-like terminal with encryption, hash generation, and network testing features.

## ✨ Features

### 🔧 Core System Tools
- **File Manager** - Advanced file and directory management with search capabilities and disk management
- **Startup Management** - Comprehensive startup location monitoring and management
- **User Management** - System user listing and management
- **TEMP Files Cleanup** - Intelligent cleanup of temporary files across multiple locations
- **System Information** - Detailed system information display
- **Command Line Access** - Direct access to CMD and PowerShell

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

### 🔒 Security & Safe Mode
- **Administrator Privilege Detection** - Automatic admin rights verification
- **Safe Mode Operation** - Limited functionality mode without admin rights
- **Safe Mode Features:**
  - Works in current terminal (no new window creation)
  - Full menu access with limited functionality
  - Status display in window title

## 🚀 Installation

### Prerequisites
- Windows 10/11 (64-bit/32-bit)
- C++ compiler (GCC, MinGW, or Visual Studio)
- Administrator privileges (for full functionality)

### Build Instructions

1. **Clone the repository:**
   ```bash
   git clone https://github.com/lackyhy/7SCW.git
   cd 7SCW
   ```

2. **Compile with CMake:**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Or compile directly with GCC:**
   ```bash
   g++ main.cpp cpp_file/terminal/terminal_commands.cpp cpp_file/startup/SHOW_ALL_STARTUP.cpp cpp_file/file_manager/file_manager.cpp cpp_file/startup/restoreStartupSettings.cpp cpp_file/terminal/terminal.cpp cpp_file/startup/startup.cpp cpp_file/terminal/speed_test.cpp -o 7SCW.exe
   ```

## 📖 Usage

### Command Line Arguments
- **`-safemod`** - Run in safe mode (works in current terminal without admin rights)
- **`-clear_tempfile`** - Clear temporary files and exit
- **`-clear_autorun`** - Restore startup settings and exit

### Main Menu Navigation
- **↑/↓ Arrows** - Navigate menu items
- **Enter** - Select option
- **Right Arrow** - Open custom terminal
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
├── cpp_file/
│   ├── terminal/
│   │   ├── terminal.cpp              # Terminal implementation
│   │   ├── terminal_commands.cpp     # Terminal commands
│   │   └── speed_test.cpp            # Network speed testing
│   ├── startup/
│   │   ├── startup.cpp               # Startup management
│   │   ├── SHOW_ALL_STARTUP.cpp      # Comprehensive startup viewer
│   │   └── restoreStartupSettings.cpp # Startup restoration
│   └── file_manager/
│       └── file_manager.cpp          # File management system
├── h_file/
│   ├── terminal/                     # Terminal headers
│   ├── startup/                      # Startup headers
│   ├── file_manager/                 # File manager headers
│   └── main.h                        # Main header
└── README.md                         # This file
```

## 🔧 Technical Details

### Architecture
- **Modular Design** - Separated components for terminal, startup, and file management
- **Windows API Integration** - Native Windows system calls and registry access
- **Console Interface** - Rich text-based user interface with color support
- **Error Handling** - Comprehensive error checking and user feedback

### Security Features
- **XOR Encryption** - Simple but effective file encryption
- **Random Hash Generation** - Cryptographically secure random number generation
- **Access Control** - Administrator privilege verification
- **Startup Security** - Malware detection in startup locations

### Performance
- **Efficient File Operations** - Optimized file I/O with buffering
- **Memory Management** - Proper resource cleanup and memory handling
- **Fast Search** - Efficient file search algorithms
- **Network Testing** - High-performance network diagnostics

## 🐛 Known Issues

- Requires Administrator privileges for full functionality
- Some features may not work on older Windows versions
- File encryption uses basic XOR method (not suitable for high-security applications)
- Safe mode provides limited functionality

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

---

**⭐ Star this repository if you find it useful!**

**🔒 Security Note:** This tool requires administrator privileges for full functionality and should be used responsibly. Always backup important files before encryption/decryption operations or startup modifications.

**🛡️ Safe Mode:** Use `-safemod` flag to run with limited functionality without administrator privileges.
