#include "neural_network/instruments/matrix.hpp"
#include <functional>

enum class CellType : int {
    Empty = 0,
    Ship = 1,
    Miss = 2,
    Hit = 3,
    Kill = 4
};

class Board {
private:
    static constexpr int cSize_ = 10;
    Matrix board_;
    std::function<void(const Matrix&)> Rendered_;
public:
    explicit Board(std::function<void(const Matrix&)> Rendered_ = nullptr,
                   std::function<void(Matrix&)> ShipPlacement = nullptr);

    void render() const;

    void SetCell(int x, int y, CellType type);

};