// overlayHook.cpp
// Sends a small overdrive state message to CHEAPOverlay over UDP on
// 127.0.0.1. UDP is fire and forget.
//we send one tiny text packet and move on, with no connection
// no reply, and no error if nobody is listening. This means it
// can never stall or crash the game.
//
// All the networking headers are kept in THIS .cpp file on purpose: on Windows,
// winsock2.h must be included before windows.h (which raylib pulls in), so
// isolating it here avoids header-order conflicts everywhere else.

#ifdef _WIN32
  #include <winsock2.h>          // Windows sockets (must come before windows.h)
  #include <ws2tcpip.h>          // inet_pton
  #pragma comment(lib, "ws2_32.lib")  // auto-links the socket library (no build change)
#else
  #include <sys/socket.h>        // macOS / Linux sockets
  #include <arpa/inet.h>
  #include <unistd.h>            // close()
#endif
#include <cstdio>               // snprintf
#include "overlayHook.h"

void OverlayEmit(int player, bool sp, double fill) {
    // On Windows, the socket system needs a one-time startup call before first use.
    // The static flag makes sure that only happens once, not on every send.
    static bool started = false;
#ifdef _WIN32
    if (!started) { WSADATA w; WSAStartup(MAKEWORD(2, 2), &w); started = true; }
#endif

    // Create a UDP socket (SOCK_DGRAM = UDP). If it fails, just give up quietly.
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;

    // Address the message to "this computer, port 2830".
    // 127.0.0.1 = localhost; 2830 = the port the overlay listens on
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(2830);                  // htons = correct byte order for the port
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    // Build the message as a small JSON string, e.g. {"player":0,"sp":true,"fill":0.420}
    char msg[128];

    int len = snprintf(msg, sizeof(msg),
        R"({"player":%d,"sp":%s,"fill":%.3f})",
        player, sp ? "true" : "false", fill);

    // Send it. No reply is expected — this returns almost instantly.
    sendto(sock, msg, len, 0, (sockaddr*)&addr, sizeof(addr));

    // Close the socket (the function name differs per OS).
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}