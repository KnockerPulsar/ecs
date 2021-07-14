#include "Includes.h"

void MainGame(std::vector<Object *> &objects)
{
    ClearBackground(BLACK);
    for (int i = 0; i < (int)objects.size(); i++)
    {
        objects[i]->Update(objects);
    }
}
void Menu(int &gameState)
{
    static float countDown = 3.25;
    // static float aspectInverse = (float)GetScreenHeight() / GetScreenWidth();
    int fontSize = 60;
    if (GetKeyPressed() == KEY_ENTER || countDown < 3.25)
    {
        countDown -= GetFrameTime();
        ClearBackground(BLACK);

        if (Between(2.25, countDown, 3.25))
        {
            DrawText("3", GetScreenWidth() / 2 - fontSize, GetScreenHeight() / 2 - fontSize, fontSize * 4, RED);
        }
        else if (Between(1.25, countDown, 2.25))
        {
            DrawText("2", GetScreenWidth() / 2 - fontSize, GetScreenHeight() / 2 - fontSize, fontSize * 4, BLUE);
        }
        else if (Between(0.25, countDown, 1.25))
        {
            DrawText("1", GetScreenWidth() / 2 - fontSize, GetScreenHeight() / 2 - fontSize, fontSize * 4, RED);
        }
        else if (Between(0, countDown, 0.25))
        {
            char *psyched = "GET PSYCHED";
            // MAGIC NUMBERS BABYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
            int xPos = GetScreenWidth() / 2 - TextLength(psyched) * fontSize * 0.66;
            DrawText(psyched, xPos, GetScreenHeight() / 2 - fontSize , fontSize * 2, GRAY);
        }
        else if (countDown < 0)
            gameState = 1;
    }
    else
    {
        ClearBackground(BLACK);
        char *message = "Press Enter to Play, Escape to quit";
        int xPos = (GetScreenWidth() - TextLength(message) * fontSize / 1.85) / 2;
        DrawText(message, xPos, GetScreenHeight() / 2, fontSize, BLUE);
    }
}
