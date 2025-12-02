#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

struct point {
    int x;
    int y;
};

struct snake {
    int x;
    int y;
    struct snake *next;
};

int snake_len = 2;

void add_point(struct snake *s)
{
    int i = 0;
    struct snake *tmp = s;
    int x_tmp;
    int y_tmp;
    while (i < snake_len - 2)
    {
        i++;
        tmp = tmp->next;
    }
    x_tmp = tmp->x;
    y_tmp = tmp->y;
    tmp = tmp->next;
    struct snake *new = malloc(sizeof(struct snake));
    if(x_tmp == tmp->x)
    {
        new->x = tmp->x;
        new->y = tmp->y + 1;
    }
    else 
    {
        new->x = tmp->x + 1;
        new->y = tmp->y;
    }
    new->next = NULL;
    tmp->next = new;
    snake_len++;
}

struct snake* init_snake(int max_x, int max_y)
{
    struct snake *head = malloc(sizeof(struct snake));
    head->x = max_x / 2;
    head->y = max_y / 2;
    struct snake *tail = malloc(sizeof(struct snake));
    tail->x = max_x / 2 + 1;
    tail->y = max_y / 2;
    head->next = tail;
    tail->next = NULL;
    add_point(head);
    add_point(head);
    return head;
}

void free_snake(struct snake *s)
{
    struct snake *tmp = s;
    while (s != NULL)
    {
        tmp = s->next;
        free(s);
        s = tmp;
    }
}

void print_snake(struct snake *s)
{
    struct snake *tmp = s;
    while(tmp != NULL)
    {
        //printf("x : %zu, y : %zu \n", tmp->x, tmp->y);
        mvaddch(tmp->y, tmp->x, '@');
        tmp = tmp->next;
    }
}

void move_snake(struct snake *s, char ch)
{
    struct snake *tmp = s;
    int last_x = tmp->x;
    int last_y = tmp->y;
    mvaddch(tmp->y, tmp->x, ' ');
    if (ch == 3 && tmp->next->y >= tmp->y)
    {
        tmp->y -= 1;
        ch = ' ';
    }
    else if (ch == 5 && tmp->next->x <= tmp->x)
    {
        tmp-> x += 1;
        ch = ' ';
    }
    else if (ch == 2 && tmp->next->y <= tmp->y)
    {
        tmp->y += 1;
        ch = ' ';
    }
    else if (ch == 4 && tmp->next->x >= tmp->x)
    {
        tmp->x -= 1;
        ch = ' ';
    }
    else if(tmp->next->x == tmp->x)
    {
        tmp->y = tmp->y + (tmp->y - tmp->next->y);
    }
    else 
    {
        tmp->x = tmp->x + (tmp->x - tmp->next->x);
    }
    mvaddch(tmp->y, tmp->x, '@');
    tmp = tmp->next;
    while (tmp->next != NULL)
    {
        mvaddch(tmp->y, tmp->x, ' ');
        int tx = tmp->x;
        int ty = tmp->y;
        tmp->x = last_x;
        tmp->y = last_y;
        last_x = tx;
        last_y = ty;
        mvaddch(tmp->y, tmp->x, '@');
        tmp = tmp->next;
    }
    mvaddch(tmp->y, tmp->x, ' ');
    tmp->x = last_x;
    tmp->y = last_y;
    mvaddch(tmp->y, tmp->x, '@');
    //mvprintw(10,8,"%d", s->x);
    //mvprintw(11,8,"%d", s->y);
}

bool inside_snake(struct snake *s, int x, int y)
{
    struct snake *tmp = s;
    while (tmp != NULL)
    {
        if (x == tmp->x && y == tmp->y && s != tmp)
            return true;
        tmp = tmp->next;
    }
    return false;
}

void spawn_food(int max_x, int max_y, struct snake *s, struct point *p)
{
    int y;
    int x;
    do {
        y = (rand() % (max_y) + 1);
        x = (rand() % (max_x) + 1);
    } while (inside_snake(s, x, y));
    mvaddch(y, x, 'O');
    p->x = x;
    p->y = y;
}

void eat_food(int max_x, int max_y,struct snake *s)
{
    mvaddch(s->x, s->y, ' ');
    add_point(s);
}

bool check_state(int max_x, int max_y,struct snake *s)
{
    if (inside_snake(s, s->x, s->y))
        return true;
    if (s->x <= 0 || s->y <= 0 || s->x > max_x || s->y > max_y)
        return true;
    return false;
}

int main()
{
    initscr();
    nodelay(stdscr, TRUE);
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    int max_y_map;
    int max_x_map;
    getmaxyx(stdscr, max_y_map, max_x_map);
    max_x_map -= 2;
    max_y_map -= 2;
    int max_snake = max_y_map * max_x_map;
    struct point *current_food = malloc(sizeof(struct point));
    erase();
    box(stdscr, 0, 0);
    mvprintw(0, 3, " Snake ");
    mvprintw(0, 15, " Score :  ");
    mvprintw(0, 24, "%d", 0);
    mvprintw(0, 25, "%s", " ");
    int score = 0;
    struct snake *snake = init_snake(max_x_map, max_y_map);
    print_snake(snake);
    spawn_food(max_x_map, max_y_map, snake, current_food);
    refresh();
    bool game_over = false;
    int height = LINES / 2;
    int width = COLS / 2;
    int size = (height < width) ? height : width;

    int tick = 3500 / (size + score);
    if (tick < 50) tick = 50;
    while (!game_over)
    {
        tick = 3500 / (size + score);
        int ch;
        int last = -1;
        while ((ch = getch()) != ERR) {
            last = ch;
        }
        ch = last;
        if (ch == 'q' || ch == 'Q')
            break;
        if (ch == ' ')
            tick /= 5;
        move_snake(snake, ch);
        game_over = check_state(max_x_map, max_y_map, snake);
        if (snake->x == current_food->x && snake->y == current_food->y)
        {
            eat_food(max_x_map, max_y_map, snake);
            spawn_food(max_x_map, max_y_map, snake, current_food);
            score++;
            mvprintw(0, 24, "%d", score);
        }
        refresh();
        napms(tick);
    }
    clear();
    mvprintw(max_y_map/2 - 2, (max_x_map - 21) / 2, " GAME OVER ");
    mvprintw(max_y_map/2, (max_x_map - 20) / 2, " Score: %d ", score);
    mvprintw(max_y_map/2 + 2, (max_x_map - 30) / 2, " Press any key to exit ");
    nodelay(stdscr, FALSE);
    getch();
    endwin();
    free_snake(snake);
    free(current_food);
    return 0;
}