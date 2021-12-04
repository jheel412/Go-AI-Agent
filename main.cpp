#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>
#include <map>

class Coordinate {
  int x;
  int y;
public:
  Coordinate(): x(0), y(0) {}
  Coordinate(int x, int y): x(x), y(y){}
  int getX() const {
    return x;
  }
  int getY() const {
    return y;
  }
  friend std::ostream& operator<<(std::ostream& os, const Coordinate& coordinate);
};

std::ostream& operator<<(std::ostream& os, const Coordinate& coordinate) {
  os << '(' << coordinate.x << ',' << coordinate.y << ')';
  return os;
}

struct CoordinateCompare
{
  bool operator() (const Coordinate& lhs, const Coordinate& rhs) const
  {
    if(lhs.getX() < rhs.getX()) return true;
    else if(lhs.getX() > rhs.getX()) return false;
    else {
      return lhs.getY() < rhs.getY();
    }
  }
};

enum Color {
  WHITE = '2',
  BLACK = '1'
};

class Board {
  std::vector<std::vector<char>> board;
  std::vector<std::vector<bool>> visitedStates;
  std::map<Coordinate, std::vector<Coordinate>, CoordinateCompare> removedCoordinates;
public:
  Board() {
    board.resize(5, std::vector<char>(5, '0'));
    visitedStates.resize(5, std::vector<bool>(5, false));
  }

  void updateCoordinates(const std::vector<Coordinate>& coordinates, char value) {
    for(Coordinate coordinate: coordinates) {
      board[coordinate.getX()][coordinate.getY()] = value;
    }
  }

