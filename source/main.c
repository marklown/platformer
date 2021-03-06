
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

static const float PLAYER_X_VEL = 200;
static const float PLAYER_X_ACC = 500;
static const float PLAYER_X_DEC = 600;
static const float PLAYER_JUMP_VEL = 450;
static const float GRAVITY = 1000;

static const Uint8 SIZE_X = 20;
static const Uint8 SIZE_Y = 15;

static const Uint8 DOWN = 0;
static const Uint8 UP = 1;
static const Uint8 LEFT = 2;
static const Uint8 RIGHT = 3;
static const Uint8 UP_LEFT = 4;
static const Uint8 UP_RIGHT = 5;
static const Uint8 DOWN_LEFT = 6;
static const Uint8 DOWN_RIGHT = 7;
static const Uint8 NONE = 255;

typedef struct Player
{
    SDL_Rect rect;
    float vx;
    float vy;
    float ax;
    bool isOnGround;
    bool canJumpAgain;
    Uint8 direction;
} Player;

typedef struct Tile
{
    SDL_Rect rect;
    bool isSolid;
    void(*onCollision)(void);
} Tile;

SDL_Renderer* renderer = NULL;

Player player;
Tile map[SIZE_X][SIZE_Y];

static FILE* mapFile = NULL;
static bool run = true;

void DrawDebugRect(const SDL_Rect* rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawRect(renderer, rect);
}

void DrawDebugMap()
{
    for (int i=0;i<SIZE_X;i++) {
        for (int j=0;j<SIZE_Y;j++) {
            DrawDebugRect(&map[i][j].rect, 128, 128, 128, 255);
        }
    }
}

bool Collides(const SDL_Rect* a, const SDL_Rect* b)
{
    if (a->x < b->x + b->w &&
        a->x + a->w > b->x &&
        a->y < b->y + b->h &&
        a->y + a->h > b->y) {
        return true;
    }
    return false;
}

void InitPlayer()
{
    memset(&player, 0, sizeof(Player));
    player.rect.x = 0;
    player.rect.y = 0;
    player.rect.w = 28;
    player.rect.h = 28;
    player.vx = 0;
    player.vy = 0;
    player.ax = 0;
    player.isOnGround = false;
    player.canJumpAgain = true;
    player.direction = DOWN;
}

void ResolveXCollisions()
{
    int mapX = (player.rect.x + 16) / 32;
    int mapY = (player.rect.y + 16) / 32;
    Tile* sorted[8];
    sorted[DOWN] = &map[mapX][mapY+1];
    sorted[UP] = &map[mapX][mapY-1];
    sorted[LEFT] = &map[mapX-1][mapY];
    sorted[RIGHT] = &map[mapX+1][mapY];
    sorted[UP_LEFT] = &map[mapX-1][mapY-1];
    sorted[UP_RIGHT] = &map[mapX+1][mapY-1];
    sorted[DOWN_LEFT] = &map[mapX-1][mapY+1];
    sorted[DOWN_RIGHT] = &map[mapX+1][mapY+1];
    for (int i=0;i<8;i++) {
        if (sorted[i]->isSolid) {
            if (Collides(&player.rect, &sorted[i]->rect)) {
                DrawDebugRect(&sorted[i]->rect, 255, 0, 0, 255);
                if (player.vx > 0) {
                    player.rect.x = sorted[i]->rect.x - player.rect.w;
                } else if (player.vx < 0) {
                    player.rect.x = sorted[i]->rect.x + sorted[i]->rect.w;
                }
                player.vx = 0;
            }
        }
    }
}

void ResolveYCollisions()
{
    int mapX = (player.rect.x + 16) / 32;
    int mapY = (player.rect.y + 16) / 32;
    Tile* sorted[8];
    sorted[DOWN] = &map[mapX][mapY+1];
    sorted[UP] = &map[mapX][mapY-1];
    sorted[LEFT] = &map[mapX-1][mapY];
    sorted[RIGHT] = &map[mapX+1][mapY];
    sorted[UP_LEFT] = &map[mapX-1][mapY-1];
    sorted[UP_RIGHT] = &map[mapX+1][mapY-1];
    sorted[DOWN_LEFT] = &map[mapX-1][mapY+1];
    sorted[DOWN_RIGHT] = &map[mapX+1][mapY+1];
    for (int i=0;i<8;i++) {
        if (sorted[i]->isSolid) {
            if (Collides(&player.rect, &sorted[i]->rect)) {
                DrawDebugRect(&sorted[i]->rect, 255, 0, 0, 255);
                if (player.vy > 0) {
                    player.rect.y = sorted[i]->rect.y - player.rect.h;
                    player.vy = 0;
                    player.isOnGround = true;
                } else if (player.vy < 0) {
                    player.rect.y = sorted[i]->rect.y + sorted[i]->rect.h;
                    player.vy = 0;
                }
            }
        }
    }

}

