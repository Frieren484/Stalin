#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "DBManager.h"
#include "ResourceManager.h"

using namespace std;

// ункция для установки цвета текста
void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// алидация числового ввода
int GetIntInput() {
    string input;
    int value;
    while (true) {
        cin >> input;
        try {
            value = stoi(input);
            return value;
        } catch (...) {
            SetColor(4); // расный
            cout << "[Ш] ожалуйста, введите число: ";
            SetColor(7); // елый
        }
    }
}

void ShowMenu() {
    cout << "\n=== ByteKeeper: Система управления активами ===" << endl;
    cout << "1. обавить новый ресурс" << endl;
    cout << "2. Список всех ресурсов (Сортировка)" << endl;
    cout << "3. оиск по имени (LIKE)" << endl;
    cout << "4. нтеллектуальный многословный поиск" << endl;
    cout << "5. Статистика (оличество / ес)" << endl;
    cout << "6. далить в корзину (Soft Delete)" << endl;
    cout << "7. абота с корзиной (осстановление)" << endl;
    cout << "8. остраничный просмотр (Pagination)" << endl;
    cout << "9. кспорт данных (CSV / тчет)" << endl;
    cout << "10. чистка старых данных (>30 дней)" << endl;
    cout << "11. роверка связи с сервером (Ping)" << endl;
    cout << "12. Сменить текущую базу данных" << endl;
    cout << "0. ыйти из программы" << endl;
    cout << "аш выбор: ";
}

int main() {
    // ключаем поддержку русского языка в консоли
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    DBManager db;

    if (!db.Connect("ByteKeeperDB")) {
        SetColor(4);
        cout << "ритическая ошибка: невозможно подключиться к ." << endl;
        SetColor(7);
        system("pause");
        return 1;
    }

    ResourceManager rm(db.GetConnection());
    int choice = -1;

    while (choice != 0) {
        ShowMenu();
        choice = GetIntInput();

        switch (choice) {
        case 1: {
            string name; long long size; int cid, oid;
            cout << "мя файла (с расширением): "; cin >> name;
            cout << "азмер в байтах: "; size = GetIntInput();
            cout << "ID категории: "; cid = GetIntInput();
            cout << "ID владельца: "; oid = GetIntInput();
            rm.AddFile(name, size, cid, oid);
            break;
        }
        case 2: {
            cout << "Сортировать по (Name/Size/ID): ";
            string s; cin >> s;
            rm.GetFiles(s);
            break;
        }
        case 3: {
            cout << "ведите имя или его часть: ";
            string p; cin >> p;
            rm.SearchByName(p);
            break;
        }
        case 4: {
            cout << "ведите слова через пробел: ";
            string q; cin.ignore(); getline(cin, q);
            rm.IntelligentSearch(q);
            break;
        }
        case 5:
            rm.ShowStatistics();
            break;
        case 6: {
            cout << "ведите ID ресурса для удаления: ";
            int id = GetIntInput();
            rm.SoftDelete(id);
            break;
        }
        case 7: {
            rm.ShowRecycleBin();
            cout << "ведите ID для восстановления (0 для отмены): ";
            int id = GetIntInput();
            if (id > 0) rm.RestoreFile(id);
            break;
        }
        case 8: {
            cout << "ведите номер страницы: ";
            int p = GetIntInput();
            rm.GetFilesPaged(p);
            break;
        }
        case 9: {
            cout << "1. кспорт в CSV (export.csv)\n2. Текстовый отчет (report.txt)\nыбор: ";
            int ex = GetIntInput();
            if (ex == 1) rm.ExportToCSV("export.csv");
            else rm.ExportReport("report.txt");
            break;
        }
        case 10:
            rm.CleanupOldData();
            break;
        case 11:
            db.Ping();
            break;
        case 12: {
            cout << "ведите имя новой : ";
            string nb; cin >> nb;
            if (db.ChangeDatabase(nb)) {
                rm = ResourceManager(db.GetConnection());
            }
            break;
        }
        case 0:
            cout << "авершение работы..." << endl;
            break;
        default:
            SetColor(4);
            cout << "екорректный выбор. овторите ввод." << endl;
            SetColor(7);
        }
    }
    return 0;
}
