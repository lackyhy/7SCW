# ⚡ Quick Start Guide

Get your Advanced System Management Tool running in minutes!

## 🚀 Quick Setup

### 1. Download & Compile
```bash
# Clone or download the project
# Navigate to project folder
cd advanced-system-tool

# Compile (Windows)
g++ 7cpp.cpp terminal_commands.cpp -o system_tool.exe

# Or use MinGW
mingw32-g++ 7cpp.cpp terminal_commands.cpp -o system_tool.exe
```

### 2. Run as Administrator
```bash
# Right-click system_tool.exe → "Run as administrator"
# Or from command line:
system_tool.exe
```

### 3. Use the Tool
- **Arrow Keys** - Navigate menu
- **Enter** - Select option
- **Right Arrow** - Open custom terminal
- **Ctrl+C** - Exit

## 🎯 Key Features to Try

### 🔐 File Encryption
```bash
# In terminal (Right Arrow → help)
encrypt -pass mypassword test.txt
deencrypt test.txt.encrypted
```

### 🔑 Hash Generation
```bash
# Generate 5 hashes of length 20
create_hash -l 20 -q 5

# Use standard settings
create_hash -s -q 10
```

### 📁 File Management
- Select "File Manager" from main menu
- Navigate folders with arrow keys
- Right arrow for file operations
- Search files with 'S' key

## 🆘 Need Help?

- Type `help` in terminal
- Check `README.md` for full documentation
- Use `create_hash -h` for hash command help

## ⚠️ Important Notes

- **Always run as Administrator**
- **Backup files before encryption**
- **Test on non-critical files first**

---

**🎉 You're ready to go!** Explore the features and enjoy your new system management tool!
