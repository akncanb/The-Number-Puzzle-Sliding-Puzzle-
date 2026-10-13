#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <string>
#include <cstdlib>

using namespace std;

struct Move {
    int r;
    int c;
};

struct Stack {
    Move* data;
    int top;
    int capacity;

    Stack(int cap) {
        capacity = cap;
        data = new Move[capacity];
        top = -1;
    }

    void push(Move m) {
        if (top < capacity - 1) {
            data[++top] = m;
        } else {
            for (int i = 0; i < capacity - 1; i++) {
                data[i] = data[i + 1];
            }
            data[top] = m;
        }
    }

    Move pop() {
        if (top >= 0) return data[top--];
        return {-1, -1};
    }

    bool isEmpty() { return top == -1; }

    ~Stack() { delete[] data; }
};


int** allocateBoard(int N);
void deallocateBoard(int** board, int N);
void initializeGoal(int** board, int N);
void drawBoard(int** board, int N);
bool isWin(int** current, int** goal, int N);
int calculateManhattan(int** current, int** goal, int N);
void shuffleBoard(int** current, int N, int& br, int& bc);
void saveGame(int** current, int** goal, int N, int moves, Stack* s, int br, int bc);
bool loadGame(int**& current, int**& goal, int& N, int& moves, Stack* s, int& br, int& bc);


bool registerUser();
bool loginUser();

int main() {
    srand(time(0));


    int authChoice;
    bool isAuthenticated = false;

    while (!isAuthenticated) {
        system("CLS");
        cout << "=================================" << endl;
        cout << "   WELCOME TO THE NUMBER PUZZLE  " << endl;
        cout << "=================================" << endl;
        cout << "1. Register" << endl;
        cout << "2. Login" << endl;
        cout << "3. Exit" << endl;
        cout << "Choice: ";
        cin >> authChoice;

        if (authChoice == 1) {
            registerUser();
            system("PAUSE");
        } else if (authChoice == 2) {
            if (loginUser()) {
                isAuthenticated = true;
                system("PAUSE");
            }
        } else if (authChoice == 3) {
            cout << "Exiting program..." << endl;
            return 0;
        } else {
            cout << "Invalid choice! Please try again." << endl;
            system("PAUSE");
        }
    }


    int N = 0;
    int totalMoves = 0;
    int blankRow = 0, blankCol = 0;
    int** goalBoard = NULL;
    int** currentBoard = NULL;
    Stack* undoStack = new Stack(5);

    system("CLS");
    cout << "--- SLIDING PUZZLE (GAME 3) ---" << endl;
    cout << "1. New Game" << endl;
    cout << "2. Load Saved Game" << endl;
    cout << "Choice: ";
    int choice;
    cin >> choice;

    if (choice == 2) {
        if (!loadGame(currentBoard, goalBoard, N, totalMoves, undoStack, blankRow, blankCol)) {
            cout << "Failed to load game. Exiting..." << endl;
            delete undoStack;
            return 1;
        }
        cout << "Game loaded successfully!" << endl;
        system("PAUSE");
    } else {
        cout << "Enter board size (3: Easy, 4: Medium, 5: Hard): ";
        cin >> N;

        if (N < 2) {
            cout << "Invalid board size!" << endl;
            delete undoStack;
            return 1;
        }

        goalBoard = allocateBoard(N);
        currentBoard = allocateBoard(N);

        if (goalBoard == NULL || currentBoard == NULL) {
            cout << "Memory allocation failed!" << endl;
            delete undoStack;
            return 1;
        }

        initializeGoal(goalBoard, N);

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                currentBoard[i][j] = goalBoard[i][j];
            }
        }

        blankRow = N - 1;
        blankCol = N - 1;

        cout << "Shuffling board (1000+ slides)..." << endl;
        shuffleBoard(currentBoard, N, blankRow, blankCol);
    }

    int tileInput;
    while (true) {
        system("CLS");
        drawBoard(currentBoard, N);

        cout << "Distance to solution: " << calculateManhattan(currentBoard, goalBoard, N) << endl;
        cout << "Total Moves: " << totalMoves << endl;
        cout << "Commands: [Tile Number] Move | [-1] Undo | [-2] Save & Exit | [0] Exit Without Saving" << endl;
        cout << "Action: ";

        if (!(cin >> tileInput)) {
            cin.clear();
            cin.ignore(100, '\n');
            continue;
        }

        if (tileInput == 0) break;

        if (tileInput == -2) {
            saveGame(currentBoard, goalBoard, N, totalMoves, undoStack, blankRow, blankCol);
            break;
        }

        if (tileInput == -1) {
            if (!undoStack->isEmpty()) {
                Move last = undoStack->pop();
                int temp = currentBoard[last.r][last.c];
                currentBoard[last.r][last.c] = 0;
                currentBoard[blankRow][blankCol] = temp;
                blankRow = last.r;
                blankCol = last.c;
                totalMoves++;
            } else {
                cout << "No more moves to undo!" << endl;
                system("PAUSE");
            }
            continue;
        }

        bool moved = false;
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (currentBoard[i][j] == tileInput && tileInput != 0) {
                    if (abs(i - blankRow) + abs(j - blankCol) == 1) {
                        undoStack->push({blankRow, blankCol});

                        currentBoard[blankRow][blankCol] = tileInput;
                        currentBoard[i][j] = 0;

                        blankRow = i;
                        blankCol = j;

                        totalMoves++;
                        moved = true;
                        break;
                    }
                }
            }
            if(moved) break;
        }

        if (!moved && tileInput != -1 && tileInput != -2) {
            cout << "Invalid move! Tile must be next to the blank space." << endl;
            system("PAUSE");
        }

        if (isWin(currentBoard, goalBoard, N)) {
            system("CLS");
            drawBoard(currentBoard, N);
            cout << "CONGRATULATIONS! Puzzle solved in " << totalMoves << " moves." << endl;

            remove("savegame.txt");
            break;
        }
    }

    deallocateBoard(goalBoard, N);
    deallocateBoard(currentBoard, N);
    delete undoStack;

    return 0;
}



