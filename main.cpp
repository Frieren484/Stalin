#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <limits>
#include <fcntl.h>
#include <io.h>
#include "DBManager.h"
#include "ResourceManager.h"

using namespace std;

// ункция для установки цвета
void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// жидание нажатия Enter (Unicode)
void WaitEnter() {
    wcout << L"\n\u041d\u0430\u0436\u043c\u0438\u0442\u0435 Enter, \u0447\u0442\u043e\u0431\u044b \u043f\u0440\u043e\u0434\u043e\u043b\u0436\u0438\u0442\u044c..."; 
    wcin.ignore((numeric_limits<streamsize>::max)(), L'\n');
    wcin.get();
}

// езопасный ввод числа
int GetIntInput() {
    wstring input;
    int value;
    while (true) {
        wcin >> input;
        try {
            value = stoi(input);
            return value;
        } catch (...) {
            SetColor(4);
            wcout << L"[\u041e\u0428\u0418\u0411\u041a\u0410] \u0412\u0432\u0435\u0434\u0438\u0442\u0435 \u0447\u0438\u0441\u043b\u043e: ";
            SetColor(7);
        }
    }
}

// тображение меню (используем HEX-коды для 100% надежности)
void ShowMenu() {
    system("cls");
    SetColor(6);
    wcout << L"=== ByteKeeper: \u0421\u0438\u0441\u0442\u0435\u043c\u0430 \u0443\u043f\u0440\u0430\u0432\u043b\u0435\u043d\u0438\u044f \u0430\u043a\u0442\u0438\u0432\u0430\u043c\u0438 ===" << endl;
    SetColor(7);
    wcout << L"1. \u0414\u043e\u0431\u0430\u0432\u0438\u0442\u044c \u043d\u043e\u0432\u044b\u0439 \u0440\u0435\u0441\u0443\u0440\u0441" << endl;
    wcout << L"2. \u0421\u043f\u0438\u0441\u043e\u043a \u0432\u0441\u0435\u0445 \u0440\u0435\u0441\u0443\u0440\u0441\u043e\u0432 (JOIN + \u0421\u043e\u0440\u0442\u0438\u0440\u043e\u0432\u043a\u0430)" << endl;
    wcout << L"3. \u041f\u043e\u0438\u0441\u043a \u043f\u043e \u0438\u043c\u0435\u043d\u0438 (LIKE)" << endl;
    wcout << L"4. \u0418\u043d\u0442\u0435\u043b\u043b\u0435\u043a\u0442\u0443\u0430\u043b\u044c\u043d\u044b\u0439 \u043c\u043d\u043e\u0433\u043e\u0441\u043b\u043e\u0432\u043d\u044b\u0439 \u043f\u043e\u0438\u0441\u043a" << endl;
    wcout << L"5. \u0421\u0442\u0430\u0442\u0438\u0441\u0442\u0438\u043a\u0430 (\u041a\u043e\u043b\u0438\u0447\u0435\u0441\u0442\u0432\u043e / \u0412\u0435\u0441)" << endl;
    wcout << L"6. \u0423\u0434\u0430\u043b\u0438\u0442\u044c \u0432 \u043a\u043e\u0440\u0437\u0438\u043d\u0443 (Soft Delete)" << endl;
    wcout << L"7. \u0420\u0430\u0431\u043e\u0442\u0430 \u0441 \u043a\u043e\u0440\u0437\u0438\u043d\u043e\u0439 (\u0412\u043e\u0441\u0441\u0442\u0430\u043d\u043e\u0432\u043b\u0435\u043d\u0438\u0435)" << endl;
    wcout << L"8. \u041f\u043e\u0441\u0442\u0440\u0430\u043d\u0438\u0447\u043d\u044b\u0439 \u043f\u0440\u043e\u0441\u043c\u043e\u0442\u0440 (Pagination)" << endl;
    wcout << L"9. \u042d\u043a\u0441\u043f\u043e\u0440\u0442 \u0434\u0430\u043d\u043d\u044b\u0445 (CSV / \u041e\u0442\u0447\u0435\u0442)" << endl;
    wcout << L"10. \u0427\u0438\u0441\u0442\u043a\u0430 \u0441\u0442\u0430\u0440\u044b\u0445 \u0434\u0430\u043d\u043d\u044b\u0445 (>30 \u0434\u043d\u0435\u0439)" << endl;
    wcout << L"11. \u041f\u0440\u043e\u0432\u0435\u0440\u043a\u0430 \u0441\u0432\u044f\u0437\u0438 \u0441 \u0441\u0435\u0440\u0432\u0435\u0440\u043e\u043c (Ping)" << endl;
    wcout << L"12. \u0421\u043c\u0435\u043d\u0438\u0442\u044c \u0442\u0435\u043a\u0443\u0449\u0443\u044e \u0431\u0430\u0437\u0443 \u0434\u0430\u043d\u043d\u044b\u0445" << endl;
    wcout << L"13. [\u0410\u0434\u043c] \u0423\u0434\u0430\u043b\u0438\u0442\u044c \u043a\u0430\u0442\u0435\u0433\u043e\u0440\u0438\u044e (Integrity check)" << endl;
    wcout << L"0. \u0412\u044b\u0439\u0442\u0438 \u0438\u0437 \u043f\u0440\u043e\u0433\u0440\u0430\u043c\u043c\u044b" << endl;
    SetColor(6); wcout << L"\u0412\u0430\u0448 \u0432\u044b\u0431\u043e\u0440: "; SetColor(7);
}

