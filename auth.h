#include <iostream>
#include <string>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <cstdlib>
#include <random>
using namespace std;

struct globalData {
    std::string username;
    std::string password;
    double balance = 0.0;
    std::string loginQuery;
    std::string passwordCheck;
    std::string password2;
    bool exitCheck = false;
    bool loginCheck = false; 
    int winnings = 0; // Initialize winnings
    int adminPerms = 0;
    int accountNumber = 0; // Initialize accountNumber

    // Optional constructor for better initialization
    globalData() : balance(0.0), winnings(0), adminPerms(0), accountNumber(0), exitCheck(false), loginCheck(false) {}
};

// Function prototypes
void checkDiagnosticRecord(SQLHANDLE handle, SQLSMALLINT handleType, RETCODE retCode);
bool initializeEnvironment(SQLHENV &hEnv);
bool connectToDatabase(SQLHDBC &hDbc, SQLHENV hEnv);
bool executeQuery(SQLHDBC hDbc, const globalData &global);
void cleanup(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt);


void checkDiagnosticRecord(SQLHANDLE handle, SQLSMALLINT handleType, RETCODE retCode) {
    if (retCode == SQL_INVALID_HANDLE || retCode == SQL_ERROR) {
        cerr << "Error: Invalid handle or SQL error." << endl;
        return;
    }

    SQLSMALLINT i = 0;
    SQLINTEGER nativeError;
    SQLCHAR sqlState[7];
    SQLCHAR messageText[256];
    SQLSMALLINT textLength;
    while (SQLGetDiagRec(handleType, handle, ++i, sqlState, &nativeError, messageText, sizeof(messageText), &textLength) == SQL_SUCCESS) {
        cerr << "SQLSTATE: " << sqlState << ", Native Error: " << nativeError << ", Message: " << messageText << endl;
    }
}

bool initializeEnvironment(SQLHENV &hEnv) {
    SQLRETURN retCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error allocating environment handle." << endl;
        return false;
    }

    retCode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error setting ODBC version." << endl;
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    return true;
}

bool connectToDatabase(SQLHDBC &hDbc, SQLHENV hEnv) {
    SQLRETURN retCode = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error allocating connection handle." << endl;
        return false;
    }

    SQLCHAR connStr[] = "DRIVER={ODBC Driver 17 for SQL Server};SERVER=crow-sql.database.windows.net;DATABASE=crowsqldatbase;UID=dom;PWD=Password1*;";
    retCode = SQLDriverConnect(hDbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error connecting to the database." << endl;
        checkDiagnosticRecord(hDbc, SQL_HANDLE_DBC, retCode);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    cout << "Database connected successfully!" << endl;
    return true;
}

bool executeQuery(SQLHDBC hDbc, const globalData &global) {
    SQLHSTMT hStmt;
    SQLRETURN retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error allocating statement handle." << endl;
        return false;
    }

    // Build the query string
    globalData modifiedGlobal = global;
    modifiedGlobal.loginQuery = "SELECT * FROM USERS WHERE username = '" + modifiedGlobal.username + "' AND password = '" + modifiedGlobal.password + "'";

    // Convert global.loginQuery to C-style string
    SQLCHAR query[1024];
    strncpy((char*)query, modifiedGlobal.loginQuery.c_str(), sizeof(query) - 1);
    query[sizeof(query) - 1] = '\0'; // Null-terminate the string

    retCode = SQLExecDirect(hStmt, query, SQL_NTS);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error executing query." << endl;
        checkDiagnosticRecord(hStmt, SQL_HANDLE_STMT, retCode);
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    cout << "Query executed successfully." << endl;
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return true;
}

void cleanup(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt) {
    if (hStmt != SQL_NULL_HSTMT) {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }
    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}