  void print() {
    for(int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j++) {
        std::cout << board[i][j] << ' ';
      }
      std::cout << '\n';
    }
    std::cout << '\n';
  }

  static bool isEmpty(Board& current) {
    for(int i = 0; i < 5; i++) {
      for(int j = 0; j < 5; j++) {
        if(current.getValue(i, j) != '0') return false;
      }
    }
    return true;
  }

  static bool compareBoards(Board& current, Board& other) {
    for(int i = 0; i < 5; i++) {
      for(int j = 0; j < 5; j++) {
        if(current.getValue(i, j) != other.getValue(i, j)) return false;
      }
    }
    return true;
  }

  static int countPieces(Board& current) {
    int cnt = 0;
    for(int i = 0; i < 5; i++) {
      for(int j = 0; j < 5; j++) {
        if(current.getValue(i, j) != '0') cnt++;
      }
    }
    return cnt;
  }

  static bool checkValidity(int x, int y) {
    return x >= 0 && x < 5 && y >= 0 && y < 5;
  }

  inline void setValue(int i, int j, char c) {
    assert(i >= 0 && i < 5);
    assert(j >= 0 && j < 5);
    board[i][j] = c;
  }

  inline char getValue(int i, int j) {
    assert(i >= 0 && i < 5);
    assert(j >= 0 && j < 5);
    return board[i][j];
  }

  void checkLiberty(
      int i, int j, char c, std::map<int, bool>& isRegionAlive, std::map<int, std::vector<Coordinate>>& pointsInRegion, int counter) {
    visitedStates[i][j] = true;
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};
    pointsInRegion[counter].push_back(Coordinate(i, j));
    for(int x = 0; x < 4; x++) {
      if(checkValidity(dx[x] + i, dy[x] + j) && !visitedStates[dx[x] + i][dy[x] + j]) {
        if(board[dx[x] + i][dy[x] + j] == c) {
          checkLiberty(dx[x] + i, dy[x] + j, c, isRegionAlive, pointsInRegion, counter);
        } else if(board[dx[x] + i][dy[x] + j] == '0') {
          isRegionAlive[counter] = true;
        }
      }
    }
  }

  std::vector<Coordinate> findLibertyAndGetKilledCoordinates(Color color) {
    for(auto & visitedState : visitedStates) {
      std::fill(visitedState.begin(), visitedState.end(), false);
    }

    char c = color;
    std::vector<Coordinate> killedCoordinates;
    int counter = 0;
    std::map<int, bool> isRegionAlive;
    std::map<int, std::vector<Coordinate>> pointsInRegion;
    for(int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j++) {
        if (!visitedStates[i][j] && board[i][j] == c) {
          //mark isRegionAlive for a connected component if it has a '0' neighbor
          isRegionAlive[counter] = false;
          checkLiberty(i, j, c, isRegionAlive, pointsInRegion, counter);
          counter++;
        }
      }
    }
    for(auto value: isRegionAlive) {
      if(!value.second) {
        killedCoordinates.insert(killedCoordinates.end(), pointsInRegion[value.first].begin(), pointsInRegion[value.first].end());
      }
    }
    return killedCoordinates;
  }

  double getScore(Color color) {
    int whiteLiberty = 0, blackLiberty = 0;
    int whitePiece = 0, blackPiece = 0;
    int numEdgeWhite = 0, numEdgeBlack = 0;
    for(int i = 0; i < 5; i++) {
      for(int j = 0; j < 5; j++) {
        if(board[i][j] == '0') {
          int dx[] = {1, -1, 0, 0};
          int dy[] = {0, 0, 1, -1};
          bool black = false;
          bool white = false;
          for(int k = 0; k < 4; k++) {
            if(checkValidity(dx[k] + i, dy[k] + j)) {
              if(board[dx[k] + i][dy[k] + j] == '1') black = true;
              else if(board[dx[k] + i][dy[k] + j] == '2') white = true;
            }
          }
          if(white) whiteLiberty++;
          if(black) blackLiberty++;
        }
        else if(board[i][j] == '1') {
          blackPiece++;
        }
        else if(board[i][j] == '2') {
          whitePiece++;
        }
        if(i == 0 || j == 0 || i == 4 || j == 4){
          if(board[i][j] == '1') numEdgeBlack++;
          else if(board[i][j] == '2') numEdgeWhite++;
        }
      }
    }
    double cntW1 = 0, cntW2 = 0, cntW3 = 0;
    double cntB1 = 0, cntB2 = 0, cntB3 = 0;
    int changeX[] = {0, 0, 1, 1};
    int changeY[] = {0, 1, 0, 1};
    for(int i = 0; i < 4; i++) {
      for(int j = 0; j < 4; j++) {
        int cnt0 = 0, cnt1 = 0, cnt2 = 0;
        for(int k = 0; k < 4; k++) {
          char c = board[i + changeX[k]][j + changeY[k]];
          if(c == '0') {
            cnt0++;
          } else if(c == '1') {
            cnt1++;
          } else {
            cnt2++;
          }
        }
        if(cnt1 == 1 && cnt0 == 3) cntB1++;
        else if(cnt2 == 1 && cnt0 == 3) cntW1++;
        else if(cnt1 == 3 && cnt0 == 1) cntB2++;
        else if(cnt2 == 3 && cnt0 == 1) cntW2++;
        if(board[i][j] == '1' && board[i][j + 1] == '0' && board[i + 1][j] == '0' && board[i + 1][j + 1] == '1') cntB3++;
        if(board[i][j] == '0' && board[i][j + 1] == '1' && board[i + 1][j] == '1' && board[i + 1][j + 1] == '0') cntB3++;
        if(board[i][j] == '2' && board[i][j + 1] == '0' && board[i + 1][j] == '0' && board[i + 1][j + 1] == '2') cntW3++;
        if(board[i][j] == '0' && board[i][j + 1] == '2' && board[i + 1][j] == '2' && board[i + 1][j + 1] == '0') cntW3++;
      }
    }
    double eW = (cntW1 - cntW2 + 2 * cntW3) / 4;
    double eB = (cntB1 - cntB2 + 2 * cntB3) / 4;
    double liberty, e, numPiece, numEdge;
    if(color == WHITE) {
      liberty = whiteLiberty - blackLiberty;
      e = eW - eB;
      numPiece = whitePiece - blackPiece;
      numEdge = numEdgeBlack - numEdgeWhite;
    } else {
      liberty = blackLiberty - whiteLiberty;
      e = eB - eW;
      numPiece = blackPiece - whitePiece;
      numEdge = numEdgeWhite - numEdgeBlack;
    }
    double score = std::min(std::max(liberty, -4.0), 4.0) - 4 * e + 10 * numPiece + numEdge;
    if(color == BLACK) score -= 2.5;
    else score += 2.5;
    return score;
  }

  std::vector<Coordinate> getEmptyPoints() {
    std::vector<Coordinate> points;
    for(int i = 0; i < 5; i++) {
      for(int j = 0; j < 5; j++) {
        if(board[i][j] == '0') points.emplace_back(Coordinate(i, j));
      }
    }
    return points;
  }

  std::vector<Coordinate> filterValidPoints(const std::vector<Coordinate>& emptyPoints, Color c, Board& previousBoard) {
    std::vector<Coordinate> validPoints;
    Color opponent = (c == WHITE ? BLACK : WHITE);
    for(Coordinate coord: emptyPoints) {
      board[coord.getX()][coord.getY()] = c;
      // find liberty of opponent and kill off the opponent's pieces if its liberty becomes 0
      std::vector<Coordinate> killedPiecesOfOpponent =
          findLibertyAndGetKilledCoordinates(opponent);
      updateCoordinates(killedPiecesOfOpponent, '0');
      // if the liberty of any group of pieces is still 0, then this move is suicide
      std::vector<Coordinate> killedPiecesOfSelf =
          findLibertyAndGetKilledCoordinates(c);
      if(killedPiecesOfSelf.empty()) {
        // no KO
        if(!compareBoards(*this, previousBoard)) {
          removedCoordinates[coord] = killedPiecesOfOpponent;
          validPoints.emplace_back(coord);
        }
      }
      // revert the captured coordinates
      updateCoordinates(killedPiecesOfOpponent, opponent);
      board[coord.getX()][coord.getY()] = '0';
    }
    return validPoints;
  }

  std::vector<Coordinate> getRemovedCoordinates(Coordinate coordinate) {
    return removedCoordinates[coordinate];
  }

};


