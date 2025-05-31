#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <limits>

using namespace std;

struct Book {
    int id;
    string title;
    string topic;
    int copies;
    int value;

    Book() {}
    Book(int i, string t, string tp, int c, int v)
        : id(i), title(t), topic(tp), copies(c), value(v) {}
};

struct User {
    int userId;
    string name;

    User() {}
    User(int id, string n) : userId(id), name(n) {}
};

class BookHash {
    static const int SIZE = 101;
    vector<Book> table[SIZE];

    int hashFunction(int id) {
        return id % SIZE;
    }

public:
    void insert(Book b) {
        int idx = hashFunction(b.id);
        table[idx].push_back(b);
    }

    Book* search(int id) {
        int idx = hashFunction(id);
        for (size_t i = 0; i < table[idx].size(); ++i) {
            if (table[idx][i].id == id)
                return &table[idx][i];
        }
        return NULL;
    }
};

struct TreeNode {
    Book book;
    TreeNode* left;
    TreeNode* right;

    TreeNode(Book b) : book(b), left(NULL), right(NULL) {}
};

class BookBST {
    TreeNode* root;

    TreeNode* insert(TreeNode* node, Book b) {
        if (!node) return new TreeNode(b);
        if (b.title < node->book.title)
            node->left = insert(node->left, b);
        else
            node->right = insert(node->right, b);
        return node;
    }

    void inorder(TreeNode* node) {
        if (!node) return;
        inorder(node->left);
        cout << "ID: " << node->book.id << ", Title: " << node->book.title << ", Topic: " << node->book.topic << ", Copies: " << node->book.copies << ", Value: " << node->book.value << endl;
        inorder(node->right);
    }

    void searchTopic(TreeNode* node, const string& topic, vector<Book>& res) {
        if (!node) return;
        searchTopic(node->left, topic, res);
        if (node->book.topic == topic)
            res.push_back(node->book);
        searchTopic(node->right, topic, res);
    }

public:
    BookBST() : root(NULL) {}

    void insert(Book b) {
        root = insert(root, b);
    }

    void display() {
        if (!root) {
            cout << "The library is currently empty.\n";
            return;
        }
        inorder(root);
    }

    vector<Book> getBooksByTopic(const string& topic) {
        vector<Book> res;
        searchTopic(root, topic, res);
        return res;
    }
};

class UserDatabase {
    map<int, User> users;

public:
    void addUser(User u) {
        if (users.find(u.userId) == users.end()) {
            users[u.userId] = u;
            cout << "User '" << u.name << "' (ID: " << u.userId << ") added successfully.\n";
        } else {
            cout << "Error: User with ID " << u.userId << " already exists.\n";
        }
    }

    User* getUser(int userId) {
        auto it = users.find(userId);
        if (it != users.end()) {
            return &(it->second);
        }
        return NULL;
    }

    void displayUsers() {
        if (users.empty()) {
            cout << "No users registered in the system.\n";
            return;
        }
        cout << "\n--- Registered Users ---\n";
        for (const auto& pair : users) {
            cout << "ID: " << pair.second.userId << ", Name: " << pair.second.name << endl;
        }
    }
};


struct Reservation {
    int userId;
    int bookId;
    bool vip;
    Reservation(int u, int b, bool v) : userId(u), bookId(b), vip(v) {}
};

class ReservationSystem {
public:
    struct CompareReservation {
        bool operator()(const Reservation& a, const Reservation& b) {
            if (a.vip != b.vip)
                return a.vip < b.vip;
            return a.userId > b.userId;
        }
    };

private:
    queue<Reservation> normalQueue;
    priority_queue<Reservation, vector<Reservation>, CompareReservation> vipQueue;

public:
    void addReservation(int userId, int bookId, bool vip, UserDatabase& userDb) {
        if (userDb.getUser(userId) == NULL) {
            cout << "Error: User ID " << userId << " not found. Please register the user first.\n";
            return;
        }
        if (vip) {
            vipQueue.push(Reservation(userId, bookId, vip));
            cout << "VIP reservation added for User " << userDb.getUser(userId)->name << " (ID: " << userId << ") for Book ID " << bookId << ".\n";
        } else {
            cout<<"SORRY!! Reservation is only for VIP members"<<".\n";
        }
    }

