#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "DBManager.h"
#include "ResourceManager.h"

using namespace std;

void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void WaitEnter() {
    cout << "\nажмите Enter, чтобы продолжить...";
    cin.ignore();
    cin.get();
}

int GetIntInput() {
    string input;
    int value;
    while (true) {
        cin >> input;
        try {
            value = stoi(input);
            return value;
        } catch (...) {
            SetColor(4);
            cout << "[Ш] ведите число: ";
            SetColor(7);
        }
    }
}

void ShowMenu() {
    system("cls");
    SetColor(6); // елтый для заголовков
    cout << "=== ByteKeeper: Система управления активами ===" << endl;
    SetColor(7);
    cout << "1. обавить новый ресурс" << endl;
    cout << "2. Список всех ресурсов (JOIN + Сортировка)" << endl;
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
    cout << "13. [дм] далить категорию (Integrity check)" << endl;
    cout << "0. ыйти из программы" << endl;
    cout << "аш выбор: ";
}

int main() {
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
            cout << "мя файла: "; cin >> name;
            cout << "азмер (байт): "; size = GetIntInput();
            cout << "ID категории: "; cid = GetIntInput();
            cout << "ID владельца: "; oid = GetIntInput();
            rm.AddFile(name, size, cid, oid);
            WaitEnter();
            break;
        }
        case 2: {
            cout << "Сортировка (Name/Size/ID): "; string s; cin >> s;
            rm.GetFiles(s);
            WaitEnter();
            break;
        }
        case 3: {
            cout << "мя или часть: "; string p; cin >> p;
            rm.SearchByName(p);
            WaitEnter();
            break;
        }
        case 4: {
            cout << "Слова через пробел: "; string q; cin.ignore(); getline(cin, q);
            rm.IntelligentSearch(q);
            WaitEnter();
            break;
        }
        case 5:
            rm.ShowStatistics();
            WaitEnter();
            break;
        case 6: {
            cout << "ID для удаления: "; int id = GetIntInput();
            rm.SoftDelete(id);
            WaitEnter();
            break;
        }
        case 7: {
            rm.ShowRecycleBin();
            cout << "ID для восстановления (0 для отмены): ";
            int id = GetIntInput();
            if (id > 0) rm.RestoreFile(id);
            WaitEnter();
            break;
        }
        case 8: {
            cout << "Страница: "; int p = GetIntInput();
            rm.GetFilesPaged(p);
            WaitEnter();
            break;
        }
        case 9: {
            cout << "1. CSV\n2. тчет\nыбор: "; int ex = GetIntInput();
            if (ex == 1) rm.ExportToCSV("export.csv");
            else rm.ExportReport("report.txt");
            WaitEnter();
            break;
        }
        case 10:
            rm.CleanupOldData();
            WaitEnter();
            break;
        case 11:
            db.Ping();
            WaitEnter();
            break;
        case 12: {
            cout << "мя новой : "; string nb; cin >> nb;
            if (db.ChangeDatabase(nb)) rm = ResourceManager(db.GetConnection());
            WaitEnter();
            break;
        }
        case 13: {
            cout << "ID категории для удаления: "; int id = GetIntInput();
            rm.DeleteCategory(id);
            WaitEnter();
            break;
        }
        case 0:
            cout << "о свидания!" << endl;
            break;
        default:
            SetColor(4); cout << "еверный выбор." << endl; SetColor(7);
            WaitEnter();
        }
    }
    return 0;
}