class Player {
  Color color;
public:
  explicit Player(): color(BLACK) {}

  void setColor(char c){
    color = (c == '1' ? BLACK : WHITE);
  }

  Color getColor() {
    return color;
  }
};

class AlphaBeta {
  Board currentBoard;
  Board previousBoard;
  int depth;
  Player player;
  Coordinate nextMove;

public:
  AlphaBeta(Board& previousBoard, Board& currentBoard, int depth, Player player): previousBoard(previousBoard),currentBoard(currentBoard), depth(depth), player(player) {}

  Coordinate getNextMove() {
    alphaBeta(depth, currentBoard, previousBoard, player.getColor(), INT32_MIN, INT32_MAX, false);
    return nextMove;
  }

  double alphaBeta(int currentDepth, Board curBoard, Board preBoard, Color color, double alpha, double beta, bool flag) {
    if(flag) {
      std::cout << currentDepth << '\n';
      curBoard.print();
    }
    if(currentDepth == 0) {
      if(flag) {
        std::cout << curBoard.getScore(player.getColor()) << '\n';
      }
      return curBoard.getScore(player.getColor());
    }
    std::vector<Coordinate> emptyPoints = curBoard.getEmptyPoints();
    std::vector<Coordinate> validPoints =
        curBoard.filterValidPoints(emptyPoints, color, preBoard);
    validPoints.emplace_back(Coordinate(-1, -1));
    bool maxPlayer = (color == player.getColor());
    Board nextPreBoard = curBoard;
    if(maxPlayer) {
      double bestValue = INT32_MIN;
      for (Coordinate coordinate: validPoints) {
        std::vector<Coordinate> removed;
        if(coordinate.getX() != -1) {
          curBoard.setValue(coordinate.getX(), coordinate.getY(), color);
          removed = curBoard.getRemovedCoordinates(coordinate);
          curBoard.updateCoordinates(removed, '0');
        }
        if(currentDepth == depth && coordinate.getX() == 2 && coordinate.getY() == 4) flag = true;
        double val = alphaBeta(currentDepth - 1, curBoard, nextPreBoard, color == WHITE ? BLACK : WHITE, alpha, beta, flag);

        if(depth == currentDepth) {
          std::cout << coordinate << ' ' << val << '\n';
        }
        if(val > bestValue) {
          if(depth == currentDepth) {
            nextMove = coordinate;
          }
          bestValue = val;
        }
        alpha = std::max(bestValue, alpha);
        if(beta <= alpha) {
          break;
        }
        if(coordinate.getX() != -1) {
          curBoard.setValue(coordinate.getX(), coordinate.getY(), '0');
          removed = curBoard.getRemovedCoordinates(coordinate);
          curBoard.updateCoordinates(removed, color == BLACK ? WHITE : BLACK);
        }
      }
      return bestValue;
    } else {
      double bestValue = INT32_MAX;
      for (Coordinate coordinate: validPoints) {
        if(coordinate.getX() != -1) {
          curBoard.setValue(coordinate.getX(), coordinate.getY(), color);
          std::vector<Coordinate> removed = curBoard.getRemovedCoordinates(coordinate);
          curBoard.updateCoordinates(removed, '0');
        }
        double val = alphaBeta(currentDepth - 1, curBoard, nextPreBoard, color == WHITE ? BLACK : WHITE, alpha, beta, flag);
        bestValue = std::min(bestValue, val);
        beta = std::min(bestValue, beta);
        if(beta <= alpha) {
          break;
        }
        if(coordinate.getX() != -1) {
          curBoard.setValue(coordinate.getX(), coordinate.getY(), '0');
          std::vector<Coordinate> removed = curBoard.getRemovedCoordinates(coordinate);
          curBoard.updateCoordinates(removed, color == BLACK ? WHITE : BLACK);
        }
      }
      return bestValue;
    }
  }
};


