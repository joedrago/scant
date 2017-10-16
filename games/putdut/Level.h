#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>

class Level
{
public:
    Level();
    ~Level();

    enum CellType
    {
        CT_NONE = 0,

        CT_WALL,
        CT_FLOOR,
        CT_DEST,

        CT_COUNT
    };

    static const int MAX_W = 32;
    static const int MAX_H = 24;

    bool win();

    bool parse(const std::vector<std::string> & lines);
    void copy(Level &src);

    struct Cell
    {
        bool box_;
        bool dest_;
        bool floor_;
        bool start_;
        bool wall_;

        void clear()
        {
            box_ = false;
            dest_ = false;
            floor_ = false;
            start_ = false;
            wall_ = false;
        }
    };

    std::string title_;
    Cell cells_[MAX_W][MAX_H];
    int startX_;
    int startY_;
    // TODO: add box positions
};

#endif // ifndef LEVEL_H