void UpdatePlayerPhysics(float dt)
{
    player.vx += player.ax * dt;

    if (player.vx > PLAYER_X_VEL) {
        player.vx = PLAYER_X_VEL;
    } else if (player.vx < -PLAYER_X_VEL) {
        player.vx = -PLAYER_X_VEL;
    }

    if (player.direction == RIGHT && player.ax < 0 && player.vx <= 0) {
        player.vx = 0;
        player.ax = 0;
        player.direction = NONE;
    } else if (player.direction == LEFT && player.ax > 0 && player.vx >= 0) {
        player.vx = 0;
        player.ax = 0;
        player.direction = NONE;
    }

    player.rect.x += player.vx * dt;

    ResolveXCollisions();

    float gravity = GRAVITY;
    player.vy += gravity * dt;
    player.rect.y += player.vy * dt;

    ResolveYCollisions();


    if (player.rect.y >= SIZE_Y * 32) {
        run = false;        
        printf("You lost\n");
    }
}

void AddCollisionTileAt(Uint32 x, Uint32 y)
{
    if (x > SIZE_X - 1 || y > SIZE_Y - 1) {
        return;
    }
    map[x][y].rect.x = x * 32;
    map[x][y].rect.y = y * 32;
    map[x][y].rect.w = 32;
    map[x][y].rect.h = 32;
    map[x][y].isSolid = true;
}

char* ReadMapFile()
{
    char* data = malloc(1000);
    mapFile = fopen("/Users/markl/dev/platformer/data/map1.txt", "r");
    int count = 0;
    char ch = 0;
    for (int i=0;i<1000;i++) {
        char ch = fgetc(mapFile);
        data[count++] = ch;
        if (ch == 'e') break;
    }
    data[count] = '\0';
    return data;
}

void InitMap()
{
    memset(&map, 0, sizeof(map));
    char* mapData = ReadMapFile();
    int row = 0;
    int col = 0;
    for (int i=0;i<1000;i++) {
        if (mapData[i] == 'e') {
            break;
        } else if (mapData[i] == '\n') {
            col = 0;
            row++;
        } else if (mapData[i] == 'a') {
            col++;
        } else if (mapData[i] == 'b') {
            AddCollisionTileAt(col++, row);
        }
    }
    fclose(mapFile);
    free(mapData);
}

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Game", 0, 0, 640, 480, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Failed to create SDL Window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Failed to create SDL Renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int flags = IMG_INIT_PNG;
    if (!(IMG_Init(flags) & flags)) {
        printf("Failed to initialize SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Event event;

    InitPlayer();
    InitMap();

    Uint32 currentTime = SDL_GetTicks();
    Uint32 lastTime;

    while (run) {

        lastTime = currentTime;
        currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) * 0.001f;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    run = false;
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:
                            if (player.direction == RIGHT) {
                                player.vx = 0;
                            }
                            player.ax = -PLAYER_X_ACC;
                            player.direction = LEFT;
                            break;
                        case SDLK_RIGHT:
                            if (player.direction == LEFT) {
                                player.vx = 0;
                            }
                            player.ax = PLAYER_X_ACC;
                            player.direction = RIGHT;
                            break;
                        case SDLK_SPACE:
                            if (player.isOnGround && player.canJumpAgain) {
                                player.vy = -PLAYER_JUMP_VEL;
                                player.isOnGround = false;
                                player.canJumpAgain = false;
                            }
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:
                            if (player.direction == LEFT) {
                                player.ax = PLAYER_X_DEC;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (player.direction == RIGHT) {
                                player.ax = -PLAYER_X_DEC;
                            }
                            break;
                        case SDLK_SPACE:
                            player.canJumpAgain = true;
                            break;
                    }
                    break;
            }
        }

        if (run) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            DrawDebugMap();

            UpdatePlayerPhysics(dt);

            DrawDebugRect(&player.rect, 0, 255, 0, 255);

            SDL_RenderPresent(renderer);
        }
    }

    SDL_Quit();

    return 0;
}
