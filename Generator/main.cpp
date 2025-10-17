/*******************************************************************************
 * Файл: main.cpp (Проект: Generator)
 * Призначення: Створює файл даних та заповнює його випадковими числами.
 * Також створює системний м'ютекс.
 * Автор: Круковський Данило
 ******************************************************************************/

#include <windows.h>
#include <iostream>
#include <time.h>
#include "../common.h"

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    srand((unsigned)time(NULL));

    std::cout << "Запуск Генератора..." << std::endl;

    HANDLE hMutex = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
    if (hMutex == NULL) {
        std::cerr << "Не вдалося створити м'ютекс. Помилка: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "М'ютекс створено. Отримання блокування..." << std::endl;

    DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);
    if (waitResult != WAIT_OBJECT_0) {
        std::cerr << "Не вдалося отримати м'ютекс. Помилка: " << GetLastError() << std::endl;
        CloseHandle(hMutex);
        return 1;
    }

    std::cout << "Блокування отримано. Генерація даних..." << std::endl;

    __try {
        hFile = CreateFile(
            DATA_FILENAME,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            std::cerr << "Не вдалося створити файл. Помилка: " << GetLastError() << std::endl;
            __leave;
        }

        int numbers[ARRAY_SIZE];
        for (int i = 0; i < ARRAY_SIZE; ++i) {
            numbers[i] = 10 + (rand() % 91);
        }

        DWORD bytesWritten;
        if (!WriteFile(hFile, numbers, sizeof(numbers), &bytesWritten, NULL)) {
            std::cerr << "Не вдалося записати у файл. Помилка: " << GetLastError() << std::endl;
        }
        else {
            std::wcout << L"Файл даних '" << DATA_FILENAME << L"' створено та заповнено." << std::endl;
        }

        CloseHandle(hFile);

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        std::cerr << "Стався виняток під час запису файлу!" << std::endl;
    }

    std::cout << "Звільнення блокування..." << std::endl;

    ReleaseMutex(hMutex);

    std::cout << "Генератор завершив роботу. Натисніть Enter для виходу." << std::endl;
    std::cin.get();

    CloseHandle(hMutex);
    return 0;
}

// --- Кінець файлу main.cpp (Generator) ---