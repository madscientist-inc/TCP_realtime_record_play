// client.cpp
#include <iostream>
#include <cstring>
#include <WinSock2.h>
#include <windows.h>
#include <Ws2tcpip.h> // inet_ntopを利用するために必要な追加のヘッダー
#include <iostream>
#include <cstring>
#include <mmsystem.h>               //Sound
#pragma comment(lib, "winmm.lib")   //Sound
#pragma comment(lib, "ws2_32.lib")
void ReceiveAndPlayData(SOCKET serverSocket);

int main() {
    // ポート番号とIPアドレス
    int port = 12345;
    const char* ipAddress = "127.0.0.1"; // サーバーとクライアントが同じマシン上で動作する場合

    // Winsockの初期化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // ソケットの作成
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // 接続先の設定
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress, &serverAddr.sin_addr); // inet_addrをinet_ptonに変更

    // サーバーに接続
    if (connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // 受信した音声データの再生
    ReceiveAndPlayData(serverSocket);

    // ソケットのクローズ
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

void ReceiveAndPlayData(SOCKET serverSocket) {
    // 音声データの受信と再生
    const int bufferSize = 1024;
    char buffer[bufferSize];

    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Failed to open waveform audio output device." << std::endl;
        return;
    }

    while (true) {
        // サーバーから音声データを受信
        int bytesRead = recv(serverSocket, buffer, bufferSize, 0);
        if (bytesRead <= 0) {
            break; // サーバーが切断した場合は受信ループを終了
        }

        // 音声データを再生
        WAVEHDR WaveOutHdr;
        WaveOutHdr.lpData = buffer;
        WaveOutHdr.dwBufferLength = bytesRead;
        WaveOutHdr.dwBytesRecorded = bytesRead;
        WaveOutHdr.dwUser = 0;
        WaveOutHdr.dwFlags = 0;
        WaveOutHdr.dwLoops = 0;
        waveOutPrepareHeader(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));
        Sleep(1);
    }

    waveOutClose(hWaveOut);
}