    void processReservations(BookHash& hash, BookBST& bst, UserDatabase& userDb) {
        if (vipQueue.empty() && normalQueue.empty()) {
            cout << "No reservations to process.\n";
            return;
        }

        cout << "\nProcessing reservations...\n";

        while (!vipQueue.empty()) {
            Reservation r = vipQueue.top();
            vipQueue.pop();

            ReservationSystem::borrowBook(r.userId, r.bookId, true, hash, bst, userDb);
        }

        while (!normalQueue.empty()) {
            Reservation r = normalQueue.front();
            normalQueue.pop();
            ReservationSystem::borrowBook(r.userId, r.bookId, false, hash, bst, userDb);
        }
        cout << "Finished processing reservations.\n";
    }

    static void borrowBook(int userId, int bookId, bool vip, BookHash& hash, BookBST& bst, UserDatabase& userDb) {
        Book* b = hash.search(bookId);
        User* u = userDb.getUser(userId);

        if (u == NULL) {
            cout << "Error: User ID " << userId << " not found. Cannot process borrow request.\n";
            return;
        }
        if (b == NULL) {
            cout << "Book ID " << bookId << " not found for user " << u->name << ".\n";
            return;
        }

        if (b->copies <= 1) {
            cout << "Cannot borrow book '" << b->title << "' (ID: " << b->id << ") for user " << u->name << ": Must keep at least one copy in library.\n";

            vector<Book> suggestions = bst.getBooksByTopic(b->topic);
            if (!suggestions.empty()) {
                cout << "Suggested books on topic '" << b->topic << "':\n";
                bool foundSuggestion = false;
                for (size_t i = 0; i < suggestions.size(); i++) {

                    if (suggestions[i].id != b->id && suggestions[i].copies > 1) {
                        cout << "- " << suggestions[i].title << " (ID " << suggestions[i].id << ", Copies: " << suggestions[i].copies << ")\n";
                        foundSuggestion = true;
                    }
                }
                if (!foundSuggestion) {
                    cout << "No other suitable books on this topic are available for borrowing.\n";
                }
            } else {
                cout << "No other books on this topic found in the library.\n";
            }
            return;
        }
        b->copies--;
        cout << "User " << u->name << (vip ? " (VIP)" : "") << " borrowed book: " << b->title << " (ID: " << b->id << "). Remaining copies: " << b->copies << endl;
    }

    static void returnBook(int userId, int bookId, BookHash& hash, UserDatabase& userDb) {
        Book* b = hash.search(bookId);
        User* u = userDb.getUser(userId);

        if (u == NULL) {
            cout << "Error: User ID " << userId << " not found. Cannot process return request.\n";
            return;
        }
        if (b == NULL) {
            cout << "Book ID " << bookId << " not found. Cannot return a non-existent book for user " << u->name << ".\n";
            return;
        }
        b->copies++;
        cout << "User " << u->name << " returned book: " << b->title << " (ID: " << b->id << "). New copies: " << b->copies << endl;
    }
};


vector<Book> knapsackSelectBooks(vector<Book>& books, int capacity) {
    int n = (int)books.size();
    if (n == 0 || capacity <= 0) {
        return {};
    }

    vector<vector<int>> dp(n+1, vector<int>(capacity+1, 0));

    for (int i=1; i<=n; ++i) {
        for (int w=1; w<=capacity; ++w) {
            if (books[i-1].copies > 0 && 1 <= w) {
                dp[i][w] = max(dp[i-1][w], books[i-1].value + dp[i-1][w-1]);
            } else {
                dp[i][w] = dp[i-1][w];
            }
        }
    }

    vector<Book> selected;
    int w = capacity;
    for (int i=n; i>0 && w>0; --i) {
        if (dp[i][w] != dp[i-1][w]) {
            selected.push_back(books[i-1]);
            w -= 1;
        }
    }

    reverse(selected.begin(), selected.end());
    return selected;
}

