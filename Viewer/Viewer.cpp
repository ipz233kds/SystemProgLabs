/*******************************************************************************
 * Файл:    Viewer.cpp (Проект: Viewer)
 * Призначення: Віконна програма. Відкриває проекцію файлу та за таймером
 * відображає її вміст у вигляді "стовпчиків" із зірочок.
 * Автор: Круковський Данило
 ******************************************************************************/

#include "framework.h"
#include "Viewer.h"
#include <windows.h>
#include "../common.h"

HANDLE hMapFile;
HANDLE hMutex;
int* pData;
int localData[ARRAY_SIZE];

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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
    wcex.lpszClassName = L"VIEWER_CLASS";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    HWND hWnd = CreateWindowW(L"VIEWER_CLASS", L"Viewer", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 400, 600, nullptr, nullptr, hInstance, nullptr);
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
        SetWindowText(hWnd, L"Візуалізатор");

        hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
        if (hMutex == NULL) {
            MessageBox(hWnd, L"Не вдалося відкрити м'ютекс! Спочатку запустіть Генератор/Сортувальник.", L"Помилка", MB_OK);
            PostQuitMessage(1);
            return -1;
        }

        hMapFile = OpenFileMapping(
            FILE_MAP_READ,
            FALSE,
            MAP_OBJECT_NAME
        );

        if (hMapFile == NULL) {
            MessageBox(hWnd, L"Не вдалося відкрити проекцію файлу! Спочатку запустіть Сортувальник.", L"Помилка", MB_OK);
            CloseHandle(hMutex);
            PostQuitMessage(1);
            return -1;
        }

        pData = (int*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, ARRAY_SIZE * sizeof(int));
        if (pData == NULL) {
            MessageBox(hWnd, L"Не вдалося відобразити файл у пам'ять!", L"Помилка", MB_OK);
            CloseHandle(hMapFile);
            CloseHandle(hMutex);
            PostQuitMessage(1);
            return -1;
        }

        SetTimer(hWnd, TIMER_ID, 500, NULL);
        break;
    }
    case WM_TIMER:
        if (wParam == TIMER_ID) {

            WaitForSingleObject(hMutex, INFINITE);
            __try {
                memcpy(localData, pData, sizeof(localData));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
            }
            ReleaseMutex(hMutex);

            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        HBRUSH hGreenBrush = CreateSolidBrush(RGB(0, 180, 0));

        HGDIOBJ hOldBrush = SelectObject(hdc, hGreenBrush);

        SelectObject(hdc, GetStockObject(NULL_PEN));

        int yPos = 10;

        for (int i = 0; i < ARRAY_SIZE; ++i) {
            int value = localData[i];

            int stripWidth = value * 3;

            Rectangle(hdc, 10, yPos, 10 + stripWidth, yPos + 18);

            yPos += 20;
        }

        SelectObject(hdc, hOldBrush);
        DeleteObject(hGreenBrush);


        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        KillTimer(hWnd, TIMER_ID);
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

// --- Кінець файлу Viewer.cpp (Viewer) ---