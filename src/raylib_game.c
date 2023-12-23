/*******************************************************************************************
*
*   raylib gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for: 
#include <math.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    SCREEN_LOGO = 0, 
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

// TODO: Define your custom data types here
typedef struct {
    Vector2 position;
    Vector2 speed;
    float heading;
    float acceleration;
    int type;
    bool active;
} sEntity;

typedef enum {
    TYPE_METEOR_SMALL = 0,
    TYPE_METEOR_MED,
    TYPE_METEOR_LARGE,
    TYPE_PLAYER
} EntityType;

#define MAX_METEORS 50
#define MAX_TEXTURES 4
#define MAX_SHOTS 5
#define MAX_SOUNDS 2

typedef enum {
    TEXTURE_METEOR_SMALL = 0,
    TEXTURE_METEOR_MED,
    TEXTURE_METEOR_LARGE,
    TEXTURE_PLAYER
};

Texture2D textures[MAX_TEXTURES];

typedef enum {
    SOUND_LASER_SHOOT = 0,
    SOUND_EXPLOSION
};

Sound sounds[MAX_SOUNDS];

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static const int screenWidth = 1280;
static const int screenHeight = 720;

static RenderTexture2D target = { 0 };  // Render texture to render our game

// TODO: Define global variables here, recommended to make them static
sEntity sPlayer = { 0 };
sEntity sMeteors[MAX_METEORS];
sEntity sShots[MAX_SHOTS];
int currentMeteorCount = 0;
int currentScore = 0;
bool isGameOver = false;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
void GameStartup(void);
void GameUpdate(void);
void GameRender(void);
void GameShutdown(void);
void GameReset(void);


void GameStartup(void) {

    InitAudioDevice();

    Image image1 = LoadImage("resources/player.png");
    textures[TEXTURE_PLAYER] = LoadTextureFromImage(image1);
    UnloadImage(image1);

    Image image2 = LoadImage("resources/meteor_small.png");
    textures[TEXTURE_METEOR_SMALL] = LoadTextureFromImage(image2);
    UnloadImage(image2);

    Image image3 = LoadImage("resources/meteor_med.png");
    textures[TEXTURE_METEOR_MED] = LoadTextureFromImage(image3);
    UnloadImage(image3);

    Image image4 = LoadImage("resources/meteor_large.png");
    textures[TEXTURE_METEOR_LARGE] = LoadTextureFromImage(image4);
    UnloadImage(image4);

    sounds[SOUND_LASER_SHOOT] = LoadSound("resources/laser-shoot.wav");
    sounds[SOUND_EXPLOSION] = LoadSound("resources/laser-explosion.wav");



    GameReset();
}

void GameUpdate(void) {

    if (!isGameOver) {

        // player rotation
        if (IsKeyDown(KEY_LEFT)) sPlayer.heading -= 5.0f;
        if (IsKeyDown(KEY_RIGHT)) sPlayer.heading += 5.0f;

        // player speed
        sPlayer.speed.x = cosf(sPlayer.heading * DEG2RAD) * 6.0f;
        sPlayer.speed.y = sinf(sPlayer.heading * DEG2RAD) * 6.0f;

        // player acceleration
        if (IsKeyDown(KEY_UP)) {
            if (sPlayer.acceleration < 1.0f) sPlayer.acceleration += 0.04f;

        }

        // player movement
        sPlayer.position.x += (sPlayer.speed.x * sPlayer.acceleration);
        sPlayer.position.y += (sPlayer.speed.y * sPlayer.acceleration);

        if (sPlayer.position.x > screenWidth) {
            sPlayer.position.x = 0;
        }
        else if (sPlayer.position.x < 0) {
            sPlayer.position.x = screenWidth;
        }

        if (sPlayer.position.y > screenHeight) {
            sPlayer.position.y = 0;
        }
        else if (sPlayer.position.y < 0) {
            sPlayer.position.y = screenHeight;
        }

        // spawn shots
        if (IsKeyPressed(KEY_LEFT_CONTROL)) {
            for (int i = 0; i < MAX_SHOTS; i++) {
                if (!sShots[i].active) {
                    sShots[i].active = true;
                    sShots[i].position = sPlayer.position;
                    sShots[i].heading = sPlayer.heading;
                    sShots[i].acceleration = 1.0f;
                    sShots[i].speed.x = cosf(sPlayer.heading * DEG2RAD) * 10.0f;
                    sShots[i].speed.y = sinf(sPlayer.heading * DEG2RAD) * 10.0f;

                    PlaySound(sounds[SOUND_LASER_SHOOT]);
                    break;
                }
            }
        }

        // Update Meteors
        for (int i = 0; i < MAX_METEORS; i++) {
            if (sMeteors[i].active) {
                sMeteors[i].position.x += sMeteors[i].speed.x * cosf(sMeteors[i].heading * DEG2RAD);
                sMeteors[i].position.y += sMeteors[i].speed.y * sinf(sMeteors[i].heading * DEG2RAD);

                if (sMeteors[i].position.x > screenWidth) {
                    sMeteors[i].position.x = 0;
                }
                else if (sMeteors[i].position.x < 0) {
                    sMeteors[i].position.x = screenWidth;
                }

                if (sMeteors[i].position.y > screenHeight) {
                    sMeteors[i].position.y = 0;
                }
                else if (sMeteors[i].position.y < 0) {
                    sMeteors[i].position.y = screenHeight;
                }
            }
        }

        // update shots
        for (int i = 0; i < MAX_SHOTS; i++) {
            if (sShots[i].active) {
                sShots[i].position.x += (sShots[i].speed.x * sShots[i].acceleration);
                sShots[i].position.y += (sShots[i].speed.y * sShots[i].acceleration);

                if (sShots[i].position.x > screenWidth || sShots[i].position.x < 0) {
                    sShots[i].active = false;
                }

                if (sShots[i].position.y > screenHeight || sShots[i].position.y < 0) {
                    sShots[i].active = false;
                }
            }
        }

        // collision detection between laser and meteor
        for (int i = 0; i < MAX_SHOTS; i++) {
            if (sShots[i].active) {
                for (int j = 0; j < MAX_METEORS; j++) {
                    if (sMeteors[j].active) {
                        float texWidth = textures[sMeteors[j].type].width;
                        if (CheckCollisionCircles(sShots[i].position, 1, sMeteors[j].position, texWidth / 2)) {
                            // collision!
                            sMeteors[j].active = false;
                            sShots[i].active = false;

                            currentMeteorCount--;
                            currentScore += 10;

                            PlaySound(sounds[SOUND_EXPLOSION]);

                            break;
                        }
                    }
                }
            }
        }
    }

    // detect a game over condition
    if (currentMeteorCount < 1) {
        isGameOver = true;
    }

}

void GameRender(void) {

    // draw the meteors
    for (int i = 0; i < MAX_METEORS; i++) {
        if (sMeteors[i].active) {
            DrawTexturePro(textures[sMeteors[i].type],
                (Rectangle) {
                0, 0, textures[sMeteors[i].type].width, textures[sMeteors[i].type].height
            },
                (Rectangle) {
                sMeteors[i].position.x, sMeteors[i].position.y, textures[sMeteors[i].type].width, textures[sMeteors[i].type].height
            },
                (Vector2) {
                textures[sMeteors[i].type].width / 2, textures[sMeteors[i].type].height / 2
            },
                sMeteors[i].heading,
                RAYWHITE);
        }
    }

    // draw the shots
    for (int i = 0; i < MAX_SHOTS; i++) {
        if (sShots[i].active) {
            DrawCircle(sShots[i].position.x, sShots[i].position.y, 1, YELLOW);
        }
    }

    // draw the player
    DrawTexturePro(textures[TEXTURE_PLAYER],
        (Rectangle) {
        0, 0, textures[TEXTURE_PLAYER].width, textures[TEXTURE_PLAYER].height
    },
        (Rectangle) {
        sPlayer.position.x, sPlayer.position.y, textures[TEXTURE_PLAYER].width / 2, textures[TEXTURE_PLAYER].height / 2
    },
        (Vector2) {
        textures[TEXTURE_PLAYER].width / 4, textures[TEXTURE_PLAYER].height / 4
    },
        sPlayer.heading,
        RAYWHITE);

    DrawCircle(sPlayer.position.x, sPlayer.position.y, 5.0f, GREEN);

    // draw a basic UI
    DrawRectangle(5, 5, 250, 100, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(5, 5, 250, 100, BLUE);

    DrawText(TextFormat("- Player Position: (%06.1f, %06.1f)", sPlayer.position.x, sPlayer.position.y), 15, 30, 10, YELLOW);
    DrawText(TextFormat("- Player Heading: %06.1f", sPlayer.heading), 15, 45, 10, YELLOW);
    DrawText(TextFormat("- Current Meteor Count: %i", currentMeteorCount), 15, 60, 10, YELLOW);
    DrawText(TextFormat("- Player Score: %i", currentScore), 15, 75, 10, YELLOW);

    if (isGameOver) {
        DrawText("GAME OVER", screenWidth / 2, screenHeight / 2, 40, YELLOW);
    }
}

void GameShutdown(void) {

    for (int i = 0; i < MAX_TEXTURES; i++) {
        UnloadTexture(textures[i]);
    }

    for (int i = 0; i < MAX_SOUNDS; i++) {
        UnloadSound(sounds[i]);
    }

    CloseAudioDevice();

}

void GameReset(void) {

    sPlayer.position = (Vector2){ screenWidth / 2, screenHeight / 2 };
    sPlayer.speed = (Vector2){ 0, 0 };
    sPlayer.heading = 0;
    sPlayer.acceleration = 0;
    sPlayer.type = TYPE_PLAYER;
    sPlayer.active = true;

    for (int i = 0; i < MAX_METEORS; i++) {
        sMeteors[i].active = true;
        sMeteors[i].heading = (float)GetRandomValue(0, 360);
        sMeteors[i].position = (Vector2){ (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
        sMeteors[i].type = GetRandomValue(TYPE_METEOR_SMALL, TYPE_METEOR_LARGE);
        sMeteors[i].speed = (Vector2){ (float)GetRandomValue(1, 2), (float)GetRandomValue(1,2) };
    }

    for (int i = 0; i < MAX_SHOTS; i++) {
        sShots[i].active = false;
    }

    currentMeteorCount = MAX_METEORS;
    isGameOver = false;
    currentScore = 0;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messsages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");
    
    // TODO: Load resources / Initialize variables at this point
    GameStartup();
    
    // Render texture to draw full screen, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    
    // TODO: Unload all loaded resources at this point
    GameShutdown();

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module functions definition
//--------------------------------------------------------------------------------------------
// Update and draw frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    //----------------------------------------------------------------------------------
    GameUpdate();

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture, 
    // it could be useful for scaling or further sahder postprocessing
    BeginTextureMode(target);
        ClearBackground(BLACK);
        
        // TODO: Draw your game screen here
        GameRender();
        // DrawRectangle(10, 10, screenWidth - 20, screenHeight - 20, SKYBLUE);
        
    EndTextureMode();
    
    // Render to screen (main framebuffer)
    BeginDrawing();
        ClearBackground(BLACK);
        
        // Draw render texture to screen, scaled if required
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, (Rectangle){ 0, 0, (float)target.texture.width, (float)target.texture.height }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
    //----------------------------------------------------------------------------------  
}