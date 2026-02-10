#include <windows.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <ctime>

struct Job {
    int id;
    time_t createdAt;
};

std::queue<Job> buffer;
const int MAX_SIZE = 5;
std::mutex mtx;
std::condition_variable cv;
int jobCounter = 1;

HWND hStatus;

void updateStatus(const std::string& msg) {
    SetWindowTextA(hStatus, msg.c_str());
}

void producer() {
    std::unique_lock<std::mutex> lock(mtx);
    if (buffer.size() == MAX_SIZE) {
        updateStatus("Buffer full. Producer waiting...");
        return;
    }
    Job job{ jobCounter++, time(nullptr) };
    buffer.push(job);
    updateStatus("Produced Job ID: " + std::to_string(job.id));
}

void consumer() {
    std::unique_lock<std::mutex> lock(mtx);
    if (buffer.empty()) {
        updateStatus("Buffer empty. Consumer waiting...");
        return;
    }
    Job job = buffer.front();
    buffer.pop();
    updateStatus("Consumed Job ID: " + std::to_string(job.id));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND:
        if (wp == 1)
            std::thread(producer).detach();
        else if (wp == 2)
            std::thread(consumer).detach();
        else if (wp == 3)
            PostQuitMessage(0);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmd) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "PC_GUI";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        "PC_GUI", "Producer Consumer Simulator",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        200, 200, 400, 250,
        NULL, NULL, hInst, NULL
    );

    CreateWindow("BUTTON", "Produce", WS_VISIBLE | WS_CHILD,
        30, 30, 100, 30, hwnd, (HMENU)1, NULL, NULL);

    CreateWindow("BUTTON", "Consume", WS_VISIBLE | WS_CHILD,
        150, 30, 100, 30, hwnd, (HMENU)2, NULL, NULL);

    CreateWindow("BUTTON", "Exit", WS_VISIBLE | WS_CHILD,
        270, 30, 80, 30, hwnd, (HMENU)3, NULL, NULL);

    hStatus = CreateWindow("STATIC", "System Ready",
        WS_VISIBLE | WS_CHILD,
        30, 90, 300, 30, hwnd, NULL, NULL, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
