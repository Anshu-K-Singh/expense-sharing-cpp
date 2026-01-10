/*
===============================================================================
    EXPENSE SHARING APPLICATION - C++ CLI

    Features:
    - User registration and login
    - Three split methods:  EQUAL, EXACT, PERCENTAGE
    - Balance calculation
    - File-based data persistence
    - CSV export
    
    Compile: g++ -std=c++17 expense_app.cpp -o expense_app
    Run: ./expense_app
===============================================================================
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <map>
#include <cmath>
#include <sys/stat.h>

using namespace std;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

namespace Utils {
    // Get current date and time as string
    string getCurrentDateTime() {
        time_t now = time(0);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buffer);
    }

    // Clear console screen (cross-platform)
    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    // Pause and wait for user input
    void pauseScreen() {
        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
    }

    // Validate email format (basic check)
    bool isValidEmail(const string& email) {
        return email.find('@') != string::npos && email.find('.') != string::npos;
    }

    // Validate phone number (basic check - digits only)
    bool isValidPhone(const string& phone) {
        if (phone.empty() || phone.length() < 10) return false;
        for (char c : phone) {
            if (!isdigit(c)) return false;
        }
        return true;
    }

    // Format currency with 2 decimal places
    string formatCurrency(double amount) {
        stringstream ss;
        ss << fixed << setprecision(2) << "$" << amount;
        return ss.str();
    }

    // Split string by delimiter
    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        while (getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Trim whitespace from string
    string trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    // Create directory if it doesn't exist
    void createDirectory(const string& path) {
        #ifdef _WIN32
            _mkdir(path.c_str());
        #else
            mkdir(path.c_str(), 0777);
        #endif
    }
}

// ============================================================================
// ENUMS
// ============================================================================

enum class SplitMethod {
    EQUAL,
    EXACT,
    PERCENTAGE
};

string splitMethodToString(SplitMethod method) {
    switch(method) {
        case SplitMethod::EQUAL:  return "EQUAL";
        case SplitMethod::EXACT:  return "EXACT";
        case SplitMethod::PERCENTAGE:  return "PERCENTAGE";
        default: return "UNKNOWN";
    }
}

SplitMethod stringToSplitMethod(const string& str) {
    if (str == "EQUAL") return SplitMethod::EQUAL;
    if (str == "EXACT") return SplitMethod:: EXACT;
    if (str == "PERCENTAGE") return SplitMethod::PERCENTAGE;
    return SplitMethod::EQUAL;
}

// ============================================================================
// USER CLASS
// ============================================================================

class User {
private:
    int id;
    string name;
    string email;
    string phone;
    string password;

public:
    // Constructors
    User() : id(0), name(""), email(""), phone(""), password("") {}
    
    User(int id, string name, string email, string phone, string password)
        : id(id), name(name), email(email), phone(phone), password(password) {}

    // Getters
    int getId() const { return id; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getPhone() const { return phone; }
    
    // Password verification
    bool verifyPassword(const string& pwd) const {
        return password == pwd;
    }

    // Display user information
    void display() const {
        cout << "ID: " << id << " | Name: " << name 
             << " | Email: " << email << " | Phone: " << phone << endl;
    }

    // Serialize to string for file storage
    string serialize() const {
        return to_string(id) + "|" + name + "|" + email + "|" + phone + "|" + password;
    }

    // Deserialize from string
    static User deserialize(const string& data) {
        vector<string> parts = Utils::split(data, '|');
        if (parts.size() >= 5) {
            return User(stoi(parts[0]), parts[1], parts[2], parts[3], parts[4]);
        }
        return User();
    }
};

// ============================================================================
// EXPENSE PARTICIPANT CLASS
// ============================================================================

class ExpenseParticipant {
private:
    int userId;
    double share;

public:
    ExpenseParticipant() : userId(0), share(0.0) {}
    
    ExpenseParticipant(int userId, double share)
        : userId(userId), share(share) {}

    // Getters
    int getUserId() const { return userId; }
    double getShare() const { return share; }

    // Serialization
    string serialize() const {
        stringstream ss;
        ss << userId << ":" << fixed << setprecision(2) << share;
        return ss. str();
    }

    static ExpenseParticipant deserialize(const string& data) {
        vector<string> parts = Utils::split(data, ':');
        if (parts.size() >= 2) {
            return ExpenseParticipant(stoi(parts[0]), stod(parts[1]));
        }
        return ExpenseParticipant();
    }
};

// ============================================================================
// EXPENSE CLASS
// ============================================================================

class Expense {
private:
    int id;
    string description;
    double amount;
    SplitMethod splitMethod;
    int createdBy;
    string createdAt;
    vector<ExpenseParticipant> participants;

public:
    // Constructors
    Expense() : id(0), description(""), amount(0.0), 
                splitMethod(SplitMethod::EQUAL), createdBy(0), createdAt("") {}
    
    Expense(int id, string desc, double amt, SplitMethod method, int creator)
        : id(id), description(desc), amount(amt), 
          splitMethod(method), createdBy(creator) {
        createdAt = Utils:: getCurrentDateTime();
    }

    // Getters
    int getId() const { return id; }
    string getDescription() const { return description; }
    double getAmount() const { return amount; }
    SplitMethod getSplitMethod() const { return splitMethod; }
    int getCreatedBy() const { return createdBy; }
    string getCreatedAt() const { return createdAt; }
    vector<ExpenseParticipant> getParticipants() const { return participants; }

    // Add participant
    void addParticipant(const ExpenseParticipant& participant) {
        participants.push_back(participant);
    }

    // Display expense details
    void display(const vector<User>& users) const {
        cout << "\n----------------------------------------" << endl;
        cout << "Expense ID: " << id << endl;
        cout << "Description: " << description << endl;
        cout << "Amount: " << Utils::formatCurrency(amount) << endl;
        cout << "Split Method: " << splitMethodToString(splitMethod) << endl;
        cout << "Created At: " << createdAt << endl;
        cout << "Participants:" << endl;
        
        for (const auto& p : participants) {
            // Find user name
            string userName = "Unknown";
            for (const auto& user : users) {
                if (user.getId() == p.getUserId()) {
                    userName = user.getName();
                    break;
                }
            }
            cout << "  - " << userName << " (ID:  " << p.getUserId() << "): " 
                 << Utils:: formatCurrency(p.getShare()) << endl;
        }
        cout << "----------------------------------------" << endl;
    }

    // Serialization
    string serialize() const {
        stringstream ss;
        ss << id << "|" << description << "|" << fixed << setprecision(2) << amount 
           << "|" << splitMethodToString(splitMethod) << "|" << createdBy 
           << "|" << createdAt << "|";
        
        // Serialize participants
        for (size_t i = 0; i < participants.size(); i++) {
            ss << participants[i]. serialize();
            if (i < participants.size() - 1) ss << ",";
        }
        
        return ss.str();
    }

    static Expense deserialize(const string& data) {
        vector<string> parts = Utils::split(data, '|');
        if (parts. size() >= 7) {
            Expense exp;
            exp.id = stoi(parts[0]);
            exp.description = parts[1];
            exp.amount = stod(parts[2]);
            exp.splitMethod = stringToSplitMethod(parts[3]);
            exp.createdBy = stoi(parts[4]);
            exp.createdAt = parts[5];
            
            // Deserialize participants
            if (! parts[6].empty()) {
                vector<string> participantStrings = Utils::split(parts[6], ',');
                for (const auto& pStr : participantStrings) {
                    exp.participants.push_back(ExpenseParticipant::deserialize(pStr));
                }
            }
            
            return exp;
        }
        return Expense();
    }
};

// ============================================================================
// EXPENSE MANAGER CLASS
// ============================================================================

class ExpenseManager {
private: 
    vector<User> users;
    vector<Expense> expenses;
    User* currentUser;
    int nextUserId;
    int nextExpenseId;
    
    const string DATA_DIR = "data";
    const string USERS_FILE = "data/users.txt";
    const string EXPENSES_FILE = "data/expenses.txt";

public:
    ExpenseManager() : currentUser(nullptr), nextUserId(1), nextExpenseId(1) {
        loadData();
    }

    ~ExpenseManager() {
        saveData();
    }

    // ========================================================================
    // USER OPERATIONS
    // ========================================================================

    bool registerUser(string name, string email, string phone, string password) {
        // Validate email
        if (!Utils::isValidEmail(email)) {
            cout << "Error: Invalid email format!" << endl;
            return false;
        }

        // Validate phone
        if (!Utils:: isValidPhone(phone)) {
            cout << "Error: Invalid phone number!  (must be 10+ digits)" << endl;
            return false;
        }

        // Check if email already exists
        for (const auto& user : users) {
            if (user.getEmail() == email) {
                cout << "Error: Email already registered!" << endl;
                return false;
            }
        }

        // Create new user
        User newUser(nextUserId++, name, email, phone, password);
        users.push_back(newUser);
        saveData();
        
        cout << "\n✓ User registered successfully!" << endl;
        return true;
    }

    bool login(string email, string password) {
        for (auto& user : users) {
            if (user.getEmail() == email && user.verifyPassword(password)) {
                currentUser = &user;
                cout << "\n✓ Login successful!  Welcome, " << user.getName() << "!" << endl;
                return true;
            }
        }
        cout << "\nError: Invalid email or password!" << endl;
        return false;
    }

    void logout() {
        if (currentUser != nullptr) {
            cout << "\n✓ Logged out successfully!" << endl;
            currentUser = nullptr;
        }
    }

    void displayAllUsers() const {
        if (users.empty()) {
            cout << "\nNo users registered yet." << endl;
            return;
        }

        cout << "\n========================================" << endl;
        cout << "         REGISTERED USERS" << endl;
        cout << "========================================" << endl;
        for (const auto& user : users) {
            user.display();
        }
        cout << "========================================" << endl;
    }

    User* getCurrentUser() {
        return currentUser;
    }

    // ========================================================================
    // EXPENSE OPERATIONS
    // ========================================================================

    bool addExpense(string description, double amount, SplitMethod method, 
                   vector<int> participantIds, vector<double> shares = {}) {
        
        if (currentUser == nullptr) {
            cout << "Error: Please login first!" << endl;
            return false;
        }

        if (amount <= 0) {
            cout << "Error: Amount must be greater than 0!" << endl;
            return false;
        }

        // Ensure creator is in participants
        bool creatorIncluded = false;
        for (int id : participantIds) {
            if (id == currentUser->getId()) {
                creatorIncluded = true;
                break;
            }
        }
        if (!creatorIncluded) {
            participantIds.push_back(currentUser->getId());
        }

        // Validate participants exist
        for (int id : participantIds) {
            bool found = false;
            for (const auto& user : users) {
                if (user.getId() == id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                cout << "Error:  User with ID " << id << " not found!" << endl;
                return false;
            }
        }

        // Create expense
        Expense newExpense(nextExpenseId++, description, amount, method, currentUser->getId());

        // Calculate shares based on split method
        if (method == SplitMethod:: EQUAL) {
            double shareAmount = amount / participantIds.size();
            for (int userId : participantIds) {
                newExpense.addParticipant(ExpenseParticipant(userId, shareAmount));
            }
        }
        else if (method == SplitMethod::EXACT) {
            if (shares.size() != participantIds.size()) {
                cout << "Error: Number of shares doesn't match participants!" << endl;
                return false;
            }

            double total = 0;
            for (double share : shares) {
                total += share;
            }

            if (abs(total - amount) > 0.01) {
                cout << "Error: Sum of shares (" << Utils::formatCurrency(total) 
                     << ") doesn't match total amount (" << Utils::formatCurrency(amount) << ")!" << endl;
                return false;
            }

            for (size_t i = 0; i < participantIds.size(); i++) {
                newExpense. addParticipant(ExpenseParticipant(participantIds[i], shares[i]));
            }
        }
        else if (method == SplitMethod::PERCENTAGE) {
            if (shares.size() != participantIds.size()) {
                cout << "Error: Number of percentages doesn't match participants!" << endl;
                return false;
            }

            double totalPercentage = 0;
            for (double percentage : shares) {
                totalPercentage += percentage;
            }

            if (abs(totalPercentage - 100.0) > 0.01) {
                cout << "Error:  Percentages must add up to 100%!  (Current: " 
                     << totalPercentage << "%)" << endl;
                return false;
            }

            for (size_t i = 0; i < participantIds.size(); i++) {
                double shareAmount = amount * (shares[i] / 100.0);
                newExpense. addParticipant(ExpenseParticipant(participantIds[i], shareAmount));
            }
        }

        expenses.push_back(newExpense);
        saveData();
        
        cout << "\n✓ Expense added successfully!  (ID: " << newExpense. getId() << ")" << endl;
        return true;
    }

    void displayUserExpenses() const {
        if (currentUser == nullptr) {
            cout << "Error: Please login first!" << endl;
            return;
        }

        bool found = false;
        cout << "\n========================================" << endl;
        cout << "      YOUR EXPENSES" << endl;
        cout << "========================================" << endl;

        for (const auto& expense :  expenses) {
            // Check if current user is a participant
            for (const auto& participant : expense.getParticipants()) {
                if (participant.getUserId() == currentUser->getId()) {
                    expense.display(users);
                    cout << "Your share: " << Utils::formatCurrency(participant.getShare()) << endl;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            cout << "No expenses found for you." << endl;
        }
    }

    void displayAllExpenses() const {
        if (currentUser == nullptr) {
            cout << "Error: Please login first!" << endl;
            return;
        }

        if (expenses.empty()) {
            cout << "\nNo expenses recorded yet." << endl;
            return;
        }

        cout << "\n========================================" << endl;
        cout << "         ALL EXPENSES" << endl;
        cout << "========================================" << endl;

        for (const auto& expense : expenses) {
            expense.display(users);
        }
    }

    // ========================================================================
    // BALANCE OPERATIONS
    // ========================================================================

    void displayBalance() const {
        if (currentUser == nullptr) {
            cout << "Error: Please login first!" << endl;
            return;
        }

        // Calculate balance:  who owes whom
        map<int, double> balance; // positive means they owe current user, negative means current user owes them

        for (const auto& expense : expenses) {
            int payer = expense.getCreatedBy();
            
            for (const auto& participant : expense. getParticipants()) {
                int userId = participant.getUserId();
                double share = participant.getShare();

                // If current user is the payer
                if (payer == currentUser->getId() && userId != currentUser->getId()) {
                    balance[userId] += share; // Others owe current user
                }
                // If current user is a participant but not the payer
                else if (userId == currentUser->getId() && payer != currentUser->getId()) {
                    balance[payer] -= share; // Current user owes the payer
                }
            }
        }

        cout << "\n========================================" << endl;
        cout << "         YOUR BALANCE" << endl;
        cout << "========================================" << endl;

        if (balance.empty()) {
            cout << "No balances to show." << endl;
            return;
        }

        bool hasBalance = false;
        for (const auto& [userId, amount] : balance) {
            if (abs(amount) > 0.01) {
                hasBalance = true;
                string userName = "Unknown";
                for (const auto& user : users) {
                    if (user.getId() == userId) {
                        userName = user.getName();
                        break;
                    }
                }

                if (amount > 0) {
                    cout << userName << " owes you: " << Utils::formatCurrency(amount) << endl;
                } else {
                    cout << "You owe " << userName << ": " << Utils::formatCurrency(-amount) << endl;
                }
            }
        }

        if (!hasBalance) {
            cout << "All settled up!" << endl;
        }
        cout << "========================================" << endl;
    }

    void exportBalanceToCSV(const string& filename) const {
        if (currentUser == nullptr) {
            cout << "Error:  Please login first!" << endl;
            return;
        }

        ofstream file(filename);
        if (!file.is_open()) {
            cout << "Error:  Could not create file!" << endl;
            return;
        }

        // Write CSV header
        file << "Expense ID,Description,Total Amount,Payer,Payer Name,User ID,User Name,Share,Created At\n";

        // Write expense data
        for (const auto& expense : expenses) {
            bool isRelevant = false;
            
            // Check if current user is involved
            for (const auto& participant : expense.getParticipants()) {
                if (participant. getUserId() == currentUser->getId()) {
                    isRelevant = true;
                    break;
                }
            }

            if (expense.getCreatedBy() == currentUser->getId()) {
                isRelevant = true;
            }

            if (isRelevant) {
                string payerName = "Unknown";
                for (const auto& user : users) {
                    if (user.getId() == expense.getCreatedBy()) {
                        payerName = user.getName();
                        break;
                    }
                }

                for (const auto& participant : expense. getParticipants()) {
                    string userName = "Unknown";
                    for (const auto& user : users) {
                        if (user.getId() == participant.getUserId()) {
                            userName = user.getName();
                            break;
                        }
                    }

                    file << expense.getId() << ","
                         << expense.getDescription() << ","
                         << fixed << setprecision(2) << expense.getAmount() << ","
                         << expense.getCreatedBy() << ","
                         << payerName << ","
                         << participant.getUserId() << ","
                         << userName << ","
                         << participant.getShare() << ","
                         << expense.getCreatedAt() << "\n";
                }
            }
        }

        file.close();
        cout << "\n✓ Balance sheet exported to " << filename << " successfully!" << endl;
    }

    // ========================================================================
    // DATA PERSISTENCE
    // ========================================================================

    void loadData() {
        Utils::createDirectory(DATA_DIR);

        // Load users
        ifstream usersFile(USERS_FILE);
        if (usersFile.is_open()) {
            string line;
            while (getline(usersFile, line)) {
                if (! line.empty()) {
                    User user = User::deserialize(line);
                    if (user.getId() > 0) {
                        users.push_back(user);
                        if (user.getId() >= nextUserId) {
                            nextUserId = user.getId() + 1;
                        }
                    }
                }
            }
            usersFile. close();
        }

        // Load expenses
        ifstream expensesFile(EXPENSES_FILE);
        if (expensesFile.is_open()) {
            string line;
            while (getline(expensesFile, line)) {
                if (!line. empty()) {
                    Expense expense = Expense::deserialize(line);
                    if (expense.getId() > 0) {
                        expenses.push_back(expense);
                        if (expense.getId() >= nextExpenseId) {
                            nextExpenseId = expense.getId() + 1;
                        }
                    }
                }
            }
            expensesFile. close();
        }
    }

    void saveData() {
        Utils::createDirectory(DATA_DIR);

        // Save users
        ofstream usersFile(USERS_FILE);
        if (usersFile.is_open()) {
            for (const auto& user : users) {
                usersFile << user.serialize() << "\n";
            }
            usersFile.close();
        }

        // Save expenses
        ofstream expensesFile(EXPENSES_FILE);
        if (expensesFile.is_open()) {
            for (const auto& expense : expenses) {
                expensesFile << expense.serialize() << "\n";
            }
            expensesFile.close();
        }
    }

    // Get user by ID (helper function)
    User* getUserById(int id) {
        for (auto& user : users) {
            if (user.getId() == id) {
                return &user;
            }
        }
        return nullptr;
    }
};

// ============================================================================
// MAIN MENU FUNCTIONS
// ============================================================================

void showMainMenu() {
    cout << "\n========================================" << endl;
    cout << "   EXPENSE SHARING APPLICATION" << endl;
    cout << "========================================" << endl;
    cout << "1. Register" << endl;
    cout << "2. Login" << endl;
    cout << "3. View All Users" << endl;
    cout << "4. Exit" << endl;
    cout << "========================================" << endl;
    cout << "Enter your choice: ";
}

void showUserMenu(const string& userName) {
    cout << "\n========================================" << endl;
    cout << "   WELCOME, " << userName << "!" << endl;
    cout << "========================================" << endl;
    cout << "1. Add Expense" << endl;
    cout << "2. View My Expenses" << endl;
    cout << "3. View All Expenses" << endl;
    cout << "4. View Balance" << endl;
    cout << "5. Export Balance to CSV" << endl;
    cout << "6. Logout" << endl;
    cout << "7. Exit" << endl;
    cout << "========================================" << endl;
    cout << "Enter your choice: ";
}

void handleRegister(ExpenseManager& manager) {
    Utils::clearScreen();
    cout << "\n========== USER REGISTRATION ==========" << endl;
    
    string name, email, phone, password;
    
    cout << "Enter name: ";
    cin. ignore();
    getline(cin, name);
    
    cout << "Enter email: ";
    getline(cin, email);
    
    cout << "Enter phone:  ";
    getline(cin, phone);
    
    cout << "Enter password: ";
    getline(cin, password);
    
    manager.registerUser(name, email, phone, password);
    Utils::pauseScreen();
}

void handleLogin(ExpenseManager& manager) {
    Utils::clearScreen();
    cout << "\n============ USER LOGIN ============" << endl;
    
    string email, password;
    
    cout << "Enter email: ";
    cin.ignore();
    getline(cin, email);
    
    cout << "Enter password: ";
    getline(cin, password);
    
    manager.login(email, password);
    Utils::pauseScreen();
}

void handleAddExpense(ExpenseManager& manager) {
    Utils::clearScreen();
    cout << "\n========== ADD EXPENSE ==========" << endl;
    
    string description;
    double amount;
    int methodChoice;
    
    cout << "Enter description: ";
    cin.ignore();
    getline(cin, description);
    
    cout << "Enter amount: $";
    cin >> amount;
    
    cout << "\nSplit Method:" << endl;
    cout << "1. EQUAL - Split equally among all participants" << endl;
    cout << "2. EXACT - Specify exact amount for each participant" << endl;
    cout << "3. PERCENTAGE - Split by percentage" << endl;
    cout << "Enter choice (1-3): ";
    cin >> methodChoice;
    
    SplitMethod method;
    switch(methodChoice) {
        case 1: method = SplitMethod::EQUAL; break;
        case 2: method = SplitMethod::EXACT; break;
        case 3: method = SplitMethod::PERCENTAGE; break;
        default: 
            cout << "Invalid choice!  Defaulting to EQUAL." << endl;
            method = SplitMethod::EQUAL;
    }
    
    int numParticipants;
    cout << "Enter number of participants: ";
    cin >> numParticipants;
    
    vector<int> participantIds;
    vector<double> shares;
    
    cout << "\nEnter participant user IDs:" << endl;
    for (int i = 0; i < numParticipants; i++) {
        int userId;
        cout << "Participant " << (i + 1) << " ID: ";
        cin >> userId;
        participantIds.push_back(userId);
    }
    
    if (method == SplitMethod:: EXACT) {
        cout << "\nEnter exact amounts for each participant:" << endl;
        for (int i = 0; i < numParticipants; i++) {
            double share;
            cout << "Amount for participant " << participantIds[i] << ":  $";
            cin >> share;
            shares.push_back(share);
        }
    }
    else if (method == SplitMethod::PERCENTAGE) {
        cout << "\nEnter percentage for each participant (must total 100%):" << endl;
        for (int i = 0; i < numParticipants; i++) {
            double percentage;
            cout << "Percentage for participant " << participantIds[i] << ":  ";
            cin >> percentage;
            shares.push_back(percentage);
        }
    }
    
    manager.addExpense(description, amount, method, participantIds, shares);
    Utils::pauseScreen();
}

void handleExportCSV(ExpenseManager& manager) {
    Utils::clearScreen();
    cout << "\n========== EXPORT TO CSV ==========" << endl;
    
    string filename;
    cout << "Enter filename (e.g., balance. csv): ";
    cin.ignore();
    getline(cin, filename);
    
    manager.exportBalanceToCSV(filename);
    Utils::pauseScreen();
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    ExpenseManager manager;
    int choice;
    bool running = true;

    cout << "\n╔════════════════════════════════════════╗" << endl;
    cout << "║  EXPENSE SHARING APPLICATION - C++     ║" << endl;
    cout << "║  Perfect for Beginner Interviews!       ║" << endl;
    cout << "╚════════════════════════════════════════╝" << endl;

    while (running) {
        User* currentUser = manager.getCurrentUser();
        
        if (currentUser == nullptr) {
            // Main menu (not logged in)
            showMainMenu();
            cin >> choice;
            
            switch(choice) {
                case 1: 
                    handleRegister(manager);
                    break;
                case 2:
                    handleLogin(manager);
                    break;
                case 3:
                    Utils::clearScreen();
                    manager.displayAllUsers();
                    Utils::pauseScreen();
                    break;
                case 4:
                    cout << "\nThank you for using Expense Sharing App!  Goodbye!" << endl;
                    running = false;
                    break;
                default:
                    cout << "\nInvalid choice! Please try again." << endl;
                    Utils::pauseScreen();
            }
        }
        else {
            // User menu (logged in)
            showUserMenu(currentUser->getName());
            cin >> choice;
            
            switch(choice) {
                case 1:
                    handleAddExpense(manager);
                    break;
                case 2:
                    Utils::clearScreen();
                    manager.displayUserExpenses();
                    Utils::pauseScreen();
                    break;
                case 3:
                    Utils::clearScreen();
                    manager.displayAllExpenses();
                    Utils::pauseScreen();
                    break;
                case 4:
                    Utils::clearScreen();
                    manager.displayBalance();
                    Utils::pauseScreen();
                    break;
                case 5:
                    handleExportCSV(manager);
                    break;
                case 6:
                    manager.logout();
                    Utils::pauseScreen();
                    break;
                case 7:
                    cout << "\nThank you for using Expense Sharing App! Goodbye!" << endl;
                    running = false;
                    break;
                default:
                    cout << "\nInvalid choice!  Please try again." << endl;
                    Utils::pauseScreen();
            }
        }
        
        if (running) {
            Utils::clearScreen();
        }
    }

    return 0;
}
