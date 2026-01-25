#pragma once
// --- 平台相关的头文件 ---
#ifdef _WIN32
    #include <windows.h>
    #include <iostream>
#else
    #include <fcntl.h>
    #include <termios.h>
    #include <unistd.h>
    #include <dirent.h>
#endif

class SerialPort {
#ifdef _WIN32
    HANDLE hSerial = INVALID_HANDLE_VALUE;
#else
    int fd = -1;
#endif
public:
    ~SerialPort() { Close(); }

    void Close() {
#ifdef _WIN32
        if (hSerial != INVALID_HANDLE_VALUE) {
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;
        }
#else
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
#endif
    }

    bool Connect(const std::string& port) {
        Close(); // 先清理旧连接
#ifdef _WIN32
        // Windows 实现
        // 加上 \\.\ 前缀以支持 COM10 以上的端口
        const std::string portName = R"(\\.\)" + port;

        hSerial = CreateFileA(portName.c_str(),
                              GENERIC_READ | GENERIC_WRITE,
                              0,    // 独占访问
                              nullptr, // 无安全属性
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              nullptr);

        if (hSerial == INVALID_HANDLE_VALUE) return false;

        // 设置参数 (Baudrate, ByteSize, StopBits, Parity)
        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(hSerial, &dcbSerialParams)) {
            Close(); return false;
        }

        dcbSerialParams.BaudRate = CBR_115200; // 对应 B115200
        dcbSerialParams.ByteSize = 8;          // 对应 CS8
        dcbSerialParams.StopBits = ONESTOPBIT; // 对应 ~CSTOPB
        dcbSerialParams.Parity   = NOPARITY;   // 对应 ~PARENB

        if (!SetCommState(hSerial, &dcbSerialParams)) {
            Close(); return false;
        }

        // 设置超时 (对应 POSIX 的 VTIME/VMIN)
        // 下面的设置模拟了非阻塞读但在写入时有超时
        COMMTIMEOUTS timeouts = {0};
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 100; // 100ms (对应你的 VTIME=1)
        timeouts.WriteTotalTimeoutMultiplier = 0;

        if (!SetCommTimeouts(hSerial, &timeouts)) {
            Close(); return false;
        }

        return true;
#else
        if (fd != -1) close(fd);
        fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd == -1) return false;
        
        fcntl(fd, F_SETFL, 0); // 恢复阻塞模式 (带超时)

        termios options{};
        tcgetattr(fd, &options);
        cfsetispeed(&options, B115200); // 必须匹配 ESP32
        cfsetospeed(&options, B115200);
        
        options.c_cflag |= (CLOCAL | CREAD);
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw mode
        options.c_oflag &= ~OPOST;
        
        // 关键：设置写入超时，防止拔线卡死
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 1; // 0.1s 超时

        tcsetattr(fd, TCSANOW, &options);
        return true;
#endif
    }

    bool Write(const void* data, const size_t len) {
#ifdef _WIN32
        if (hSerial == INVALID_HANDLE_VALUE) return false;
        DWORD bytesWritten;
        // Windows WriteFile
        if (!WriteFile(hSerial, data, (DWORD)len, &bytesWritten, nullptr)) {
            Close(); // 写入失败，自动断开
            return false;
        }
        // 检查是否完整写入
        if (bytesWritten != len) {
            Close(); return false;
        }
        return true;
#else
        if (fd == -1) return false;
        const ssize_t n = write(fd, data, len);
        if (n < 0 || n != static_cast<ssize_t>(len)) {
            close(fd); fd = -1; // 自动断开
            return false;
        }
        return true;
#endif
    }

    static std::string AutoDetectPort() {
#ifdef _WIN32
        // Windows: 通过注册表查找串口
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DEVICEMAP\\SERIALCOMM", 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            return "";
        }

        char valueName[256];
        char data[256];
        DWORD valueNameSize, dataSize, type;
        DWORD index = 0;
        std::string foundPort;

        // 遍历所有串口
        while (true) {
            valueNameSize = sizeof(valueName);
            dataSize = sizeof(data);
            const LONG result = RegEnumValueA(hKey, index, valueName, &valueNameSize, NULL, &type, (LPBYTE)data, &dataSize);

            if (result != ERROR_SUCCESS) break;

            if (type == REG_SZ) {
                const std::string portName(data);
                // 简单的过滤逻辑：通常 ESP32/USB 串口在 Windows 上就是 COMx
                // 如果你有特定的 USB VID/PID 需求，Windows 上通常需要更复杂的 SetupAPI
                // 这里默认返回找到的最后一个 COM 口，或者你可以添加逻辑优先匹配 "COM3" 等
                foundPort = portName;
            }
            index++;
        }
        RegCloseKey(hKey);
        return foundPort; // 返回找到的一个端口，如 "COM3"

#else
        DIR* dir = opendir("/dev");
        dirent* ent;
        if (dir) {
            while ((ent = readdir(dir)) != nullptr) {
                std::string name = ent->d_name;
                if (name.find("tty.usbmodem") != std::string::npos ||
                    name.find("tty.usbserial") != std::string::npos) {
                    closedir(dir);
                    return "/dev/" + name;
                    }
            }
            closedir(dir);
        }
        return "";
#endif
    }
    bool IsConnected() const {
#ifdef _WIN32
        return hSerial != INVALID_HANDLE_VALUE;
#else
        return fd != -1;
#endif
    }
};