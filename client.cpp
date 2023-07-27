// client.cpp
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
void ReceiveAndPlayData(SOCKET serverSocket);

int main() {
    // �|�[�g�ԍ���IP�A�h���X
    int port = 12345;
    const char* ipAddress = "127.0.0.1"; // �T�[�o�[�ƃN���C�A���g�������}�V����œ��삷��ꍇ

    // Winsock�̏�����
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    // �\�P�b�g�̍쐬
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    // �ڑ���̐ݒ�
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress, &serverAddr.sin_addr); // inet_addr��inet_pton�ɕύX

    // �T�[�o�[�ɐڑ�
    if (connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // ��M���������f�[�^�̍Đ�
    ReceiveAndPlayData(serverSocket);

    // �\�P�b�g�̃N���[�Y
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

void ReceiveAndPlayData(SOCKET serverSocket) {
    // �����f�[�^�̎�M�ƍĐ�
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
        // �T�[�o�[���特���f�[�^����M
        int bytesRead = recv(serverSocket, buffer, bufferSize, 0);
        if (bytesRead <= 0) {
            break; // �T�[�o�[���ؒf�����ꍇ�͎�M���[�v���I��
        }

        // �����f�[�^���Đ�
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