#include <iostream>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <iterator>

using namespace std;

// Forward declaration
int compute_score(const std::vector<std::vector<int>>& board);

void write_board_csv(const vector<vector<int>>& board, bool first, const string& stage) {
    ios_base::openmode mode = ios::app;
    if (first) mode = ios::trunc;
    ofstream fout("game_output.csv", mode);
    if (!fout) return;

    fout << stage << ",";
    for (int r=0;r<4;r++){
        for (int c=0;c<4;c++){
            fout<<board[r][c];
            if (!(r==3 && c==3)) fout<<",";
        }
    }
    fout<<"\n";
}

void read_board_csv(vector<vector<int>>& board) {
    ifstream fin("game_input.csv");
    string line;
    int r = 0;
    while (getline(fin, line) && r < 4) {
        stringstream ss(line);
        string cell;
        int c = 0;
        while (getline(ss, cell, ',') && c < 4) {
            try { board[r][c] = stoi(cell); }
            catch (...) { board[r][c] = 0; }
            c++;
        }
        r++;
    }
}

// printing 
void print_board(const vector<vector<int>>& board) {
    cout << "Score: " << compute_score(board) << "\n";
    for (const auto& row : board) {
        for (size_t j = 0; j < row.size(); ++j) {
            if (row[j] == 0) cout << ".";
            else             cout << row[j];
            if (j + 1 < row.size()) cout << "\t";
        }
        cout << "\n";
    }
}

//  compress row 
std::vector<int> compress_row(const std::vector<int>& row) {
    std::vector<int> compressed;
    compressed.reserve(4);
    std::copy_if(row.begin(), row.end(), std::back_inserter(compressed),
                 [](int v){ return v != 0; });
    while (compressed.size() < 4) compressed.push_back(0);
    return compressed;
}

// merge row (assumes compressed)
std::vector<int> merge_row(std::vector<int> row) {
    for (int i = 0; i < 3; ++i) {
        if (row[i] != 0 && row[i] == row[i+1]) {
            row[i] *= 2;
            row[i+1] = 0;
            ++i; 
        }
    }
    return compress_row(row);
}

void spawn_tile(std::vector<std::vector<int>>& board) {
    std::vector<std::pair<int,int>> empty;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            if (board[r][c] == 0) empty.emplace_back(r,c);

    if (empty.empty()) return;

    static std::mt19937 gen(42);  
    std::uniform_int_distribution<> pos_dist(0, (int)empty.size()-1);
    std::uniform_int_distribution<> val_dist(1, 10);

    auto [r, c] = empty[pos_dist(gen)];
    board[r][c] = (val_dist(gen) == 1 ? 4 : 2); 
}

//  moves 
bool move_left(std::vector<std::vector<int>>& board) {
    bool moved = false;
    for (int r = 0; r < 4; ++r) {
        auto before = board[r];
        auto comp   = compress_row(before);
        auto merged = merge_row(comp);
        if (merged != before) moved = true;
        board[r] = merged;
    }
    return moved;
}

bool move_right(std::vector<std::vector<int>>& board) {
    bool moved = false;
    for (int r = 0; r < 4; ++r) {
        auto before = board[r];
        auto row = before;
        std::reverse(row.begin(), row.end());
        row = compress_row(row);
        row = merge_row(row);
        std::reverse(row.begin(), row.end());
        if (row != before) moved = true;
        board[r] = row;
    }
    return moved;
}

bool move_up(std::vector<std::vector<int>>& board) {
    bool moved = false;
    for (int c = 0; c < 4; ++c) {
        std::vector<int> col(4);
        for (int r = 0; r < 4; ++r) col[r] = board[r][c];

        auto before = col;
        col = compress_row(col);
        col = merge_row(col);

        if (col != before) moved = true;
        for (int r = 0; r < 4; ++r) board[r][c] = col[r];
    }
    return moved;
}

bool move_down(std::vector<std::vector<int>>& board) {
    bool moved = false;
    for (int c = 0; c < 4; ++c) {
        std::vector<int> col(4);
        for (int r = 0; r < 4; ++r) col[r] = board[r][c];

        auto before = col;
        std::reverse(col.begin(), col.end());
        col = compress_row(col);
        col = merge_row(col);
        std::reverse(col.begin(), col.end());

        if (col != before) moved = true;
        for (int r = 0; r < 4; ++r) board[r][c] = col[r];
    }
    return moved;
}

int compute_score(const std::vector<std::vector<int>>& board) {
    int score = 0;
    for (const auto& row : board)
        for (int v : row) score += v;
    return score;
}

// main loop with undo 
int main(){
    vector<vector<int>> board(4, vector<int>(4,0));
    read_board_csv(board);

    stack<vector<vector<int>>> history;
    bool first=true;

    while(true){
        print_board(board);
        if (first) {
            write_board_csv(board, true, "initial");
            first = false;
        }

        cout<<"Move (w=up, a=left, s=down, d=right), u=undo, q=quit: ";
        char cmd;
        if (!(cin>>cmd)) break;
        if (cmd=='q') break;

        if (cmd=='u') {
            if (!history.empty()) {
                board = history.top();
                history.pop();
                print_board(board);
                write_board_csv(board, false, "undo");
            }
            continue;
        }

        vector<vector<int>> prev = board;
        bool moved=false;
        if (cmd=='a') moved=move_left(board);
        else if (cmd=='d') moved=move_right(board);
        else if (cmd=='w') moved=move_up(board);
        else if (cmd=='s') moved=move_down(board);

        if (moved) {
            history.push(prev);             
            write_board_csv(board, false, "merge");
            spawn_tile(board);
            write_board_csv(board, false, "spawn");
        } else {
            write_board_csv(board, false, "invalid");
        }
    }
    return 0;
}