bool registerUser() {
    string username, password, fileUser, filePass;
    cout << "\n--- REGISTER NEW USER ---" << endl;
    cout << "Enter new username: ";
    cin >> username;
    cout << "Enter new password: ";
    cin >> password;


    ifstream readFile("users.txt");
    if (readFile.is_open()) {
        while (readFile >> fileUser >> filePass) {
            if (fileUser == username) {
                cout << "Registration failed! Username already exists." << endl;
                readFile.close();
                return false;
            }
        }
        readFile.close();
    }


    ofstream writeFile("users.txt", ios::app);
    if (writeFile.is_open()) {
        writeFile << username << " " << password << endl;
        writeFile.close();
        cout << "Registration successful! You can now log in." << endl;
        return true;
    }

    cout << "Database file error!" << endl;
    return false;
}

bool loginUser() {
    string username, password, fileUser, filePass;
    int attempts = 0;

    while (attempts < 3) {
        cout << "\n--- LOGIN ---" << endl;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        ifstream readFile("users.txt");
        bool credentialsMatch = false;

        if (readFile.is_open()) {
            while (readFile >> fileUser >> filePass) {
                if (fileUser == username && filePass == password) {
                    credentialsMatch = true;
                    break;
                }
            }
            readFile.close();
        }

        if (credentialsMatch) {
            cout << "Login successful! Loading game components..." << endl;
            return true;
        } else {
            attempts++;
            cout << "Incorrect username or password! Attempts remaining: " << (3 - attempts) << endl;
        }
    }

    cout << "3 failed attempts reached. Security termination." << endl;
    exit(0);
    return false;
}



int** allocateBoard(int N) {
    int** b = new int*[N];
    for (int i = 0; i < N; i++) {
        b[i] = new int[N];
    }
    return b;
}

void deallocateBoard(int** board, int N) {
    if (board == NULL) return;
    for (int i = 0; i < N; i++) {
        delete[] board[i];
    }
    delete[] board;
}

void initializeGoal(int** board, int N) {
    int counter = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            board[i][j] = counter++;
        }
    }
    board[N - 1][N - 1] = 0;
}

void drawBoard(int** board, int N) {
    cout << "\n      --- BOARD ---" << endl;
    for (int i = 0; i < N; i++) {
        cout << "      ";
        for (int j = 0; j < N; j++) {
            if (board[i][j] == 0) cout << "   ";
            else printf("%2d ", board[i][j]);
        }
        cout << endl;
    }
    cout << "      -------------" << endl;
}

bool isWin(int** current, int** goal, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (*(*(current + i) + j) != *(*(goal + i) + j)) {
                return false;
            }
        }
    }
    return true;
}

int calculateManhattan(int** current, int** goal, int N) {
    int totalDist = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int val = current[i][j];
            if (val != 0) {
                int targetR = (val - 1) / N;
                int targetC = (val - 1) % N;
                totalDist += abs(i - targetR) + abs(j - targetC);
            }
        }
    }
    return totalDist;
}

void shuffleBoard(int** current, int N, int& br, int& bc) {
    int count = 0;
    while (count < 1000) {
        int dir = rand() % 4;
        int dr[] = {0, 0, 1, -1};
        int dc[] = {1, -1, 0, 0};

        int nr = br + dr[dir];
        int nc = bc + dc[dir];

        if (nr >= 0 && nr < N && nc >= 0 && nc < N) {
            current[br][bc] = current[nr][nc];
            current[nr][nc] = 0;
            br = nr;
            bc = nc;
            count++;
        }
    }
}

void saveGame(int** current, int** goal, int N, int moves, Stack* s, int br, int bc) {
    ofstream file("savegame.txt");
    if (!file.is_open()) {
        cout << "Error: Could not create save file!" << endl;
        system("PAUSE");
        return;
    }

    file << N << " " << moves << " " << br << " " << bc << endl;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            file << current[i][j] << " ";
        }
        file << endl;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            file << goal[i][j] << " ";
        }
        file << endl;
    }

    file << s->top << endl;
    for (int i = 0; i <= s->top; i++) {
        file << s->data[i].r << " " << s->data[i].c << endl;
    }

    file.close();
    cout << "Game saved successfully to savegame.txt! Exiting..." << endl;
    system("PAUSE");
}

bool loadGame(int**& current, int**& goal, int& N, int& moves, Stack* s, int& br, int& bc) {
    ifstream file("savegame.txt");
    if (!file.is_open()) {
        cout << "No saved game found (savegame.txt missing)!" << endl;
        return false;
    }

    file >> N >> moves >> br >> bc;

    current = allocateBoard(N);
    goal = allocateBoard(N);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            file >> current[i][j];
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            file >> goal[i][j];
        }
    }

    file >> s->top;
    for (int i = 0; i <= s->top; i++) {
        file >> s->data[i].r >> s->data[i].c;
    }

    file.close();
    return true;
}

