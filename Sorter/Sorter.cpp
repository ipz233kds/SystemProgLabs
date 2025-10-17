/*******************************************************************************
 * Файл:    Sorter.cpp (Проект: Sorter)
 * Призначення: Віконна програма. Створює проекцію файлу даних у пам'ять
 * та сортує його при натисканні Пробілу.
 * Автор: Круковський Данило
 ******************************************************************************/

#include "framework.h"
#include "Sorter.h"
#include <windows.h>
#include "../common.h"

HANDLE hMapFile;
HANDLE hMutex;
int* pData;
HWND g_hWnd;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI SortThread(LPVOID lpParam);

DWORD WINAPI SortThread(LPVOID lpParam) {
    SetWindowText(g_hWnd, L"Сортування...");

    for (int i = 0; i < ARRAY_SIZE - 1; ++i) {
        for (int j = 0; j < ARRAY_SIZE - i - 1; ++j) {

            WaitForSingleObject(hMutex, INFINITE);
            __try {
                if (pData[j] > pData[j + 1]) {
                    int temp = pData[j];
                    pData[j] = pData[j + 1];
                    pData[j + 1] = temp;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                MessageBox(g_hWnd, L"Помилка доступу до спільної пам'яті!", L"Помилка Сортувальника", MB_OK);
            }
            ReleaseMutex(hMutex);

            Sleep(100);
        }
    }

    SetWindowText(g_hWnd, L"Роботу завершено. (ПРОБІЛ для повтору)");
    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"SORTER_CLASS";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    HWND hWnd = CreateWindowW(L"SORTER_CLASS", L"Sorter", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 300, 200, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
    {
        g_hWnd = hWnd;
        SetWindowText(hWnd, L"Сортувальник (Натисніть ПРОБІЛ для старту)");

        hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
        if (hMutex == NULL) {
            MessageBox(hWnd, L"Не вдалося відкрити м'ютекс! Спочатку запустіть Генератор.", L"Помилка", MB_OK);
            PostQuitMessage(1);
            return -1;
        }

        HANDLE hFile = CreateFile(DATA_FILENAME, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            MessageBox(hWnd, L"Не вдалося відкрити файл даних! Спочатку запустіть Генератор.", L"Помилка", MB_OK);
            CloseHandle(hMutex);
            PostQuitMessage(1);
            return -1;
        }

        hMapFile = CreateFileMapping(
            hFile,
            NULL,
            PAGE_READWRITE,
            0,
            ARRAY_SIZE * sizeof(int),
            MAP_OBJECT_NAME
        );
        CloseHandle(hFile);

        if (hMapFile == NULL) {
            MessageBox(hWnd, L"Не вдалося створити проекцію файлу!", L"Помилка", MB_OK);
            CloseHandle(hMutex);
            PostQuitMessage(1);
            return -1;
        }

        pData = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, ARRAY_SIZE * sizeof(int));
        if (pData == NULL) {
            MessageBox(hWnd, L"Не вдалося відобразити файл у пам'ять!", L"Помилка", MB_OK);
            CloseHandle(hMapFile);
            CloseHandle(hMutex);
            PostQuitMessage(1);
            return -1;
        }
        break;
    }
    case WM_KEYDOWN:
        if (wParam == VK_SPACE) {
            HANDLE hThread = CreateThread(NULL, 0, SortThread, NULL, 0, NULL);
            if (hThread) {
                CloseHandle(hThread);
            }
        }
        break;

    case WM_DESTROY:
        if (pData) UnmapViewOfFile(pData);
        if (hMapFile) CloseHandle(hMapFile);
        if (hMutex) CloseHandle(hMutex);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// --- Кінець файлу Sorter.cpp (Sorter) ---