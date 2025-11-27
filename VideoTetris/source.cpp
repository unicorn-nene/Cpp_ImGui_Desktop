#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>

using namespace std;

wstring tetromino[7]{};
int nFieldWidth{12};
int nFieldHeight{18};
unsigned char *pField = nullptr; // 指向游戏区域的二维数组的指针

int nScreenWidth{80};  // Console Screen Size X (columns)
int nScreenHeight{30}; // Console Screen Size Y (rows)

/**
 * @brief 旋转包含 方块 的 4*4矩阵(二维数组一维表示)
 *
 * @param px primary x
 * @param py primary y
 * @param r 当前旋转角度(0, 1, 2, 3) * 90
 * @param width 二维数组列数 = 4
 * @param height 二维数组行数 = 4
 * @return int 返回当前二维数组对应位置的索引
 */
int Rotate(int px, int py, int &r, int row = 4, int column = 4)
{
    if (r == -1)
        r = 3; //-90 = 270

    switch (r % 4)
    {
    case 0:
        return py * column + px;
    case 1:
        return 12 + py - (px * row);
    case 2:
        return 15 - (py * column) - px;
    case 3: // 270
        return 3 - py + (px * row);
    }
    return 0;
}

/**
 * @brief 碰撞检测
 *
 * @param nTetromino
 * @param nRotation
 * @param nPosX
 * @param nPosY
 * @return true
 * @return false
 */
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
    for (int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++)
        {
            // Get index into piece
            int pi = Rotate(px, py, nRotation);

            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
            {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
                {
                    // 当前方块存在 && 场景已经有方块了
                    if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
                        return false; // fail on first hit
                }
            }
        }

    return true;
}

int main()
{

#pragma region assets
    // Create assets
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"....");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");
#pragma endregion

    pField = new unsigned char[nFieldWidth * nFieldHeight]{};
    for (int x = 0; x < nFieldWidth; ++x)
    {
        for (int y = 0; y < nFieldHeight; ++y)
        {
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
        }
    }

    wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight]{};
    for (int i = 0; i < nScreenWidth * nScreenHeight; ++i)
        screen[i] = L' ';

    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Game Logic Stuff
    bool bGameOver = false;

    int nCurrentPiece = 1;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    bool bKey[5];
    bool bRotateHold{false};

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int nScore = 0;

    std::vector<int> vLines{};

    while (!bGameOver)
    {
        // GAME TIMING =====================================================
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed); // 每 20 * 50 ms 方块下降一格

        // INPUT ===========================================================

        for (int k = 0; k < 5; ++k)
        { //                                                     Right Left Down A D
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x41\x44"[k]))) != 0;
        }
        // GAME LOGIC ======================================================
        // Right
        nCurrentX += (bKey[0] && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY))) ? 1 : 0;
        // Left
        nCurrentX -= (bKey[1] && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))) ? 1 : 0;
        // Down
        nCurrentY += (bKey[2] && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))) ? 1 : 0;

        if (bKey[3]) // Right rotate
        {
            nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        if (bKey[4]) // Left rotate
        {
            nCurrentRotation -= (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation - 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else
            bRotateHold = false;

        if (bForceDown)
        {
            // It can , so do it
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                nCurrentY++;
            else
            {
                // Lock the current place in the field
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                    {
                        // take tetromino element to field element
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1; // L" ABCDEFG=#"
                    }

                nPieceCount++;
                if (nPieceCount % 10 == 0) // 每消除10个方块就加快游戏速度
                {
                    if (nSpeed >= 10)
                        nSpeed--;
                }

                // Check have we got any lines
                for (int py = 0; py < 4; ++py)
                    if (nCurrentY + py < nFieldHeight - 1)
                    {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth; px++)
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                        if (bLine) // Remove Line, set to '='
                        {
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;

                            vLines.push_back(nCurrentY + py);
                        }
                    }

                nScore += 25;
                if (!vLines.empty())
                    nScore += (1 << vLines.size()) * 100;

                // choose next piece
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;

                // If piece doesn't fit
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }

            nSpeedCounter = 0;
        }

        // RENDER OUTPUT ===================================================

        // Draw Field
        for (int x = 0; x < nFieldWidth; ++x)
            for (int y = 0; y < nFieldHeight; ++y)
            {
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
            }

        // Draw Current Piece
        for (int px = 0; px < 4; ++px)
            for (int py = 0; py < 4; ++py)
            {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                    screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65; // 将方块的数字 转换成ABCDEFG
            }

        // Draw Score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        if (!vLines.empty())
        {
            // Display Frame (cheekily to draw lines)
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
            this_thread::sleep_for(400ms); // dalay a bit

            for (auto &v : vLines)
            {
                for (int px = 1; px < nFieldWidth - 1; ++px)
                {
                    // 从当前消除行 向上递归下降一行
                    for (int py = v; py > 0; py--)
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    pField[px] = 0; // 最后/最上一行转为空白
                }
            }

            vLines.clear();
        }

        // Display Frame
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
    }

    // Show final Score
    CloseHandle(hConsole);
    cout << "GAME OVER !! Score:" << nScore << endl;
    system("pause");

    return 0;
}