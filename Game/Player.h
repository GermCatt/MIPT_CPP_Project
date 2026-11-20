#include "board.hpp"

struct Point {
    int x_;
    int y_;
};

class Player {
private:
    Board own_board_;
    Board radar_board_;
    std::function<Point()> ShotInput_;
public:
    explicit Player(std::function<void(const Matrix&)> Rendered_ = nullptr,
                    std::function<Point()> ShotInput_ = nullptr,
                    std::function<void(Matrix&)> ShipPlacement = nullptr);
    Point Shot() const;

    CellType ExecutionShot(Point target);
};
