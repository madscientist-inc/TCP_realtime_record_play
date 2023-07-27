// server.cpp
#include <iostream>
#include <cstring>
#include <WinSock2.h>
#include <windows.h>
#include <Ws2tcpip.h> // inet_ntop�𗘗p���邽�߂ɕK�v�Ȓǉ��̃w�b�_�[
#include <iostream>
#include <cstring>
#include <mmsystem.h>               //Sound
#pragma comment(lib, "winmm.lib")   //Sound
#pragma comment(lib, "ws2_32.lib")
void RecordAndSendData(SOCKET clientSocket);

int main() {
    // �|�[�g�ԍ�
    int port = 12345;

    // Winsock�̏�����
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // �\�P�b�g�̍쐬
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // ���[�J���A�h���X�̐ݒ�
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // �\�P�b�g�Ƀ��[�J���A�h���X���o�C���h
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // �N���C�A���g�̐ڑ��҂�
    listen(listenSocket, 1);
    std::cout << "Waiting for client connection..." << std::endl;

    // �N���C�A���g�̐ڑ��󂯓���
    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected." << std::endl;

    // �����f�[�^�̘^���Ƒ��M���J�n
    RecordAndSendData(clientSocket);

    // �\�P�b�g�̃N���[�Y
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}

void RecordAndSendData(SOCKET clientSocket) {
    // �����f�[�^�̘^���Ƒ��M
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
        // �����f�[�^���N���C�A���g�ɑ��M
        send(clientSocket, buffer, bufferSize, 0);
        waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
        waveInStart(hWaveIn);
        Sleep(1);
    }

    waveInClose(hWaveIn);
}