bool login(SQLHDBC hDbc, globalData &global) {
    bool loggedIn = false;

    while (!loggedIn) {
        cout << "Below please enter your username:" << endl;
        cin >> global.username;
        cout << "Now your password:" << endl;
        cin >> global.password;

        // Construct the SQL query string with proper quoting
        global.loginQuery = "SELECT * FROM USERS WHERE username = '" + global.username + "' AND password = '" + global.password + "'";

        // Create a statement handle
        SQLHSTMT hStmt;
        SQLRETURN retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
        if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
            cerr << "Error allocating statement handle." << endl;
            return false;
        }

        // Convert global.loginQuery to C-style string
        SQLCHAR query[1024];
        strncpy((char*)query, global.loginQuery.c_str(), sizeof(query) - 1);
        query[sizeof(query) - 1] = '\0'; // Null-terminate the string

        // Output the query for debugging
        cout << "Logging you in...please wait... " << endl;
        global.loginCheck = true; 
        Sleep(2000);

        // Execute SQL query
        retCode = SQLExecDirect(hStmt, query, SQL_NTS);
        if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
            cerr << "Error executing query." << endl;
            checkDiagnosticRecord(hStmt, SQL_HANDLE_STMT, retCode);
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            continue; // Retry login if query execution fails
        }

        // Bind columns for the result set
       char usernameBuffer[256];
        char passwordBuffer[256];
        SQLLEN usernameLen, passwordLen;
        double balance;
        int isAdmin;
        int account; 

SQLBindCol(hStmt, 1, SQL_C_LONG, &account, sizeof(account), NULL); // Account number
SQLBindCol(hStmt, 2, SQL_C_CHAR, usernameBuffer, sizeof(usernameBuffer), &usernameLen); // Username
SQLBindCol(hStmt, 3, SQL_C_CHAR, passwordBuffer, sizeof(passwordBuffer), &passwordLen); // Password
SQLBindCol(hStmt, 4, SQL_C_DOUBLE, &balance, sizeof(balance), NULL); // Balance
SQLBindCol(hStmt, 5, SQL_C_LONG, &isAdmin, sizeof(isAdmin), NULL); // isAdmin



        // Fetch the result
        retCode = SQLFetch(hStmt);
        if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO) {
            // Update global data
            global.username = usernameBuffer;
            global.password = passwordBuffer;
            global.balance = balance;

            cout << "Logged in successfully." << endl;
            Sleep(1500);
            if(isAdmin == 1){
                global.adminPerms = 1;
                cout <<"Admin perms granted" << endl;
            }
            global.loginCheck = true;
            cout << "Your balance is : $" << global.balance << endl;
            Sleep(2000);
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
            loggedIn = true; // Exit the loop on successful login
        } else {
            cerr << "Incorrect login info. Please try again." << endl;
            checkDiagnosticRecord(hStmt, SQL_HANDLE_STMT, retCode);
            SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }
    }

    return true; // Return true if the user logged in successfully
}

bool newUser(SQLHDBC hDbc, globalData &global) {
    bool passwordCorr = false;
    SQLHSTMT hStmt;
    SQLRETURN retCode;

    // Registration logic
    cout <<"Please type in what you would like your username to be" << endl;
    cin >> global.username;

    while (!passwordCorr) {
        cout <<"What would you like your password to be?" << endl;
        cin >> global.password;
        cout <<"Please enter your password again" << endl;
        cin >> global.password2;

        if (global.password == global.password2) {
            cout <<"Passwords matched" << endl;
            passwordCorr = true;
            cout <<"User input successful, entering into database." << endl;
        } else {
            cout <<"Incorrect, try again." << endl;
        }
    }

    // Prepare the SQL statement
    const char *sql = "INSERT INTO USERS (username, password, balance) VALUES (?, ?, 5000)";
    retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error allocating statement handle." << endl;
        return false;
    }

    retCode = SQLPrepare(hStmt, (SQLCHAR*)sql, SQL_NTS);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error preparing SQL statement." << endl;
        checkDiagnosticRecord(hStmt, SQL_HANDLE_STMT, retCode);
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    // Bind parameters
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)global.username.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, (SQLPOINTER)global.password.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &global.balance, 0, NULL);
    
    // Execute the SQL statement
    retCode = SQLExecute(hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error executing SQL statement." << endl;
        checkDiagnosticRecord(hStmt, SQL_HANDLE_STMT, retCode);
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    cout <<"Registration successful!" << endl << "You have been given $5000 as a thank you for joining our casino" << endl <<"Returning to main menu" << endl;
    Sleep(2000); // Optional delay before returning to main menu

    return true;
}

