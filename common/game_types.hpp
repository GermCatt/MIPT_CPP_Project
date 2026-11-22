//
// Created by Леонид Наумов on 22.11.2026.
//

#ifndef CPP_PROJECT_GAMETYPES_H
#define CPP_PROJECT_GAMETYPES_H
struct Position {
    int x;
    int y;
};

enum class ShotResult {
    hit,
    kill,
    miss,
    AlreadyShot
};

enum class PlayerId {
    first,
    second
};
#endif //CPP_PROJECT_GAMETYPES_H
