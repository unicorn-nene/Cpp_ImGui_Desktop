#include <iostream>
#include <chrono>
#include <cstdio>
#include <Windows.h>
#include <utility>
#include <algorithm>
#include <vector>
#include <cmath>

int nScreenWidth = 120;
int nScreenHeight = 40;

// player state
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

float fFOV = 3.14159f / 4.0f;
// map information
int nMapWidth = 16;
int nMapHeight = 16;

// some parametre
float fDepth = 16.0f;

using namespace std;

int main()
{
    ////创建屏幕缓冲区
    // wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
    ////创建并设置控制台缓冲区
    // HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    ////设置为当前缓冲区,使其显示在当前窗口上
    // SetConsoleActiveScreenBuffer(hConsole);
    // DWORD dwBytesWritten = 0;

    wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // map - a 1d array
    wstring map;
    map += L"#########......#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#.......##.....#";
    map += L"#.......##.....#";
    map += L"#..............#";
    map += L"######.........#";
    map += L"######.........#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#........#######";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    while (1) // game main loop
    {

        // 计算时间,帧率平滑加载图形
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count(); // count()函数计算出elapsedTime中多少秒

        // 设置按键输入,移动与旋转
        // 旋转
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (0.8f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (0.8f) * fElapsedTime;

        // 前后移动
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            // 碰撞检测
            if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                // 撤销移动
                fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
            fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
            // 碰撞检测
            if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                // 撤销移动
                fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
                fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
            }
        }

        // 绘制屏幕,首先从屏幕左侧向右循环(左上角(0,0)位置开始)
        for (int x = 0; x < nScreenWidth; ++x)
        {
            // for each column, calculate the projected ray angle to world space
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            float fStepSize = 0.1f;
            float fDistanceToWall = 0.0f;

            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);

            while (!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += fStepSize;
                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) // 测试视线距离 超出地图边界
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    if (map.c_str()[nTestX * nMapWidth + nTestY] == '#') // 测试视线到达墙壁
                    {
                        bHitWall = true;
                        // 视线抵达墙壁,继续判断视线是否接近墙块的角落

                        vector<pair<float, float>> p;

                        // Test each corner of hit tile, storing the distance from
                        // the player, and the calculated dot product of the two rays
                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                // Angle of corner to eye
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }

                        // Sort Pairs from closest to farthest
                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right)
                             { return left.first < right.first; });

                        // First two/three are closest (we will never see all four)
                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound)
                            bBoundary = true;
                        if (acos(p.at(1).second) < fBound)
                            bBoundary = true;
                        if (acos(p.at(2).second) < fBound)
                            bBoundary = true;
                    }
                }
            }
            // calculate distance to ceiling and floor
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' ';
            // 设置随距离不同变化墙壁色彩阴影
            if (fDistanceToWall <= fDepth / 4.0f)
                nShade = 0x2588; // very close
            else if (fDistanceToWall < fDepth / 3.0f)
                nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f)
                nShade = 0x2592;
            else if (fDistanceToWall < fDepth)
                nShade = 0x2591;
            else
                nShade = ' ';

            if (bBoundary)
                nShade = ' '; // Black it out

            for (int y = 0; y < nScreenHeight; ++y) // 竖向绘制屏幕缓冲区,Y轴从左上角开始向下增加
            {
                if (y <= nCeiling)
                    screen[y * nScreenWidth + x] = ' ';
                else if (y > nCeiling && y <= nFloor)
                    screen[y * nScreenWidth + x] = nShade;
                else
                {
                    // Shade floor based on distance
                    //	b越大 -> 绘制地面更靠近屏幕中心 -> 距离玩家越远 -> 视觉效果更浅
                    //	b越小 -> 绘制地面更靠近屏幕下方 -> 距离玩家越近 -> 视觉效果更深
                    float b = 1.0f - (((float)y - nScreenHeight / 2.0) / ((float)nScreenHeight / 2.0));
                    if (b < 0.25) // closest
                        nShade = '#';
                    else if (b < 0.5)
                        nShade = 'X';
                    else if (b < 0.75)
                        nShade = '.';
                    else if (b < 0.9)
                        nShade = '_'; // far away
                    else
                        nShade = ' ';
                    // 绘制地板
                    screen[y * nScreenWidth + x] = nShade;
                }
            }
        }
        // display stats
        swprintf_s(screen, 40, L"X=%3.2f,Y=%3.2f,A=%3.2f,FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

        // display map
        for (int nx = 0; nx < nMapWidth; ++nx)
            for (int ny = 0; ny < nMapHeight; ++ny)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }
        // dispaly player position
        screen[(int)(fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';
        // 绘制并输出屏幕缓冲区
        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
    } // end while loop of game main loop

    return 0;
}