#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "DBManager.h"
#include "ResourceManager.h"

using namespace std;

// Color handling
void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Input validation
int GetIntInput() {
    string input;
    int value;
    while (true) {
        cin >> input;
        try {
            value = stoi(input);
            return value;
        } catch (...) {
            SetColor(4); // RED
            cout << "[ERROR] Please enter a number: ";
            SetColor(7); // WHITE
        }
    }
}

void ShowMenu() {
    cout << "\n=== ByteKeeper: DAM System ===" << endl;
    cout << "1. Add File" << endl;
    cout << "2. List Files (Sort)" << endl;
    cout << "3. Search by Name (LIKE)" << endl;
    cout << "4. Intelligent Search" << endl;
    cout << "5. Statistics (COUNT/SUM)" << endl;
    cout << "6. Delete File (Soft Delete)" << endl;
    cout << "7. Recycle Bin (Restore)" << endl;
    cout << "8. Pagination" << endl;
    cout << "9. Export (CSV/Report)" << endl;
    cout << "10. Cleanup Old Data" << endl;
    cout << "11. Server Ping" << endl;
    cout << "12. Change Database" << endl;
    cout << "0. Exit" << endl;
    cout << "Choice: ";
}

int main() {
    DBManager db;

    if (!db.Connect("ByteKeeperDB")) {
        SetColor(4);
        cout << "Critical Error: Could not connect to DB." << endl;
        SetColor(7);
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
            cout << "Name: "; cin >> name;
            cout << "Size: "; size = GetIntInput();
            cout << "Cat ID: "; cid = GetIntInput();
            cout << "Owner ID: "; oid = GetIntInput();
            rm.AddFile(name, size, cid, oid);
            break;
        }
        case 2: {
            cout << "Sort by (Name/Size/ID): ";
            string s; cin >> s;
            rm.GetFiles(s);
            break;
        }
        case 3: {
            cout << "Part of name: ";
            string p; cin >> p;
            rm.SearchByName(p);
            break;
        }
        case 4: {
            cout << "Keywords: ";
            string q; cin.ignore(); getline(cin, q);
            rm.IntelligentSearch(q);
            break;
        }
        case 5:
            rm.ShowStatistics();
            break;
        case 6: {
            cout << "File ID to delete: ";
            int id = GetIntInput();
            rm.SoftDelete(id);
            break;
        }
        case 7: {
            rm.ShowRecycleBin();
            cout << "Restore ID (0 to cancel): ";
            int id = GetIntInput();
            if (id > 0) rm.RestoreFile(id);
            break;
        }
        case 8: {
            cout << "Page number: ";
            int p = GetIntInput();
            rm.GetFilesPaged(p);
            break;
        }
        case 9: {
            cout << "1. CSV\n2. Report\nChoice: ";
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
            cout << "New DB Name: ";
            string nb; cin >> nb;
            if (db.ChangeDatabase(nb)) {
                rm = ResourceManager(db.GetConnection());
            }
            break;
        }
        case 0:
            cout << "Goodbye!" << endl;
            break;
        default:
            SetColor(4);
            cout << "Invalid choice." << endl;
            SetColor(7);
        }
    }
    return 0;
}