bool loginMenu(SQLHDBC hDbc, globalData &global) {
    SQLHENV hEnv;
    SQLHSTMT hStmt = SQL_NULL_HSTMT;
  //  clearScreen();

    bool restartChk = false; // Initialize restartChk
    int regCheck;
    Sleep(2000);
    cout << "Welcome to Dom's Casino!!!!" << endl;
    Sleep(1000);

    while (!restartChk) {
        // Prompt user for action
        cout << "Type 1 for existing user, 2 to register or 3 to exit" << endl;
        cin >> regCheck;

        if (regCheck == 1) {   // Login screen
            if (!login(hDbc, global)) {
                cerr << "Login failed." << endl;
                // No return here, just retry
            } else {
                restartChk = true; // Exit the loop after a successful login
            }
        } else if (regCheck == 2) { // Registration screen
            if (!newUser(hDbc, global)) {
                cerr << "Registration failed." << endl;
                // No return here, just retry
            } else {
                // Registration was successful, so we return to the main menu loop
                restartChk = false; // Continue to prompt the user after registration
            }
        } else if (regCheck == 3) { // Quit
            restartChk = true;
            global.exitCheck = true; 
            cout << "Exiting the program. Come back soon!" << endl;
        } else {
            cerr << "Invalid option. Please enter 1, 2, or 3." << endl;
            // Do not return, just prompt again
        }
    }

    return true;
}

bool updateBalance(SQLHDBC hDbc, globalData &global) {
    // Build the SQL query string
    string updateBalanceQuery = "UPDATE USERS SET balance = " + to_string(global.balance) + " WHERE username = '" + global.username + "';";
    SQLHSTMT hStmt;
    SQLRETURN retCode;

    // Allocate statement handle
    retCode = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error allocating statement handle." << endl;
        return false;
    }

    // Convert updateBalanceQuery to C-style string
    SQLCHAR query[1024];
    strncpy((char*)query, updateBalanceQuery.c_str(), sizeof(query) - 1);
    query[sizeof(query) - 1] = '\0'; // Null-terminate the string

    // Execute SQL query
    retCode = SQLExecDirect(hStmt, query, SQL_NTS);
    if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO) {
        cerr << "Error executing query." << endl;
        checkDiagnosticRecord(hStmt, SQL_HANDLE_STMT, retCode);
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        return false;
    }

    // Clean up
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    return true;

    /* to update the balance, use this code

    if (!updateBalance(hDbc, global)) {
            cerr << "Failed to update balance in the database." << endl;
        }
    
    
    */
}


bool adminMenu(SQLHDBC hDbc, globalData &global){
cout <<"You are in the admin menu!" << endl;
// admin menu logic goes here 
return true;
}



bool logout(SQLHDBC hDbc, globalData &global){

    cout <<"okay " << global.username <<" logging you out and updating your balance. See you next time!" << endl;
    if (!updateBalance(hDbc, global)) {
           cerr << "Failed to update balance in the database." << endl;
                }  
    global.username = "";
    global.password = "";
    global.accountNumber = 0;
    global.balance = 0;
    cout <<"Returning to main menu" << endl;
    if (!loginMenu(hDbc, global)) {
           cerr << "Failed to update balance in the database." << endl;
                }  
return true;
}

