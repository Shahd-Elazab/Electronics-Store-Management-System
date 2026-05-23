/*******************************************************************************
 * FILE: main.cpp
 *
 * ROLE:
 * Entry point for the Store application.
 * Initialises all subsystems (Inventory, userManager, FileManager),
 * loads persisted data, then runs the top-level menu loop.
 *
 * Flow:
 *   1. Load all saved data (products, users, orders).
 *   2. Show the main menu: Sign Up / Login / Exit.
 *   3. After login the appropriate menu (Customer or Admin) is driven
 *      entirely by userManager::login(), which already contains the
 *      per-role sub-menu loops.
 *   4. On exit, save all data back to disk.
 *******************************************************************************/

#include "Store.h"
#include <limits>
int main() {
    // ── Core singletons / containers ────────────────────────────────────────
    Inventory     inventory;
    userManager*  um      = userManager::getUMinstance();
    vector<Order> orders;

    // ── Persistence layer ────────────────────────────────────────────────────
    FileManager fm(inventory, *um, orders);
    fm.loadAll();          // loads Warehouse.csv, users.csv, Orders.csv

    // ── Make sure at least one Admin exists ──────────────────────────────────
    // Only prompts when no Admin account is found after loading users.csv.
    um->ensureAdminExists();

    // ── Main application loop ────────────────────────────────────────────────
    int choice = 0;
    do {
        cout << "========================================" << endl;
        cout << "   WELCOME TO THE TECH STORE SYSTEM     " << endl;
        cout << "========================================" << endl;
        cout << "\n--- MAIN MENU ---" << endl;
        cout << "0) Exit Program" << endl;
        cout << "1) Sign Up (New Customer)" << endl;
        cout << "2) Login (Existing User)" << endl;
        cout << "Selection: ";

        if (!(cin >> choice)) {          // handle non-numeric input
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = -1;
        } else {
            cin.ignore();
        }

        switch (choice) {
            case 1:
                um->signUp();
                break;

            case 2: {
                // login() drives the full per-role sub-menu loop.
                // Orders are now saved to disk immediately after each checkout
                // inside login(), so order_count is always persisted correctly.
                User* user = um->login(inventory, orders, fm);
                (void)user;   // currently unused after return
                break;
            }

            case 0:
                cout << "\nSaving all data before exit...\n";
                fm.saveAll();
                cout << "Thank you for using the Store. Goodbye!\n";
                break;

            default:
                cout << "Invalid choice. Please enter 0, 1, or 2.\n";
                break;
        }

    } while (choice != 0);

    return 0;
}
