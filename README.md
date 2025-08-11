# 🖥️ Advanced System Management Tool

**Version:** 8.1.2 | **Created by:** LCKY

A powerful Windows console application that provides comprehensive system management capabilities, file operations, and a custom bash-like terminal with encryption and hash generation features.

## ✨ Features

### 🔧 Core System Tools
- **File Manager** - Advanced file and directory management with search capabilities
- **User Management** - System user listing and management
- **TEMP Files Cleanup** - Intelligent cleanup of temporary files
- **System Information** - Detailed system information display
- **Command Line Access** - Direct access to CMD and PowerShell

### 🖥️ Custom Terminal (Bash-like)
Accessible via **Right Arrow** key from main menu, featuring:

#### 🔐 Encryption & Security
- **`encrypt -pass <password> <file>`** - Encrypt files with XOR encryption
- **`deencrypt <file>`** - Decrypt files (prompts for password)
- **`deencrypt -pass <password> <file>`** - Decrypt with provided password

#### 🔑 Hash Generation
- **`create_hash -l <length> -q <quantity> -f <file>`** - Generate random hashes
- **`create_hash -s`** - Use standard settings (length 27)
- **`create_hash -h`** - Show detailed help

#### 🐚 Standard Commands
- **File Operations:** `ls`, `cd`, `pwd`, `mkdir`, `rm`, `rmdir`, `cp`, `mv`, `cat`
- **System Info:** `whoami`, `hostname`, `systeminfo`, `processes`, `services`
- **Network:** `netstat`, `ipconfig`
- **Utilities:** `clear`, `date`, `echo`, `help`

### 📁 Advanced File Management
- **Directory Navigation** - Intuitive folder browsing with history
- **File Operations** - Copy, move, delete, rename, create
- **Search Engine** - Powerful file search with multiple filters
- **Disk Management** - Drive information and cleanup tools
- **Access Control** - File permission checking and management

## 🚀 Installation

### Prerequisites
- Windows 10/11 (64-bit)
- Administrator privileges
- C++ compiler (GCC, MinGW, or Visual Studio)

### Build Instructions

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/advanced-system-tool.git
   cd advanced-system-tool
   ```

2. **Compile the project:**
   ```bash
   g++ 7cpp.cpp terminal_commands.cpp -o 7SCW.exe
   ```

3. **Run as Administrator:**
   ```bash
   7SCW.exe
   ```

## 📖 Usage

### Main Menu Navigation
- **↑/↓ Arrows** - Navigate menu items
- **Enter** - Select option
- **Right Arrow** - Open custom terminal
- **Ctrl+C** - Exit application

### Terminal Commands

#### 🔐 File Encryption
```bash
# Encrypt a file
encrypt -pass mypassword document.txt

# Decrypt a file (will prompt for password)
deencrypt document.txt.encrypted

# Decrypt with password
deencrypt -pass mypassword document.txt.encrypted
```

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
advanced-system-tool/
├── 7cpp.cpp                 # Main application file
├── terminal_commands.h      # Terminal commands header
├── terminal_commands.cpp    # Terminal commands implementation
├── README.md               # This file
└── 7SCW.exe         # Compiled executable
```

## 🔧 Technical Details

### Architecture
- **Modular Design** - Separated terminal commands into dedicated module
- **Windows API Integration** - Native Windows system calls
- **Console Interface** - Rich text-based user interface
- **Error Handling** - Comprehensive error checking and user feedback

### Security Features
- **XOR Encryption** - Simple but effective file encryption
- **Random Hash Generation** - Cryptographically secure random number generation
- **Access Control** - Administrator privilege verification

### Performance
- **Efficient File Operations** - Optimized file I/O with buffering
- **Memory Management** - Proper resource cleanup and memory handling
- **Fast Search** - Efficient file search algorithms

## 🐛 Known Issues

- Requires Administrator privileges for full functionality
- Some features may not work on older Windows versions
- File encryption uses basic XOR method (not suitable for high-security applications)

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
   g++ -m64 7cpp.cpp terminal_commands.cpp -o 7SCW_64.exe
   
   # For 32-bit Windows:
   g++ -m32 7cpp.cpp terminal_commands.cpp -o 7SCW_32.exe
   ```

3. **Use static linking for maximum compatibility:**
   ```bash
   g++ -static 7cpp.cpp terminal_commands.cpp -o 7SCW_static.exe
   ```

#### Check System Architecture:
```bash
# In Command Prompt:
systeminfo | findstr "System Type"
```

#### Alternative Compilers:
```bash
# Try MinGW:
mingw32-g++ 7cpp.cpp terminal_commands.cpp -o 7SCW.exe

# Or Visual Studio Developer Command Prompt:
cl 7cpp.cpp terminal_commands.cpp /Fe:7SCW.exe
```

#### 32-bit Compiler Issues:

If you get **"sorry, unimplemented: 64-bit mode not compiled in"**:

1. **Use 32-bit compilation (default):**
   ```bash
   g++ 7cpp.cpp terminal_commands.cpp -o 7SCW.exe
   ```

2. **Install 64-bit MinGW-w64:**
   - Download [MSYS2](https://www.msys2.org/)
   - Install: `pacman -S mingw-w64-x86_64-gcc`

3. **Use static linking for compatibility:**
   ```bash
   g++ -static 7cpp.cpp terminal_commands.cpp -o 7SCW_static.exe
   ```

4. **Check your compiler version:**
   ```bash
   g++ --version
   systeminfo | findstr "System Type"
   ```

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
- Check the help documentation in the application
- Review the terminal help commands

---

**⭐ Star this repository if you find it useful!**

**🔒 Security Note:** This tool requires administrator privileges and should be used responsibly. Always backup important files before encryption/decryption operations.
