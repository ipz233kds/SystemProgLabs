/*******************************************************************************
 * Файл: main.cpp (Проект: ReverseSorter)
 * Призначення: Консольна програма, яка сортує дані у спільній пам'яті
 * у зворотному порядку іншим методом.
 * Автор: Круковський Данило
 ******************************************************************************/

#include <windows.h>
#include <iostream>
#include "../common.h"

void SelectionSortDescending(int* pData, HANDLE hMutex) {
    for (int i = 0; i < ARRAY_SIZE - 1; ++i) {
        int max_idx = i;

        for (int j = i + 1; j < ARRAY_SIZE; ++j) {
            WaitForSingleObject(hMutex, INFINITE);
            int val_j, val_max;
            __try {
                val_j = pData[j];
                val_max = pData[max_idx];
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                std::cerr << "Memory read error!" << std::endl;
                val_j = 0; val_max = 0;
            }
            ReleaseMutex(hMutex);

            if (val_j > val_max) {
                max_idx = j;
            }
        }

        if (max_idx != i) {
            WaitForSingleObject(hMutex, INFINITE);
            __try {
                int temp = pData[max_idx];
                pData[max_idx] = pData[i];
                pData[i] = temp;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                std::cerr << "Memory write error!" << std::endl;
            }
            ReleaseMutex(hMutex);
        }

        std::cout << ".";
        Sleep(100);
    }
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    std::cout << "Запуск Реверс-Сортувальника..." << std::endl;

    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
    if (hMutex == NULL) {
        std::cerr << "Не вдалося відкрити м'ютекс!" << std::endl;
        return 1;
    }

    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MAP_OBJECT_NAME);
    if (hMapFile == NULL) {
        std::cerr << "Не вдалося відкрити проекцію!" << std::endl;
        CloseHandle(hMutex);
        return 1;
    }

    int* pData = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, ARRAY_SIZE * sizeof(int));
    if (pData == NULL) {
        std::cerr << "Не вдалося відобразити файл!" << std::endl;
        CloseHandle(hMapFile);
        CloseHandle(hMutex);
        return 1;
    }

    std::cout << "Сортування у зворотному порядку..." << std::endl;
    SelectionSortDescending(pData, hMutex);
    std::cout << "\nЗворотне сортування завершено." << std::endl;

    UnmapViewOfFile(pData);
    CloseHandle(hMapFile);
    CloseHandle(hMutex);

    system("pause");
    return 0;
}

// --- Кінець файлу main.cpp (ReverseSorter) ---