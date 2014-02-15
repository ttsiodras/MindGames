#include <set>
#include <map>
#include <list>
#include <vector>
#include <iostream>

#include <assert.h>

using namespace std;

// The tiles on the board where the bishops can land on:
//
//   0 1 2
//    3 4
//   5 6 7
//    8 9
//
// The two white bishops (w1,w2 henceforth) are initially on 0 and 5,
// and the two black ones (b1, b2) on 2 and 7.
//
// We represent the board state by the 4 indexes of the two white and
// the two black bishops, i.e. a simple vector of [w1, w2, b1, b2]:
typedef vector<int> Board;

// The map of what tiles are threatened by each tile
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

// Does tile i threaten tile j?
bool threatens(int i, int j)
{
    return g_threats[i].count(j) == 1;
}

// The map of what intermediate cells must be empty
// to move from tile i to tile j (the two integers in the tuple key)
map<tuple<int, int>, list<int>> g_checkEmpty = {
    { make_tuple(0,6), {3}   }, // To go from 0 to 6, tile 3 must be empty
    { make_tuple(0,9), {3,6} }, // To go from 0 to 9, tiles 3, 6 must be empty
    { make_tuple(1,5), {3}   }, // etc
    { make_tuple(1,7), {4}   },
    { make_tuple(2,6), {4}   },
    { make_tuple(2,8), {4,6} },        // Reminder of tile placement:
    { make_tuple(3,9), {6}   },        //
    { make_tuple(4,8), {6}   },        //           0 1 2
    { make_tuple(5,1), {3}   },        //            3 4
    { make_tuple(6,0), {3}   },        //           5 6 7
    { make_tuple(6,2), {4}   },        //            8 9
    { make_tuple(7,1), {4}   },
    { make_tuple(8,4), {6}   },
    { make_tuple(8,2), {6,4} },
    { make_tuple(9,0), {6,3} },
    { make_tuple(9,3), {6}   },
};

// We need to keep a set of the visited board states.
// However, we need to be careful: the board state is
// indifferent to the placement of white bishop 1 and white
// bishop 2 - i.e. they can swap places, and it's still
// the same board. Ditto for the two black bishops...
// Our visited set must therefore use a special comparator:

Board orderBoard(const Board& b) {
    // Basically, sort the white and black indexes,
    // making sure that the smallest white appears first,
    // then the largest white, then the smallest black,
    // then the largest black.
    int whiteSrc = b[0]<b[1]?0:1, blackSrc = b[2]<b[3]?2:3;
    return Board({b[whiteSrc], b[whiteSrc^1], b[blackSrc], b[blackSrc^1]});
}

// ...and use these 'sorted' versions of the boards to
// perform the set comparisons... below we will use this
// custom comparator to declare our custom set:
//
//   set<Board, BoardComparator> visited;
//
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

// Used to print the solution, when it is found.
void printBoard(const Board& board)
{
    static map<int, vector<int>> tileNumberToXY = {
        {0, {0,0}}, {1, {2,0}}, {2, {4,0}}, {3, {1,1}}, {4, {3,1}},
        {5, {0,2}}, {6, {2,2}}, {7, {4,2}}, {8, {1,3}}, {9, {3,3}},
    };
    int w1=board[0], w2=board[1], b1=board[2], b2=board[3];
    auto whiteBishop1xy = tileNumberToXY[w1];
    auto whiteBishop2xy = tileNumberToXY[w2];
    auto blackBishop1xy = tileNumberToXY[b1];
    auto blackBishop2xy = tileNumberToXY[b2];
    for(int i=0; i<4; i++) {
        cout << "|";
        for(int j=0; j<5; j++) {
            if ((j == whiteBishop1xy[0] && i == whiteBishop1xy[1]) ||
                (j == whiteBishop2xy[0] && i == whiteBishop2xy[1]))
                cout << "W|";
            else if ((j == blackBishop1xy[0] && i == blackBishop1xy[1]) ||
                     (j == blackBishop2xy[0] && i == blackBishop2xy[1]))
                cout << "B|";
            else
                cout << " |";
        }
        cout << endl;
    }
}

void checkIfItsSolved(
    Board& board, const Board& startingBoard, map<Board,Board>& previousMoves)
{
    // Do we have the two white bishops in tiles 2 and 7,
    // and the two black ones in 0 and 5?
    static auto targetBoard = Board({2,7,0,5});

    // Remember to compare in an order-agnostic way!
    if (orderBoard(board) == targetBoard) {
        list<Board> solution;
        while(board != startingBoard) {
            solution.push_front(board);
            board = previousMoves[board];
        }
        cout << "\nSolved in " << solution.size() << " moves! :-)\n" << endl;

        // Start printing with the board we begun from...
        printBoard(startingBoard);
        for(auto b: solution) {
            cout << "\nPress ENTER to show next move...\n";
            cin.get();
            printBoard(b);
        }
        exit(0);
    }
}

