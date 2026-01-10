This is the **Code Walkthrough - Part 1**.

I will explain the **Header Files** and the **Utility Functions**. These are the tools and helpers the rest of the program relies on.

### 1. The Setup (Headers)

These lines tell the computer which pre-made code libraries we need to use.

```cpp
#include <iostream>   // Allows input/output (cin, cout)
#include <fstream>    // Allows File handling (reading/writing text files)
#include <sstream>    // Allows String Streams (treating strings like files)
#include <vector>     // A dynamic array (list) that can grow in size
#include <string>     // Allows using text (std::string) instead of character arrays
#include <iomanip>    // Helps format output (like printing exactly 2 decimal places)
#include <ctime>      // Used to get the current date and time
#include <algorithm>  // Helpers like finding items in a list
#include <map>        // A dictionary structure (Key -> Value pairs)
#include <cmath>      // Math functions (like absolute value 'abs')
#include <sys/stat.h> // Used to create folders/directories

using namespace std;  // Saves us from typing "std::" before every command

```

### 2. The Utilities (`namespace Utils`)

We wrap these functions in a `namespace Utils` so they don't get confused with other functions. Think of it as a labeled box for "Helper Tools."

#### `getCurrentDateTime()`

Gets the current time from your computer clock and turns it into a readable string.

```cpp
    string getCurrentDateTime() {
        time_t now = time(0); // Get current timestamp
        char buffer[80];      // Create a temporary storage for text
        // Format it nicely: Year-Month-Day Hour:Minute:Second
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buffer); // Send it back as a string
    }

```

#### `clearScreen()`

Wipes the terminal clean so the UI looks fresh.

```cpp
    void clearScreen() {
        #ifdef _WIN32      // If the computer is Windows...
            system("cls"); // ...run the Windows clear command
        #else              // If it's Mac or Linux...
            system("clear"); // ...run the Unix clear command
        #endif
    }

```

#### `isValidEmail()` & `isValidPhone()`

Simple checks to see if user input looks real.

```cpp
    // Check if email has an '@' AND a '.' inside it
    bool isValidEmail(const string& email) {
        return email.find('@') != string::npos && email.find('.') != string::npos;
    }

    // Check if phone number is not empty, has 10+ chars, and ONLY contains numbers
    bool isValidPhone(const string& phone) {
        if (phone.empty() || phone.length() < 10) return false;
        for (char c : phone) {      // Loop through every character
            if (!isdigit(c)) return false; // If any char is NOT a digit, fail.
        }
        return true;
    }

```

#### `formatCurrency()`

Takes a raw number like `10.5` and turns it into `$10.50`.

```cpp
    string formatCurrency(double amount) {
        stringstream ss;
        // 'fixed' means use standard notation (not scientific)
        // 'setprecision(2)' means strictly 2 decimal places
        ss << fixed << setprecision(2) << "$" << amount;
        return ss.str();
    }

```

#### `split()` (Very Important)

This acts like a pair of scissors. It cuts a long string into pieces whenever it sees a specific character (delimiter).

* **Input:** `"Alice|alice@test.com|12345"` and delimiter `|`
* **Output:** A list `["Alice", "alice@test.com", "12345"]`

```cpp
    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens; // The list of pieces we will return
        stringstream ss(str);  // Turn string into a stream
        string token;
        // Loop: Keep reading until the delimiter, store in 'token'
        while (getline(ss, token, delimiter)) {
            tokens.push_back(token); // Add the piece to our list
        }
        return tokens;
    }

```

#### `createDirectory()`

Creates the `data` folder if it doesn't exist so the program has a place to save files.

```cpp
    void createDirectory(const string& path) {
        #ifdef _WIN32
            _mkdir(path.c_str()); // Windows command to make folder
        #else
            mkdir(path.c_str(), 0777); // Linux/Mac command to make folder
        #endif
    }

```

---

This is **Code Walkthrough - Part 2**.

I will explain the **Data Structures**. These are the "Blueprints" that define what a "User" and an "Expense" actually are in the computer's memory.

### 3. The Enum (The Dropdown Menu)

An `enum` (Enumeration) is a custom variable type that restricts values to a specific list. It prevents spelling mistakes (like typing "equal" vs "EQUAL").

```cpp
enum class SplitMethod {
    EQUAL,      // Value 0
    EXACT,      // Value 1
    PERCENTAGE  // Value 2
};

```

**Helpers:** Since we can't save an `enum` directly to a text file, we need functions to convert it to text and back.

* `splitMethodToString`: Converts `SplitMethod::EQUAL` → `"EQUAL"`.
* `stringToSplitMethod`: Converts `"EQUAL"` → `SplitMethod::EQUAL`.

### 4. The User Class

This class represents one person using the app.

#### Variables (Private)

These are hidden inside the class (Encapsulation).