class Game {
  Player player1;
  Player player2;
  Board previousBoard;
  Board currentBoard;
  Coordinate nextMove;
  int numMoves;
public:

  Game(): numMoves(0) {}

  void play() {
    //perform alpha-beta pruning for all valid points
    int depth = 5;
    if(numMoves <= 6) {
      depth = 4;
    } else if(numMoves >= 12 && numMoves <= 19) {
      depth = 5;
    } else if(numMoves >= 20) {
      depth = 24 - numMoves;
    }
    AlphaBeta alphaBeta(previousBoard, currentBoard, depth, player1);
    nextMove = alphaBeta.getNextMove();
    writeResultsToFile();
  }

  void setColors(char color) {
    player1.setColor(color == '1' ? BLACK : WHITE);
    player2.setColor(color == '1' ? WHITE : BLACK);
  }

  void readBoard(){
    std::ifstream inputFile("input.txt");
    if(inputFile.is_open()) {
      std::string input;
      char color;
      inputFile >> color;
      setColors(color);
      inputFile.ignore();
      for(int i = 0; i < 5; i++) {
        std::getline(inputFile, input);
        for(int j = 0; j < 5; j++) {
          previousBoard.setValue(i, j, input[j]);
        }
      }
      for(int i = 0; i < 5; i++) {
        std::getline(inputFile, input);
        for(int j = 0; j < 5; j++) {
          currentBoard.setValue(i, j, input[j]);
        }
      }
    }
    inputFile.close();
    if(Board::isEmpty(currentBoard) && Board::isEmpty(previousBoard)) {
      std::ofstream moveFile;
      moveFile.open("moves.txt");
      moveFile << 0;
      numMoves = 0;
      moveFile.close();
    } else if(Board::isEmpty(previousBoard)) {
      std::ofstream moveFile;
      moveFile.open("moves.txt");
      moveFile << 1;
      numMoves = 1;
      moveFile.close();
    } else {
      std::ifstream moveInputFile("moves.txt");
      std::string moves;
      getline(moveInputFile, moves);
      moveInputFile.close();
      numMoves = std::stoi(moves) + 2;
      std::ofstream moveOutputFile;
      moveOutputFile.open("moves.txt");
      moveOutputFile << numMoves;
      moveOutputFile.close();
    }
  }

  void writeResultsToFile() {
    std::ofstream outputFile;
    outputFile.open("output.txt");
    if(nextMove.getX() == -1) {
      outputFile << "PASS";
    } else {
      outputFile << nextMove.getX() << "," << nextMove.getY();
    }
    outputFile.close();
  }
};

int main() {
  Game go;
  go.readBoard();
  go.play();
  return 0;
}