int getIntInput() {
    int value;
    while (!(cin >> value)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return value;
}

string getStringInput() {
    string value;
    getline(cin, value);
    return value;
}

void displayMenu() {
    cout << "\n--- Library Management System Menu ---\n";
    cout << "1. Add User\n";
    cout << "2. Display All Users\n";
    cout << "3. Add a New Book\n";
    cout << "4. Search Book by ID\n";
    cout << "5. Display All Books (by Title)\n";
    cout << "6. Get Books by Topic\n";
    cout << "7. Add Reservation\n";
    cout << "8. Reservation list\n";
    cout << "9. Borrow a Book (Directly)\n";
    cout << "10. Return a Book\n";
    cout << "11. Knapsack Select Books\n";
    cout << "12. Exit\n";
    cout << "Enter your choice: ";
}

int main() {
    BookHash bookHash;
    BookBST bookBST;
    ReservationSystem rs;
    UserDatabase userDb;
    vector<Book> allBooks;

    allBooks.push_back(Book(101, "The Great Gatsby", "Fiction", 5, 12));
    allBooks.push_back(Book(102, "Sapiens: A Brief History of Humankind", "History", 3, 15));
    allBooks.push_back(Book(103, "Clean Code", "Programming", 4, 18));

    cout << "Adding initial books...\n";
    for (const auto& book : allBooks) {
        bookHash.insert(book);
        bookBST.insert(book);
        cout << "Added: " << book.title << " (ID: " << book.id << ")\n";
    }
    cout << "Initial books added successfully!\n";

    cout << "\n";

    userDb.addUser(User(1, "Faria"));
    userDb.addUser(User(2, "Zearin"));
    userDb.addUser(User(3, "Lamia"));
    cout << "Initial users added successfully!\n";

    int choice;
    do {
        displayMenu();
        choice = getIntInput();

        switch (choice) {
            case 1: {
                cout << "\n--- Add New User ---\n";
                int userId;
                string userName;
                cout << "Enter User ID: ";
                userId = getIntInput();
                cout << "Enter User Name: ";
                userName = getStringInput();
                userDb.addUser(User(userId, userName));
                break;
            }
            case 2: {
                userDb.displayUsers();
                break;
            }
            case 3: {
                cout << "\n--- Add New Book ---\n";
                int id, copies, value;
                string title, topic;

                cout << "Enter Book ID: ";
                id = getIntInput();

                if (bookHash.search(id) != NULL) {
                    cout << "Error: Book with ID " << id << " already exists.Please use a unique ID.\n";
                    break;
                }

                cout << "Enter Title: ";
                title = getStringInput();
                cout << "Enter Topic: ";
                topic = getStringInput();
                cout << "Enter Number of Copies: ";
                copies = getIntInput();
                cout << "Enter Value (for knapsack): ";
                value = getIntInput();

                Book newBook(id, title, topic, copies, value);
                bookHash.insert(newBook);
                bookBST.insert(newBook);
                allBooks.push_back(newBook);
                cout << "Book '" << title << "' added successfully!\n";
                break;
            }
            case 4: {
                cout << "\n--- Search Book by ID ---\n";
                cout << "Enter Book ID to search: ";
                int id = getIntInput();
                Book* foundBook = bookHash.search(id);
                if (foundBook) {
                    cout << "Book Found: ID: " << foundBook->id << ", Title: " << foundBook->title
                         << ", Topic: " << foundBook->topic << ", Copies: " << foundBook->copies
                         << ", Value: " << foundBook->value << endl;
                } else {
                    cout << "Book with ID " << id << " not found.\n";
                }
                break;
            }
            case 5: {
                cout << "\n--- All Books in Library (Sorted by Title) ---\n";
                bookBST.display();
                break;
            }
            case 6: {
                cout << "\n--- Get Books by Topic ---\n";
                cout << "Enter Topic to search: ";
                string topic = getStringInput();
                vector<Book> booksByTopic = bookBST.getBooksByTopic(topic);
                if (!booksByTopic.empty()) {
                    cout << "Books on topic '" << topic << "':\n";
                    for (size_t i = 0; i < booksByTopic.size(); ++i) {
                        cout << "- ID: " << booksByTopic[i].id << ", Title: " << booksByTopic[i].title
                             << ", Copies: " << booksByTopic[i].copies << endl;
                    }
                } else {
                    cout << "No books found on topic '" << topic << "'.\n";
                }
                break;
            }
            case 7: {
                cout << "\n--- Add Reservation ---\n";
                int userId, bookId, vipChoice;
                bool vip;

                cout << "Enter User ID: ";
                userId = getIntInput();

                if (userDb.getUser(userId) == NULL) {
                    cout << "Error: User with ID " << userId << " not found. Please add the user first.\n";
                    break;
                }

                cout << "Enter Book ID to reserve: ";
                bookId = getIntInput();

                if (bookHash.search(bookId) == NULL) {
                    cout << "Error: Book with ID " << bookId << " does not exist. Cannot add reservation.\n";
                    break;
                }

                cout << "Is this a VIP reservation? (1 for Yes, 0 for No): ";
                vipChoice = getIntInput();
                vip = (vipChoice == 1);

                rs.addReservation(userId, bookId, vip, userDb);
                break;
            }
            case 8: {
                rs.processReservations(bookHash, bookBST, userDb);
                cout << "\n--- Current Book Inventory After Processing ---\n";
                bookBST.display();
                break;
            }
            case 9: {
                cout << "\n--- Borrow a Book ---\n";
                int userId, bookId;
                cout << "Enter User ID: ";
                userId = getIntInput();

                if (userDb.getUser(userId) == NULL) {
                    cout << "Error: User with ID " << userId << " not found. Please add the user first.\n";
                    break;
                }

                cout << "Enter Book ID to borrow: ";
                bookId = getIntInput();

                ReservationSystem::borrowBook(userId, bookId, false, bookHash, bookBST, userDb);
                break;
            }
            case 10: {
                cout << "\n--- Return a Book ---\n";
                int userId, bookId;
                cout << "Enter User ID: ";
                userId = getIntInput();

                if (userDb.getUser(userId) == NULL) {
                    cout << "Error: User with ID " << userId << " not found. Please add the user first.\n";
                    break;
                }

                cout << "Enter Book ID to return: ";
                bookId = getIntInput();
                ReservationSystem::returnBook(userId, bookId, bookHash, userDb);
                break;
            }
            case 11: {
                cout << "\n--- Knapsack Select Books ---\n";
                cout << "Enter maximum number of books to select (capacity): ";
                int capacity = getIntInput();

                if (capacity <= 0) {
                    cout << "Capacity must be a positive number.\n";
                    break;
                }

                vector<Book> selected = knapsackSelectBooks(allBooks, capacity);
                if (!selected.empty()) {
                    cout << "Selected books for maximum value:\n";
                    for (size_t i=0; i < selected.size(); ++i) {
                        cout << "Selected: " << selected[i].title << " (Value: " << selected[i].value << ", ID: " << selected[i].id << ")\n";
                    }
                } else {
                    cout << "No books could be selected for the given capacity.\n";
                }
                break;
            }
            case 12: {
                cout << "Exiting Library Management System.Goodbye!!\n";
                break;
            }
            default: {
                cout << "Invalid choice. Please try again.\n";
                break;
            }
        }
    } while (choice != 12);

    return 0;
}