int main() {
    // ключаем Unicode-режим консоли (самый надежный способ на Windows)
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);

    DBManager db;

    if (!db.Connect("ByteKeeperDB")) {
        SetColor(4);
        wcout << L"\u041a\u0440\u0438\u0442\u0438\u0447\u0435\u0441\u043a\u0430\u044f \u043e\u0448\u0438\u0431\u043a\u0430: \u043d\u0435\u0432\u043e\u0437\u043c\u043e\u0436\u043d\u043e \u043f\u043e\u0434\u043a\u043b\u044e\u0447\u0438\u0442\u044c\u0441\u044f \u043a \u0411\u0414." << endl;
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
            wstring wname; long long size; int cid, oid;
            wcout << L"\u0418\u043c\u044f \u0444\u0430\u0439\u043b\u0430: "; wcin >> wname;
            string name(wname.begin(), wname.end());
            wcout << L"\u0420\u0430\u0437\u043c\u0435\u0440 (\u0431\u0430\u0439\u0442): "; size = (long long)GetIntInput();
            wcout << L"ID \u043a\u0430\u0442\u0435\u0433\u043e\u0440\u0438\u0438: "; cid = GetIntInput();
            wcout << L"ID \u0432\u043b\u0430\u0434\u0435\u043b\u044c\u0446\u0430: "; oid = GetIntInput();
            rm.AddFile(name, size, cid, oid);
            WaitEnter();
            break;
        }
        case 2: {
            wcout << L"\u0421\u043e\u0440\u0442\u0438\u0440\u043e\u0432\u043a\u0430 (Name/Size/ID): "; wstring ws; wcin >> ws;
            string s(ws.begin(), ws.end());
            rm.GetFiles(s);
            WaitEnter();
            break;
        }
        case 3: {
            wcout << L"\u0418\u043c\u044f \u0438\u043b\u0438 \u0447\u0430\u0441\u0442\u044c: "; wstring wp; wcin >> wp;
            string p(wp.begin(), wp.end());
            rm.SearchByName(p);
            WaitEnter();
            break;
        }
        case 4: {
            wcout << L"\u0421\u043b\u043e\u0432\u0430 \u0447\u0435\u0440\u0435\u0437 \u043f\u0440\u043e\u0431\u0435\u043b: "; wcin.ignore((numeric_limits<streamsize>::max)(), L'\n'); 
            wstring wq; getline(wcin, wq);
            string q(wq.begin(), wq.end());
            rm.IntelligentSearch(q);
            WaitEnter();
            break;
        }
        case 5:
            rm.ShowStatistics();
            WaitEnter();
            break;
        case 6: {
            wcout << L"ID \u0434\u043b\u044f \u0443\u0434\u0430\u043b\u0435\u043d\u0438\u044f: "; int id = GetIntInput();
            rm.SoftDelete(id);
            WaitEnter();
            break;
        }
        case 7: {
            rm.ShowRecycleBin();
            wcout << L"ID \u0434\u043b\u044f \u0432\u043e\u0441\u0441\u0442\u0430\u043d\u043e\u0432\u043b\u0435\u043d\u0438\u044f (0 \u0434\u043b\u044f \u043e\u0442\u043c\u0435\u043d\u044b): ";
            int id = GetIntInput();
            if (id > 0) rm.RestoreFile(id);
            WaitEnter();
            break;
        }
        case 8: {
            wcout << L"\u0421\u0442\u0440\u0430\u043d\u0438\u0446\u0430: "; int p = GetIntInput();
            rm.GetFilesPaged(p);
            WaitEnter();
            break;
        }
        case 9: {
            wcout << L"1. CSV\n2. \u041e\u0442\u0447\u0435\u0442\n\u0412\u044b\u0431\u043e\u0440: "; int ex = GetIntInput();
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
            wcout << L"\u0418\u043c\u044f \u043d\u043e\u0432\u043e\u0439 \u0411\u0414: "; wstring wnb; wcin >> wnb;
            string nb(wnb.begin(), wnb.end());
            if (db.ChangeDatabase(nb)) rm = ResourceManager(db.GetConnection());
            WaitEnter();
            break;
        }
        case 13: {
            wcout << L"ID \u043a\u0430\u0442\u0435\u0433\u043e\u0440\u0438\u0438 \u0434\u043b\u044f \u0443\u0434\u0430\u043b\u0435\u043d\u0438\u044f: "; int id = GetIntInput();
            rm.DeleteCategory(id);
            WaitEnter();
            break;
        }
        case 0:
            wcout << L"\u0414\u043e \u0441\u0432\u0438\u0434\u0430\u043d\u0438\u044f!" << endl;
            break;
        default:
            SetColor(4); wcout << L"\u041d\u0435\u0432\u0435\u0440\u043d\u044b\u0439 \u0432\u044b\u0431\u043e\u0440." << endl; SetColor(7);
            WaitEnter();
        }
    }
    return 0;
}