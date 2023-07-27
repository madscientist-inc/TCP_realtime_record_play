// server.cpp
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
void RecordAndSendData(SOCKET clientSocket);

int main() {
    // ポート番号
    int port = 12345;

    // Winsockの初期化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // ソケットの作成
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // ローカルアドレスの設定
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // ソケットにローカルアドレスをバインド
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // クライアントの接続待ち
    listen(listenSocket, 1);
    std::cout << "Waiting for client connection..." << std::endl;

    // クライアントの接続受け入れ
    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected." << std::endl;

    // 音声データの録音と送信を開始
    RecordAndSendData(clientSocket);

    // ソケットのクローズ
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}

void RecordAndSendData(SOCKET clientSocket) {
    // 音声データの録音と送信
    const int bufferSize = 1024;
    char buffer[bufferSize];

    HWAVEIN hWaveIn;
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Failed to open waveform audio input device." << std::endl;
        return;
    }

    WAVEHDR WaveInHdr;
    WaveInHdr.lpData = buffer;
    WaveInHdr.dwBufferLength = bufferSize;
    WaveInHdr.dwBytesRecorded = 0;
    WaveInHdr.dwUser = 0;
    WaveInHdr.dwFlags = 0;
    WaveInHdr.dwLoops = 0;
    waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

    waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
    waveInStart(hWaveIn);

    while (true) {
        // 音声データをクライアントに送信
        send(clientSocket, buffer, bufferSize, 0);
        waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
        waveInStart(hWaveIn);
        Sleep(1);
    }

    waveInClose(hWaveIn);
}