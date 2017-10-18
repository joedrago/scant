#include "Level.h"

Level::Level()
    : theme_(0)
{
    memset(cells_, CT_NONE, sizeof(cells_));
}

Level::~Level()
{
}

void Level::copy(Level & src)
{
    name_ = src.name_;
    nickname_ = src.nickname_;
    theme_ = src.theme_;
    intro_ = src.intro_;
    memcpy(cells_, src.cells_, sizeof(cells_));
    startX_ = src.startX_;
    startY_ = src.startY_;
}

bool Level::win()
{
    for (int j = 0; j < MAX_H; ++j) {
        for (int i = 0; i < MAX_W; ++i) {
            Cell & cell = cells_[i][j];
            if(cell.dest_ && !cell.box_) {
                return false;
            }
        }
    }
    return true;
}

bool Level::parse(const std::vector<std::string> & lines)
{
    if (lines.size() != MAX_H)
        return false;

    startX_ = 0;
    startY_ = 0;

    int row = 0;
    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
        const std::string & line = *it;
        if (line.size() != MAX_W)
            return false;

        for (int i = 0; i < MAX_W; ++i) {
            Cell & cell = cells_[i][row];
            cell.clear();
            switch (line[i]) {
                case 'X':
                    cell.wall_ = true;
                    break;
                case '_':
                    cell.floor_ = true;
                    break;
                case '*':
                    cell.box_ = true;
                    cell.floor_ = true;
                    break;
                case '.':
                    cell.dest_ = true;
                    cell.floor_ = true;
                    break;
                case '&':
                    cell.box_ = true;
                    cell.dest_ = true;
                    cell.floor_ = true;
                    break;
                case '@':
                    cell.start_ = true;
                    cell.floor_ = true;
                    startX_ = i;
                    startY_ = row;
                    break;
                default:
                    break;
            }
        }
        ++row;
    }
    return true;
}
