#include <ncurses.h>
#include <vector>
#include <string>
#include <memory>

// Цветовые пары
#define CP_PLAYER 1
#define CP_NPC    2
#define CP_WALL   3
#define CP_HOUSE  4

enum GameState { MENU, WORLD, HOUSE, DIALOGUE };

struct Point { int x, y; };

class Entity {
public:
    int x, y, color;
    char symbol;
    std::string name;
    
    Entity(int x, int y, char s, int c, std::string n = "") 
        : x(x), y(y), symbol(s), color(c), name(n) {}

    void draw() const {
        attron(COLOR_PAIR(color));
        mvaddch(y, x, symbol);
        attroff(COLOR_PAIR(color));
    }
};

class Map {
public:
    std::vector<std::string> data;
    int offset_y = 2;

    void draw() {
        attron(COLOR_PAIR(CP_WALL));
        for (int i = 0; i < data.size(); i++) {
            mvprintw(i + offset_y, 0, "%s", data[i].c_str());
        }
        attroff(COLOR_PAIR(CP_WALL));
    }

    char get_cell(int x, int y) {
        int ty = y - offset_y;
        if (ty < 0 || ty >= data.size() || x < 0 || x >= data[ty].size()) return '#';
        return data[ty][x];
    }
};

class GameEngine {
private:
    GameState state = MENU;
    std::unique_ptr<Entity> player;
    std::unique_ptr<Entity> npc;
    Map world_map;
    Map house_map;
    std::string current_dialogue = "";

public:
    GameEngine() {
        initscr(); cbreak(); noecho(); curs_set(0); keypad(stdscr, TRUE); start_color();
        init_pair(CP_PLAYER, COLOR_CYAN, COLOR_BLACK);
        init_pair(CP_NPC, COLOR_YELLOW, COLOR_BLACK);
        init_pair(CP_WALL, COLOR_WHITE, COLOR_BLACK);
        init_pair(CP_HOUSE, COLOR_GREEN, COLOR_BLACK);

        player = std::make_unique<Entity>(5, 5, '@', CP_PLAYER);
        npc = std::make_unique<Entity>(10, 4, 'M', CP_NPC, "Мудрец");

        world_map.data = {
            "####################",
            "#                  #",
            "#   [ H - Дом ]    #",
            "#      H           #",
            "####################"
        };
        house_map.data = {
            "  /----------\\  ",
            "  |          |  ",
            "  |   [X]    |  ",
            "  \\----------/  "
        };
    }

    ~GameEngine() { endwin(); }

    void run() {
        int ch;
        while ((ch = getch()) != 'q') {
            update(ch);
            render();
        }
    }

private:
    void update(int ch) {
        if (state == MENU) {
            if (ch == ' ') state = WORLD;
            return;
        }

        if (state == DIALOGUE) {
            if (ch == ' ' || ch == 27) state = HOUSE;
            return;
        }

        int nx = player->x, ny = player->y;
        if (ch == KEY_UP    || ch == 'w') ny--;
        if (ch == KEY_DOWN  || ch == 's') ny++;
        if (ch == KEY_LEFT  || ch == 'a') nx--;
        if (ch == KEY_RIGHT || ch == 'd') nx++;

        Map& current_map = (state == WORLD) ? world_map : house_map;
        char cell = current_map.get_cell(nx, ny);

        if (state == WORLD) {
            if (cell == 'H') {
                state = HOUSE; player->x = 5; player->y = 4;
            } else if (cell == ' ') {
                player->x = nx; player->y = ny;
            }
        } else if (state == HOUSE) {
            // Взаимодействие с NPC
            if (nx == npc->x && ny == npc->y) {
                state = DIALOGUE;
                current_dialogue = "Мудрец: Приветствую, странник! Ищи выход в символе [X].";
            } else if (cell == 'X') {
                state = WORLD; player->x = 7; player->y = 5;
            } else if (cell == ' ' || cell == '|') { // Разрешаем ходить внутри
                player->x = nx; player->y = ny;
            }
        }
    }

    void render() {
        erase();
        if (state == MENU) {
            mvprintw(10, 20, "--- СУПЕР КВЕСТ ---");
            mvprintw(12, 18, "Нажмите SPACE для начала");
        } 
        else if (state == WORLD) {
            mvprintw(0, 0, "Мир: Найдите дом (H)");
            world_map.draw();
            player->draw();
        } 
        else if (state == HOUSE) {
            mvprintw(0, 0, "Внутри дома. Подойдите к Мудрецу (M)");
            house_map.draw();
            npc->draw();
            player->draw();
        }
        else if (state == DIALOGUE) {
            house_map.draw();
            npc->draw();
            attron(A_BOLD);
            mvprintw(10, 5, "============================");
            mvprintw(11, 5, "%s", current_dialogue.c_str());
            mvprintw(12, 5, "   (Нажми Пробел, чтобы уйти)");
            mvprintw(13, 5, "============================");
            attroff(A_BOLD);
        }
        refresh();
    }
};

int main() {
    GameEngine game;
    game.run();
    return 0;
}