```cpp
class User {
private:
    int id;             // Unique ID (1, 2, 3...)
    string name;        // "Alice"
    string email;       // "alice@test.com"
    string phone;       // "1234567890"
    string password;    // "secret123"

```

#### Serialization (Packing Data)

This is critical for saving to a file. It takes all the separate variables and glues them into **one long string** separated by pipes `|`.

```cpp
    string serialize() const {
        // Returns: "1|Alice|alice@test.com|1234567890|secret123"
        return to_string(id) + "|" + name + "|" + email + "|" + phone + "|" + password;
    }

```

#### Deserialization (Unpacking Data)

The reverse process. It takes a line from the text file and rebuilds a User object.

```cpp
    static User deserialize(const string& data) {
        // Use our Utils::split tool to cut the string at every '|'
        vector<string> parts = Utils::split(data, '|');
        
        // If we found 5 pieces (ID, Name, Email, Phone, Pass), build the User
        if (parts.size() >= 5) {
            return User(stoi(parts[0]), parts[1], parts[2], parts[3], parts[4]);
        }
        return User(); // Return empty user if data was bad
    }

```

### 5. The Expense Participant Class

This is a small helper class. An expense isn't just about the payer; it's about the people involved.

* `userId`: Who is this person?
* `share`: How much do they specifically owe for this bill?

It also has `serialize()` (format: `UserID:Amount`) and `deserialize()` methods, just like the User class.

### 6. The Expense Class

This is the main event. It holds the bill details.

#### Composition (Important Concept)

Notice this line inside the class:

```cpp
    vector<ExpenseParticipant> participants;

```

This is called **Composition**. An `Expense` *has-a* list of `participants`.

#### Serialization (Complex)

Saving an expense is harder because it contains a list (vector) inside it.

1. First, we save the main details (ID, Description, Amount, etc.) separated by pipes `|`.
2. Then, we loop through the `participants` vector.
3. We turn each participant into a string and join them with commas `,`.

**Format in file:**
`ID | Description | Amount | Method | PayerID | Date | Part1:Share,Part2:Share`

```cpp
    string serialize() const {
        stringstream ss;
        // 1. Add basic info
        ss << id << "|" << description << "|" << ... << createdBy << "|" << createdAt << "|";
        
        // 2. Loop through participants and add them to the end
        for (size_t i = 0; i < participants.size(); i++) {
            ss << participants[i].serialize(); // "101:50.00"
            if (i < participants.size() - 1) ss << ","; // Add comma between them
        }
        return ss.str();
    }

```

---

This is **Code Walkthrough - Part 3**.

I will explain the **ExpenseManager Class**. This is the "Brain" of the application. It controls the data, handles the logic, and saves everything to files.

### 7. The Expense Manager Class

This class manages the entire application state.

#### The Memory (Private Variables)

It keeps the data in **Vectors** (lists) while the program is running.

```cpp
class ExpenseManager {
private: 
    vector<User> users;       // List of all registered users
    vector<Expense> expenses; // List of all expenses ever added
    User* currentUser;        // Pointer to the user currently logged in (or nullptr if nobody)
    
    // File paths where we store data
    const string DATA_DIR = "data";
    const string USERS_FILE = "data/users.txt";
    const string EXPENSES_FILE = "data/expenses.txt";

```

#### Data Persistence (`loadData` & `saveData`)

These two functions make sure data survives when you restart the app.

* **`loadData()`**: Runs automatically when the app starts. It opens the text files, reads them line-by-line, converts the text back into Objects (using `deserialize`), and fills the vectors.
* **`saveData()`**: Runs whenever we add new info. It wipes the text files and re-writes everything from the vectors into the files (using `serialize`).

#### User Logic (`registerUser` & `login`)

* **`registerUser`**:
1. Checks if the email is valid.
2. Checks if the phone is valid.
3. Checks if the email is already taken.
4. If all good, creates a `User` object, adds it to the vector, and calls `saveData()`.


* **`login`**: Loops through the `users` vector. If it finds a match for *both* email and password, it sets `currentUser = &user`. This "logs them in".

#### The Core Logic: `addExpense` (The Big Function)

This is the most complex function. It handles the math.

**Step 1: Validation**
Before doing any math, it checks for errors:

```cpp
    // 1. Is user logged in?
    if (currentUser == nullptr) return false;
    
    // 2. Is amount positive?
    if (amount <= 0) return false;

    // 3. Do all participants actually exist?
    // (Loops through users vector to verify IDs)

```

**Step 2: The Logic (Switch Case)**
It changes behavior based on the `SplitMethod`.

* **EQUAL Split:**
Simple division. `Amount ÷ Count`.
```cpp
double shareAmount = amount / participantIds.size();
// Everyone gets assigned this calculated share

```


