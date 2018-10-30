
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#define PLAYER_X_VEL     200
#define PLAYER_X_ACC     500
#define PLAYER_X_DEC     1000
#define PLAYER_JUMP_VEL  400
#define GRAVITY          1000
#define SIZE_X           20
#define SIZE_Y           15


typedef struct Player
{
    SDL_Rect rect;
    float vx;
    float vy;
    float ax;
    float dx;
    bool isOnGround;
} Player;

typedef struct Tile
{
    SDL_Rect rect;
    bool isSolid;
} Tile;

SDL_Renderer* renderer = NULL;

Player player;
Tile map[SIZE_X][SIZE_Y];


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
    player.rect.x = 10 * 32;
    player.rect.y = SIZE_Y * 32 - 32 * 2 + 10;
    player.rect.w = 32-4;
    player.rect.h = 32-4;
    player.vx = 0;
    player.vy = 0;
    player.ax = 0;
    player.dx = 0;
    player.isOnGround = false;
}

void UpdatePlayerPhysics(float dt)
{
    // Compute player physics
    player.vx += player.ax * dt;
    if (player.vx > PLAYER_X_VEL) {
        player.vx = PLAYER_X_VEL;
    }
    if (player.vx < -PLAYER_X_VEL) {
        player.vx = -PLAYER_X_VEL;
    }
    player.rect.x += player.vx * dt;
    float gravity = GRAVITY;
    player.vy += gravity * dt;
    player.rect.y += player.vy * dt;
}

void UpdatePlayerMapCollisions()
{
    // Resolve player collisions with the map
    int mapX = player.rect.x / 32;
    int mapY = player.rect.y / 32;
    Tile* tilesAroundPlayer[9];
    for (int i=0;i<9;i++) {
        int c = i % 3;
        int r = (int)(i / 3);
        int tileX = mapX + c - 1;
        int tileY = mapY + r - 1;
        if (tileX >= 0 && tileY >=0 &&
            tileX <= SIZE_X && tileY <= SIZE_Y) {
            tilesAroundPlayer[i] = &map[tileX][tileY];
        }
    }
    Tile* sorted[9];
    sorted[0] = tilesAroundPlayer[0];
    sorted[1] = tilesAroundPlayer[7];
    sorted[2] = tilesAroundPlayer[2];
    sorted[3] = tilesAroundPlayer[4];
    sorted[4] = tilesAroundPlayer[5];
    sorted[5] = tilesAroundPlayer[1];
    sorted[6] = tilesAroundPlayer[3];
    sorted[7] = tilesAroundPlayer[6];
    sorted[8] = tilesAroundPlayer[8];
    for (int i=0;i<9;i++) {
        if (sorted[i]->isSolid) {
            DrawDebugRect(&sorted[i]->rect, 0, 0, 255, 255);
            if (Collides(&player.rect, &sorted[i]->rect)) {
                DrawDebugRect(&sorted[i]->rect, 255, 0, 0, 255);

                // Calculate player-tile overlap in x and y
                int xOverlap = 0;
                int yOverlap = 0;
                if (player.rect.x < sorted[i]->rect.x) {
                    xOverlap = player.rect.x + player.rect.w - sorted[i]->rect.x;
                } else {
                    xOverlap = sorted[i]->rect.x + sorted[i]->rect.w - player.rect.x;
                }
                if (player.rect.y < sorted[i]->rect.y) {
                    yOverlap = player.rect.y + player.rect.h - sorted[i]->rect.y;
                } else {
                    yOverlap = sorted[i]->rect.y + sorted[i]->rect.h - player.rect.y;
                }

                if (i == 1) {
                    // Tile directly below player
                    player.rect.y -= yOverlap;
                    player.vy = 0;
                    player.isOnGround = true;
                } else if (i == 2) {
                    // Tile directly above player
                    player.rect.y += yOverlap;
                    player.vy = 0;
                    printf("kasdfasdf\n");
                } else if (i == 3) {
                    // Tile directly to left of player
                    player.rect.x += xOverlap;
                } else if (i == 4) {
                    // Tile directly to right of player
                    player.rect.x -= xOverlap;
                } else {
                    // Deal with diagonals
                    if (xOverlap > yOverlap) {
                        int height;
                        if (i == 5) {
                            height = yOverlap;
                            player.isOnGround = true;
                        } else {
                            height = -yOverlap;
                        }
                        player.rect.y += height;
                        player.vy = 0;
                    } else {
                        int width;
                        if (i == 6 || i == 4) {
                            width = xOverlap;
                        } else {
                            width = -xOverlap;
                        }
                        player.rect.x += width;
                    }
                }
            }
        }
    }
}

void InitMap()
{
    memset(&map, 0, sizeof(map));
    for (int i=0;i<SIZE_X;i++) {
        map[i][SIZE_Y-1].rect.x = i * 32;
        map[i][SIZE_Y-1].rect.y = (SIZE_Y-1) * 32;
        map[i][SIZE_Y-1].rect.w = 32;
        map[i][SIZE_Y-1].rect.h = 32;
        map[i][SIZE_Y-1].isSolid = true;
    }
    for (int i=0;i<SIZE_X;i++) {
        map[i][SIZE_Y-3].rect.x = i * 32;
        map[i][SIZE_Y-3].rect.y = (SIZE_Y-3) * 32;
        map[i][SIZE_Y-3].rect.w = 32;
        map[i][SIZE_Y-3].rect.h = 32;
        map[i][SIZE_Y-3].isSolid = true;
    }
    map[15][SIZE_Y-2].rect.x = 15 * 32;
    map[15][SIZE_Y-2].rect.y = (SIZE_Y-2) * 32;
    map[15][SIZE_Y-2].rect.w = 32;
    map[15][SIZE_Y-2].rect.h = 32;
    map[15][SIZE_Y-2].isSolid = true;

    map[5][SIZE_Y-2].rect.x = 5 * 32;
    map[5][SIZE_Y-2].rect.y = (SIZE_Y-2) * 32;
    map[5][SIZE_Y-2].rect.w = 32;
    map[5][SIZE_Y-2].rect.h = 32;
    map[5][SIZE_Y-2].isSolid = true;
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

    bool run = true;
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
                            player.ax = -PLAYER_X_ACC;
                            break;
                        case SDLK_RIGHT:
                            player.ax = PLAYER_X_ACC;
                            break;
                        case SDLK_SPACE:
                            if (player.isOnGround) {
                                player.vy = -PLAYER_JUMP_VEL; 
                                player.isOnGround = false;
                            }
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_LEFT:
                            if (player.vx < 0) {
                                player.vx = 0;
                                player.ax = 0;
                            }
                            break;
                        case SDLK_RIGHT:
                            if (player.vx > 0) {
                                player.vx = 0;    
                                player.ax = 0;
                            }
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
            UpdatePlayerMapCollisions();

            DrawDebugRect(&player.rect, 0, 255, 0, 255);

            SDL_RenderPresent(renderer);
        }
    }

    SDL_Quit();

    return 0;
}