// The brains of the operation - basically a Breadth-First-Search
// of the problem space (BFS):
//    http://en.wikipedia.org/wiki/Breadth-first_search
//
void solve(const Board& startingBoard)
{
    // The visited set, using our custom comparator
    set<Board, BoardComparator> visited;

    // The "what was the previous step" keeper, per board
    // (used to print the solution)
    map<Board, Board> previousMoves;

    // The queue where BFS adds boards for review (at the end)
    list<Board> boardReviewQueue;

    // Setup Breadth-First-Search...
    visited.insert(startingBoard);
    boardReviewQueue.push_back(startingBoard);

    while(!boardReviewQueue.empty()) {
        auto board = *boardReviewQueue.begin();
        boardReviewQueue.pop_front();

        checkIfItsSolved(board, startingBoard, previousMoves);

        // w1, w2: the tile indexes of the two white bishops
        // b1, b2: the tile indexes of the two black bishops
        int w1=board[0], w2=board[1], b1=board[2], b2=board[3];

        // Closure that checks for valid moves of a bishop and adds
        // the resulting boards to the Q for review.
        auto addBoardsForReview =
            // Variables captured in the closure:
            [&visited, &previousMoves, &boardReviewQueue,
             &board, &w1, &w2, &b1, &b2]
            // one closure argument, the index of the bishop we will be moving:
            (int bishopIndex )
        {
            int src, otherBishopOfSameColor, diffColor1, diffColor2;
            switch(bishopIndex) {
                // 0 means move w1 (white bishop 1),
                case 0: src = w1; otherBishopOfSameColor = w2;
                        diffColor1 = b1; diffColor2 = b2; break;
                // 1 means move w2 (white bishop 2),
                case 1: src = w2; otherBishopOfSameColor = w1;
                        diffColor1 = b1; diffColor2 = b2; break;
                // 2 means move b1 (black bishop 1),
                case 2: src = b1; otherBishopOfSameColor = b2;
                        diffColor1 = w1; diffColor2 = w2; break;
                // 3 means move b2 (black bishop 2),
                case 3: src = b2; otherBishopOfSameColor = b1;
                        diffColor1 = w1; diffColor2 = w2; break;
                default: assert(0);
            }
            // Try moving the 'src' bishop to all the tiles he can threaten...
            for(auto& dest: g_threats[src]) {
                // But is that tile empty?
                if (dest == otherBishopOfSameColor ||
                        dest == diffColor1 || dest == diffColor2)
                    continue;
                // Is it theatened by the two other color bishops?
                if (threatens(diffColor1,dest) || threatens(diffColor2,dest))
                    continue;
                // Finally, does it have some other bishop blocking its move?
                auto it = g_checkEmpty.find(make_tuple(src, dest));
                if (it != g_checkEmpty.end()) {
                    // There appear to be interim tiles we'll need to traverse
                    // when making this move. Are they all empty?
                    bool canMove = true;
                    for(auto& c: it->second) {
                        if (c == otherBishopOfSameColor ||
                                c == diffColor1 || c == diffColor2) {
                            canMove = false;
                            break;
                        }
                    }
                    if (!canMove)
                        continue; // try next tile...
                }
                auto boardNew =
                    (bishopIndex == 0)?Board({dest,w2,b1,b2}):
                    (bishopIndex == 1)?Board({w1,dest,b1,b2}):
                    (bishopIndex == 2)?Board({w1,w2,dest,b2}):
                                       Board({w1,w2,b1,dest});
                // Have we already seen this new board?
                if (visited.count(boardNew))
                    continue; // yes, try next tile...

                // No, so add it now to all 3 BFS containers:
                // - the visited set
                visited.insert(boardNew);
                // - the map keeping what move was used to reach this board
                previousMoves[boardNew] = board;
                // - the review Q
                boardReviewQueue.push_back(boardNew);
            }
        };

        addBoardsForReview(0); // Try moving the first white bishop,
        addBoardsForReview(1); // ...then the second,
        addBoardsForReview(2); // ...then the first black bishop,
        addBoardsForReview(3); // ...and finally the second black bishop.
    }
}

int main()
{
    Board initial = {0,5,2,7};
    solve(initial);
}
