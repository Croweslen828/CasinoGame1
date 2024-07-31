#include <iostream>
#include <string>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <cstdlib>
#include <random>
#include "auth.h"

using namespace std;



// All above is SQL stuff
 
int main() {
    SQLHENV hEnv;
    SQLHDBC hDbc;
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
    globalData global;

   /*
     _____  _____    ______           __ 
    / ____ |  __ \  / __  \ \        / / 
    | |    | |__)  | |  |  \ \  /\  / /  
    | |    |  _  / | |  | | \ \/  \/ /   
    | |____| | \ \ | |__| |  \  /\  /    
    \_____ |_|   \_\______/   \/  \/    
                          
   LAST UPDATED 7/30 @ 1830 hrs

   github
   */ 

    while (!global.exitCheck) {
        cout << "Connecting to database....." << endl;
        if (!initializeEnvironment(hEnv)) {
            return 1;
        }

        if (!connectToDatabase(hDbc, hEnv)) {
            return 1;
        }
        cout << "Loading successful." << endl;

        if (!loginMenu(hDbc, global)) {
            cerr << "Main menu has failed to load. " << endl;
            cleanup(hEnv, hDbc, hStmt);
            // No return here, just retry
        }

        while (global.loginCheck) {
            if (!mainMenu(hDbc, global)) {
                cerr << "Game menu has failed to load." << endl;
            }
            if (!gameMenu(hDbc, global)) {
                cerr << "Game menu has failed to load." << endl;
            }
            if (global.exitCheck) {
                break; // Break out of the game loop if exit is requested
            }
        }

        // Additional checks or cleanup can be added here if necessary
    }

    // Cleanup resources and exit the program
    cleanup(hEnv, hDbc, hStmt);
    return 0;
}