* **EXACT Split:**
The user provides specific amounts. We must verify the total.
```cpp
// Sum up all the individual shares
double total = 0;
for (double share : shares) total += share;

// Check if they match the total bill (allowing for tiny 0.01 rounding errors)
if (abs(total - amount) > 0.01) {
    // Error! The numbers don't add up.
    return false;
}

```


* **PERCENTAGE Split:**
The user provides percentages (e.g., 50, 25, 25).
```cpp
// Sum up the percentages
double totalPercentage = 0;
for (double p : shares) totalPercentage += p;

// Check if they equal 100%
if (abs(totalPercentage - 100.0) > 0.01) {
    // Error! Percentages must equal 100.
    return false;
}

// Math: Convert % to $
// Share = TotalAmount * (Percent / 100)

```



#### The Balance Logic (`displayBalance`)

This function figures out the "Net Debt".

1. It creates a `map<int, double> balance`.
* **Key:** User ID
* **Value:** Amount (Positive = They owe YOU, Negative = You owe THEM).


2. It loops through **every expense** in history.
3. **The Rule:**
* If **YOU paid** and **User X** was a participant: Add X's share to the map (They owe you).
* If **User X paid** and **YOU** were a participant: Subtract your share from the map (You owe them).


4. Finally, it prints the map in a readable format.

---

This is **Code Walkthrough - Part 4**.

I will explain the **Main Function** and the **User Interface**. This is the entry point of the program—where everything starts execution.

### 8. The Main Function (`main`)

This is the conductor of the orchestra. It controls the flow of the application.

#### The Loop (`while(running)`)

We wrap the entire program in a `while (running)` loop.

* **Why?** CLI apps usually run once and close. We want this app to keep running (showing menus) until the user specifically chooses "Exit".

#### Two States: Logged In vs. Logged Out

Inside the loop, we check:

```cpp
    User* currentUser = manager.getCurrentUser();
    
    if (currentUser == nullptr) {
        // Show PUBLIC MENU (Register, Login)
    } else {
        // Show PRIVATE MENU (Add Expense, Balance, etc.)
    }

```

This simple `if/else` creates a secure session feeling. You cannot access the "Add Expense" features unless `currentUser` is set (which only happens after a successful login).

#### The Switch Case (Menu Navigation)

We use `cin >> choice` to get a number from the user, and then a `switch` statement to decide which function to call.

* **Case 1 (Register):** Calls `handleRegister()`
* **Case 2 (Login):** Calls `handleLogin()`
* **Default:** Handles invalid inputs (like typing "9" when there are only 4 options).

### 9. Input Buffering (The "Enter Key" Bug)

You will see this line used often:

```cpp
    cin.ignore();
    getline(cin, name);

```

**Why?**

* When you type a number (like "1") and hit **Enter**, `cin >> choice` reads the "1" but leaves the **Enter key (newline)** sitting in the computer's memory.
* The next time you try to read a string (`getline`), it sees that leftover Enter key, thinks you typed an empty line, and skips the input!
* `cin.ignore()` fixes this by "eating" that leftover character.

---

## 💻 How to Run This Project

Here are the exact commands she needs to type in her terminal to make this code come to life.

### Step 1: Create the file

1. Create a new folder named `ExpenseProject`.
2. Inside it, create a file named `expense_app.cpp`.
3. Paste the **entire code** I provided into that file.

### Step 2: Compile (Turn code into an app)

Open the terminal (Command Prompt) in that folder and type:

**If she is on Windows:**

```bash
g++ -std=c++17 expense_app.cpp -o expense_app.exe

```

**If she is on Mac or Linux:**

```bash
g++ -std=c++17 expense_app.cpp -o expense_app

```

*(Note: If `g++` is not recognized, she needs to install MinGW for Windows or Xcode for Mac. Most freshers have this installed already.)*

### Step 3: Run

**Windows:**

```bash
expense_app.exe

```

**Mac/Linux:**

```bash
./expense_app

```

---

## 🔮 Future Improvements (To mention in the interview)

If the interviewer asks, "What would you add next?", she should say:

1. **"I would add Groups."**
* *Scenario:* Right now, everyone sees everyone. I would create 'Groups' (like 'Vegas Trip' or 'Housemates') so expenses are isolated to specific people.


2. **"I would optimize the Debts."**
* *Scenario:* If A owes B $10, and B owes C $10, the system should tell A to pay C directly. This is called 'Debt Simplification' (using a Graph algorithm).


3. **"I would add a Database."**
* *Scenario:* Text files are okay for small projects, but for a real app, I would replace `fstream` with SQLite or MySQL to handle millions of users.



---

**That is the complete breakdown.**

1. **Concepts:** OOP, Vectors, Maps.
2. **Logic:** Split algorithms (Equal/Exact/Percentage).
3. **Persistence:** File Handling (Read/Write).

