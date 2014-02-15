#include <set>
#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <algorithm>

#include <assert.h>

using namespace std;

// The board, as far as white tiles are concerned, is this:
//
//   0 1 2
//    3 4 
//   5 6 7
//    8 9 
//
// The two white bishops initially live on 0 and 5,
// and the two black ones on 2 and 7.
//
// We represent the board state by the 4 indexes of
// the two white and the two black bishops:
typedef vector<int> Board; 

// The map of what tiles are threatened by each tile
// is this:
map<int, set<int>> g_threats = {
    {0, {3,6,9}       },
    {1, {3,5,4,7}     },
    {2, {4,6,8}       },
    {3, {0,1,5,6,9}   },
    {4, {1,2,6,8,7}   },
    {5, {3,1,8}       },
    {6, {0,3,4,2,8,9} },
    {7, {1,4,9}       },
    {8, {5,6,4,2}     },
    {9, {6,3,0,7}     },
};

map<tuple<int, int>, list<int>> g_checkEmpty = {
    {make_tuple(0,6), {3}},
    {make_tuple(0,9), {3,6}},
    {make_tuple(1,5), {3}},
    {make_tuple(1,7), {4}},
    {make_tuple(2,6), {4}},
    {make_tuple(2,8), {4,6}},
    {make_tuple(3,9), {6}},
    {make_tuple(4,8), {6}},
    {make_tuple(5,1), {3}},
    {make_tuple(6,0), {3}},
    {make_tuple(6,2), {4}},
    {make_tuple(7,1), {4}},
    {make_tuple(8,4), {6}},
    {make_tuple(8,2), {6,4}},
    {make_tuple(9,0), {6,3}},
    {make_tuple(9,3), {6}},
};

bool threatens(int i, int j)
{
    return g_threats[i].count(j) == 1;
}

// We'll brute force our way via a depth-first-search
// of board states, until we have tiles 2,7 containing White
// and tiles 0,5 containing Black.
//
// We need to keep a set of the visited board states.
// However, we need to be careful: the board state is
// indifferent to the placement of white bishop 1 and white
// bishop 2 - i.e. they can swap places, and it's still 
// the same board. Ditto for the two black bishops - 
// we therefore need a set with a special comparator...

Board orderBoard(const Board& b) {
    int whiteSrc = b[0]<b[1]?0:1;
    int blackSrc = b[2]<b[3]?2:3;
    return Board({
        b[whiteSrc], b[whiteSrc^1],
        b[blackSrc], b[blackSrc^1]});
}

struct BoardComparator {
    bool operator()(const Board& lhs, const Board& rhs) const {
        assert(lhs.size() == 4);
        assert(rhs.size() == 4);
        assert(lhs[0] != lhs[1]);
        assert(lhs[2] != lhs[3]);
        assert(rhs[0] != rhs[1]);
        assert(rhs[2] != rhs[3]);
        return orderBoard(lhs) < orderBoard(rhs);
    }
};

set<Board, BoardComparator> g_visited;

// Verifying that indeed the visited set is order agnostic:
//
//int main()
//{
//    Board b1 = {0,5,7,2};
//    Board b2 = {5,0,2,7};
//    Board b3 = {5,0,7,2};
//    g_visited.insert(b1);
//    cout << g_visited.count(b2);
//    cout << g_visited.count(b3);
//}

list<Board> g_moves;

void solve(const Board& board)
{
    static auto targetBoard = Board({2,7,0,5});
    g_visited.insert(board);
    if (orderBoard(board) == targetBoard) {
        cout << "Solved :-)" << endl;
        for_each(g_moves.begin(), g_moves.end(), [](const Board& b) {
            cout << b[0] << "," << b[1] << "," << b[2] << "," << b[3] << "\n";
        });
        cout << endl;
        exit(0);
    }
    int w1 = board[0];
    int w2 = board[1];
    int b1 = board[2];
    int b2 = board[3];
    for(auto& w: g_threats[w1]) {
        if (w == w2 || w == b1 || w == b2) continue;
        if (threatens(b1,w) || threatens(b2,w)) continue;
        auto it = g_checkEmpty.find(make_tuple(w1, w));
        if (it != g_checkEmpty.end()) {
            bool canMove = true;
            for(auto& c: it->second) {
                if (c == w2 || c == b1 || c == b2) {
                    canMove = false;
                    break;
                }
            }
            if (!canMove)
                continue;
        }
        auto boardNew = Board({w, w2, b1, b2});
        if (g_visited.count(boardNew)) continue;
        g_moves.push_back(boardNew);
        solve(boardNew);
        g_moves.pop_back();
    }
    for(auto& w: g_threats[w2]) {
        if (w == w1 || w == b1 || w == b2) continue;
        if (threatens(b1,w) || threatens(b2,w)) continue;
        auto it = g_checkEmpty.find(make_tuple(w2, w));
        if (it != g_checkEmpty.end()) {
            bool canMove = true;
            for(auto& c: it->second) {
                if (c == w1 || c == b1 || c == b2) {
                    canMove = false;
                    break;
                }
            }
            if (!canMove)
                continue;
        }
        auto boardNew = Board({w1, w, b1, b2});
        if (g_visited.count(boardNew)) continue;
        g_moves.push_back(boardNew);
        solve(boardNew);
        g_moves.pop_back();
    }
    for(auto& b: g_threats[b1]) {
        if (b == b2 || b == w1 || b == w2) continue;
        if (threatens(b,w1) || threatens(b,w2)) continue;
        auto it = g_checkEmpty.find(make_tuple(b1, b));
        if (it != g_checkEmpty.end()) {
            bool canMove = true;
            for(auto& c: it->second) {
                if (c == b2 || c == w1 || c == w2) {
                    canMove = false;
                    break;
                }
            }
            if (!canMove)
                continue;
        }
        auto boardNew = Board({w1, w2, b, b2});
        if (g_visited.count(boardNew)) continue;
        g_moves.push_back(boardNew);
        solve(boardNew);
        g_moves.pop_back();
    }
    for(auto& b: g_threats[b2]) {
        if (b == b1 || b == w1 || b == w2) continue;
        if (threatens(b,w1) || threatens(b,w2)) continue;
        auto it = g_checkEmpty.find(make_tuple(b2, b));
        if (it != g_checkEmpty.end()) {
            bool canMove = true;
            for(auto& c: it->second) {
                if (c == b1 || c == w1 || c == w2) {
                    canMove = false;
                    break;
                }
            }
            if (!canMove)
                continue;
        }
        auto boardNew = Board({w1, w2, b1, b});
        if (g_visited.count(boardNew)) continue;
        g_moves.push_back(boardNew);
        solve(boardNew);
        g_moves.pop_back();
    }
}

int main()
{
    Board initial = {0,5,2,7};
    g_moves.push_back(initial);
    solve(initial);
}