bool randomNumberGame(SQLHDBC hDbc, globalData &global) {
    bool rngRestart = false;
    char restartChoice;
    bool rngRuleCheck = false;
    char rngRuleChoice;
    double rngBet;
    bool rngBetBalanceCheck = false;
    int playerNumber;
    bool rngWinCheck;
    bool playerNumberCheck = false;
    int compNumber; 
    bool winCheck;
     

    std::random_device rd;  // Non-deterministic generator
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 5);

    cout << "Hello and welcome to the random number game!" << endl;
    Sleep(1500);
    cout << "Do you need to know the rules? If so type Y, otherwise type N" << endl;
    cin >> rngRuleChoice;

    if (rngRuleChoice == 'y' || rngRuleChoice == 'Y') {
        cout << "Okay " << global.username << ", here is how the game works:" << endl;
        Sleep(1000);
        cout << "You will select an amount to gamble." << endl;
        Sleep(1000);
        cout << "Then you will make a choice between the numbers 1 - 5. No decimal places." << endl;
        Sleep(1000);
        cout << "If you choose the right number, you will win double the amount you gambled." << endl;
    }

    while (!rngRestart) { // game logic start
        cout << "Okay, let's play!" << endl;
        cout << "You have $ " << global.balance << " in the bank" << endl; 
        Sleep(1500);
        cout << "Type in the amount you would like to bet" << endl;

        while (!rngBetBalanceCheck) { //bet check
            cin >> rngBet;
            if (rngBet <= global.balance) {
                cout << "Okay, your bet of $" << rngBet << " has been accepted!" << endl;
                global.winnings = rngBet * 2;
                rngBetBalanceCheck = true;
            } else {
                cout << "Incorrect, enter a bet less than your current balance of $" << global.balance << endl;
            }
        } // end of bet check

        // game logic here
        cout <<"Okay, generating the computers number" << endl;
        
        Sleep(1000);
        compNumber = dis(gen);
         cout <<"Computers number: " << compNumber << endl;
        Sleep(3000);
        cout <<"Ok " + global.username + " please enter a number between 1 and 5" << endl;
        cin >> playerNumber;
        while(!playerNumberCheck){ // user number check
            if(playerNumber >=1 &&  playerNumber <=5){
                cout <<"Number accepted! Your number is " + playerNumber << endl;
                playerNumberCheck = true;
            }
            else{
                cout <<"Your number was not between 1 and 5. Please try again" << endl;
                 cin >> playerNumber;
            }
        }
        cout <<"Okay, checking for winner" << endl;
        Sleep(2000);
        if (playerNumber == compNumber) {
             global.winnings = rngBet * 2; // Calculate winnings
             cout << "You won! $" << global.winnings << " is being added to your account" << endl;
            global.balance += global.winnings;
         if (!updateBalance(hDbc, global)) {
             cerr << "Failed to update balance in the database." << endl;
             }  
            } else {
             cout << "Sorry, you did not win" << endl;
            global.balance -= rngBet; // Deduct the bet amount
            if (!updateBalance(hDbc, global)) {
           cerr << "Failed to update balance in the database." << endl;
                }           
}
    
            
        cout << "Would you like to play again? Type y for yes or n for no: ";
        cin >> restartChoice;
        if (restartChoice == 'y' || restartChoice == 'Y') {
            cout << "Okay, restarting the game" << endl;
            rngBetBalanceCheck = false; // Reset the balance check for the new game
        } else if (restartChoice == 'n' || restartChoice == 'N') {
            cout << "Okay, bringing you back to the main menu" << endl;
            return true;
        }
    }

    return true; // Indicates the function completed successfully
}

bool accountManagment(SQLHDBC hDbc, globalData &global){
    //account management
    cout <<"in account management" << endl;
    return true;
}

bool gameMenu(SQLHDBC hDbc, globalData &global) {
    int gameChoice;
    bool gameChoiceChk = false;

    while (!gameChoiceChk) {
        cout << "Okay, " << global.username << ", what game would you like to play?" << endl;
        cout << "Type 1 for random number generator game or type 9 to exit" << endl;
        cin >> gameChoice;

        if (gameChoice == 1) {
            cout << "Okay, great! Starting up Random Number Game!" << endl;
            Sleep(2500);
            if (!randomNumberGame(hDbc, global)) {
                cerr << "Game menu has failed to load." << endl;
            }
        } else if (gameChoice == 9) {
            Sleep(1000);
            cout << "Okay. Exiting game. Goodbye" << endl;
            gameChoiceChk = true;
            global.exitCheck = true;
            global.loginCheck = false; // Ensure this is set to exit the inner loop
        } else {
            cout << "Invalid choice. Please enter 1 to play the game or 9 to exit." << endl;
        }
    }

    return true;
}

bool mainMenu(SQLHDBC hDbc, globalData &global){
    int mainMenuChoice;
    bool mainCheck = false;

    cout <<"Hello " << global.username << "! What would you like to do today?" << endl;
    cout <<"Type 1 for the game menu" << endl << "Type 2 for the account management menu" << endl <<"Type 3 to log out" << endl;
    cin >> mainMenuChoice;
    if(mainMenuChoice == 1){//game menu
        if (!gameMenu(hDbc, global)) {
                cerr << "Game menu has failed to load." << endl;
            }
    }
    if(mainMenuChoice == 2){ // account management
           if (!accountManagment(hDbc, global)) {
                cerr << "Game menu has failed to load." << endl;
            }
    } 
    if(mainMenuChoice == 3){
        if (!logout(hDbc, global)) {
                cerr << "Game menu has failed to load." << endl;
            }
    }
return true;
}

