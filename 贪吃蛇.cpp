#include <graphics.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <ctime>
#include <algorithm>
#include <windows.h>
#include <conio.h>
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")

#define WIDTH 1600
#define HEIGHT 840
#define CELL_SIZE 20
#define BUBBLE_COUNT 20

using namespace std;

//下载图片
IMAGE img_res[3];//保存图片的数组，和NUM保持一致
void loadImg() {
    for (int i = 0; i < 3; i++) {
        char fileName[50] = "";
        sprintf_s(fileName, "./Image/%d.jpg", i);
        loadimage(img_res + i, fileName);
    }
}

//下载音乐
void PlayMusic() {
    mciSendString(("open ninelie.mp3"), NULL, 0, NULL);
    mciSendString(("play ninelie.mp3 repeat"), NULL, 0, NULL);
}

// 蛇的节点类
class SnakeNode {
public:
    int x, y;
    int type; // 0: 蛇头, 1: 蛇身
    int direction; // 0: 上, 1: 右, 2: 下, 3: 左
    int speed;
    int size;

    SnakeNode(int _x, int _y, int _type, int _direction, int _speed, int _size)
        : x(_x), y(_y), type(_type), direction(_direction), speed(_speed), size(_size) {}
};

// 蛇类
class Snake {
public:
    list<SnakeNode> body;
    int speed;
    int size;
    bool isAccelerating;
    int accelerateCountdown;
    int longPressTime;

    Snake(int x, int y, int initSize = 3) {
        speed = CELL_SIZE;
        size = initSize;
        isAccelerating = false;
        accelerateCountdown = 0;
        longPressTime = 0;
        body.push_back(SnakeNode(x, y, 0, 1, speed, size));
        for (int i = 1; i < size; ++i) {
            body.push_back(SnakeNode(x - i * CELL_SIZE, y, 1, 1, speed, size));
        }
    }

    void move() {
        SnakeNode head = body.front();
        int newX = head.x;
        int newY = head.y;
        switch (head.direction) {
        case 0: newY -= speed; break;
        case 1: newX += speed; break;
        case 2: newY += speed; break;
        case 3: newX -= speed; break;
        }
        body.push_front(SnakeNode(newX, newY, 0, head.direction, speed, size));
        body.pop_back();
    }

    void changeDirection(int newDirection) {
        if ((newDirection + 2) % 4 != body.front().direction) {
            body.front().direction = newDirection;
        }
    }

    void grow() {
        size++;
        SnakeNode last = body.back();
        int newX = last.x;
        int newY = last.y;
        switch (last.direction) {
        case 0: newY += CELL_SIZE; break;
        case 1: newX -= CELL_SIZE; break;
        case 2: newY -= CELL_SIZE; break;
        case 3: newX += CELL_SIZE; break;
        }
        body.push_back(SnakeNode(newX, newY, 1, last.direction, speed, size));
    }

    bool isCollidingWithSelf() {
        SnakeNode head = body.front();
        auto it = body.begin();
        ++it;
        for (; it != body.end(); ++it) {
            if (head.x == it->x && head.y == it->y) {
                return true;
            }
        }
        return false;
    }

    bool isCollidingWithWall() {
        SnakeNode head = body.front();
        return head.x < 0 || head.x >= WIDTH || head.y < 0 || head.y >= HEIGHT;
    }

    bool isCollidingWithSnake(const Snake& other) {
        SnakeNode head = body.front();
        for (const auto& node : other.body) {
            if (head.x == node.x && head.y == node.y) {
                return true;
            }
        }
        return false;
    }
};

// 泡泡类
class Bubble {
public:
    int x, y;
    int size;

    Bubble(int _x, int _y, int _size) : x(_x), y(_y), size(_size) {}
};

// 排行榜记录类
struct RankRecord {
    string username;
    int score;
    string difficulty;
    int timeElapsed;
    int length;

    bool operator<(const RankRecord& other) const {
        if (score != other.score) {
            return score > other.score;
        }
        return timeElapsed < other.timeElapsed;
    }
};

// 读取排行榜
vector<RankRecord> readRankings() {
    vector<RankRecord> rankings;
    ifstream file("rankings.txt");
    if (file.is_open()) {
        string username;
        int score;
        string difficulty;
        int timeElapsed;
        int length;
        while (file >> username >> score >> difficulty >> timeElapsed >> length) {
            cout << "Read record: " << username << " " << score << " " << difficulty << " " << timeElapsed << " " << length << endl;
            rankings.push_back({ username, score, difficulty, timeElapsed, length });
        }
        file.close();
    }
    return rankings;
}

// 写入排行榜
void writeRankings(const vector<RankRecord>& rankings) {
    ofstream file("rankings.txt");
    if (file.is_open()) {
        for (const auto& record : rankings) {
            file << record.username << " " << record.score << " " << record.difficulty << " " << record.timeElapsed << " " << record.length << endl;
        }
        file.close();
    }
}

// 更新排行榜记录
void updateRankings(vector<RankRecord>& rankings, const string& username, int score, const string& difficulty, int timeElapsed, int length) {
    for (auto& record : rankings) {
        if (record.username == username && record.difficulty == difficulty) {
            if (score > record.score || (score == record.score && timeElapsed < record.timeElapsed)) {
                record.score = score;
                record.timeElapsed = timeElapsed;
                record.length = length;
            }
            return;
        }
    }
    cout << "Adding record for " << username << " in " << difficulty << " difficulty." << endl;
    rankings.push_back({ username, score, difficulty, timeElapsed, length });
}

// 显示排行榜
void showRankings(const vector<RankRecord>& rankings) {
    settextcolor(WHITE);
    setbkmode(TRANSPARENT); // 设置字体背景为透明
    putimage(0, 0, &img_res[0]);
    vector<RankRecord> superEasyRankings, easyRankings, normalRankings, hardRankings, insaneRankings;
    for (const auto& record : rankings) {
        if (record.difficulty == "SuperEasy") {
            cout << "Found SuperEasy record: " << record.username << endl;
            superEasyRankings.push_back(record);
        }
        else if (record.difficulty == "Easy") {
            easyRankings.push_back(record);
        }
        else if (record.difficulty == "Normal") {
            normalRankings.push_back(record);
        }
        else if (record.difficulty == "Hard") {
            hardRankings.push_back(record);
        }
        else if (record.difficulty == "Insane") {
            insaneRankings.push_back(record);
        }
    }

    sort(superEasyRankings.begin(), superEasyRankings.end());
    sort(easyRankings.begin(), easyRankings.end());
    sort(normalRankings.begin(), normalRankings.end());
    sort(hardRankings.begin(), hardRankings.end());
    sort(insaneRankings.begin(), insaneRankings.end());

    int xPositions[] = { 50, 350, 650, 950, 1250 };
    int y = 150;

    // 显示超级简单难度排行榜
    string superEasyTitle = "SuperEasy Rankings";
    outtextxy(xPositions[0], y, superEasyTitle.c_str());
    y += 20;
    string superEasyHeader = "Rank Username Score Time(s) Length";
    outtextxy(xPositions[0], y, superEasyHeader.c_str());
    y += 20;
    for (int i = 0; i < min(3, static_cast<int>(superEasyRankings.size())); ++i) {
        string line = to_string(i + 1) + ". " + superEasyRankings[i].username + " " + to_string(superEasyRankings[i].score) + " " + to_string(superEasyRankings[i].timeElapsed) + "s " + to_string(superEasyRankings[i].length);
        outtextxy(xPositions[0], y, line.c_str());
        y += 20;
    }

    y = 150;
    // 显示简单难度排行榜
    string easyTitle = "Easy Rankings";
    outtextxy(xPositions[1], y, easyTitle.c_str());
    y += 20;
    string easyHeader = "Rank Username Score Time(s) Length";
    outtextxy(xPositions[1], y, easyHeader.c_str());
    y += 20;
    for (int i = 0; i < min(3, static_cast<int>(easyRankings.size())); ++i) {
        string line = to_string(i + 1) + ". " + easyRankings[i].username + " " + to_string(easyRankings[i].score) + " " + to_string(easyRankings[i].timeElapsed) + "s " + to_string(easyRankings[i].length);
        outtextxy(xPositions[1], y, line.c_str());
        y += 20;
    }

    y = 150;
    // 显示普通难度排行榜
    string normalTitle = "Normal Rankings";
    outtextxy(xPositions[2], y, normalTitle.c_str());
    y += 20;
    string normalHeader = "Rank Username Score Time(s) Length";
    outtextxy(xPositions[2], y, normalHeader.c_str());
    y += 20;
    for (int i = 0; i < min(3, static_cast<int>(normalRankings.size())); ++i) {
        string line = to_string(i + 1) + ". " + normalRankings[i].username + " " + to_string(normalRankings[i].score) + " " + to_string(normalRankings[i].timeElapsed) + "s " + to_string(normalRankings[i].length);
        outtextxy(xPositions[2], y, line.c_str());
        y += 20;
    }

    y = 150;
    // 显示困难难度排行榜
    string hardTitle = "Hard Rankings";
    outtextxy(xPositions[3], y, hardTitle.c_str());
    y += 20;
    string hardHeader = "Rank Username Score Time(s) Length";
    outtextxy(xPositions[3], y, hardHeader.c_str());
    y += 20;
    for (int i = 0; i < min(3, static_cast<int>(hardRankings.size())); ++i) {
        string line = to_string(i + 1) + ". " + hardRankings[i].username + " " + to_string(hardRankings[i].score) + " " + to_string(hardRankings[i].timeElapsed) + "s " + to_string(hardRankings[i].length);
        outtextxy(xPositions[3], y, line.c_str());
        y += 20;
    }

    y = 150;
    // 显示疯狂难度排行榜
    string insaneTitle = "Insane Rankings";
    outtextxy(xPositions[4], y, insaneTitle.c_str());
    y += 20;
    string insaneHeader = "Rank Username Score Time(s) Length";
    outtextxy(xPositions[4], y, insaneHeader.c_str());
    y += 20;
    for (int i = 0; i < min(3, static_cast<int>(insaneRankings.size())); ++i) {
        string line = to_string(i + 1) + ". " + insaneRankings[i].username + " " + to_string(insaneRankings[i].score) + " " + to_string(insaneRankings[i].timeElapsed) + "s " + to_string(insaneRankings[i].length);
        outtextxy(xPositions[4], y, line.c_str());
        y += 20;
    }

    string prompt = "Please enter your username:";
    outtextxy(600, 50, prompt.c_str());
    string startPrompt = "Click the screen to start...";
    outtextxy(600, HEIGHT - 50, startPrompt.c_str());
}

// 登录界面
string login() {
    initgraph(WIDTH, HEIGHT);
    setbkcolor(WHITE);
    cleardevice();
    vector<RankRecord> rankings = readRankings();
    showRankings(rankings);
    settextcolor(BLACK);
    setbkmode(TRANSPARENT); // 设置字体背景为透明
    char username[100];
    InputBox(username, 100, "Username");

    // 处理鼠标点击事件
    MOUSEMSG msg;
    while (true) {
        msg = GetMouseMsg();
        if (msg.uMsg == WM_LBUTTONDOWN) {
            break;
        }
    }
    return string(username);
}

// 选择难度
string selectDifficulty() {
    initgraph(WIDTH, HEIGHT);
    setbkcolor(BLACK);
    cleardevice();
    settextcolor(WHITE);
    setbkmode(TRANSPARENT); // 设置字体背景为透明
    putimage(0, 0, &img_res[1]);
    string difficultyPrompt = "Select difficulty:";
    outtextxy(600, 100, difficultyPrompt.c_str());

    // 绘制难度选项按钮
    const int buttonWidth = 200;
    const int buttonHeight = 50;
    const int buttonX = 600;
    int buttonY = 150;

    string superEasyOption = "1. SuperEasy";
    rectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);
    outtextxy(buttonX + 10, buttonY + 10, superEasyOption.c_str());

    string easyOption = "2. Easy";
    buttonY += buttonHeight + 20;
    rectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);
    outtextxy(buttonX + 10, buttonY + 10, easyOption.c_str());

    string normalOption = "3. Normal";
    buttonY += buttonHeight + 20;
    rectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);
    outtextxy(buttonX + 10, buttonY + 10, normalOption.c_str());

    string hardOption = "4. Hard";
    buttonY += buttonHeight + 20;
    rectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);
    outtextxy(buttonX + 10, buttonY + 10, hardOption.c_str());

    string insaneOption = "5. Insane";
    buttonY += buttonHeight + 20;
    rectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);
    outtextxy(buttonX + 10, buttonY + 10, insaneOption.c_str());

    // 处理鼠标点击事件
    MOUSEMSG msg;
    while (true) {
        msg = GetMouseMsg();
        if (msg.uMsg == WM_LBUTTONDOWN) {
            int clickX = msg.x;
            int clickY = msg.y;

            // 检查点击位置是否在按钮范围内
            if (clickX >= buttonX && clickX <= buttonX + buttonWidth) {
                if (clickY >= 150 && clickY <= 150 + buttonHeight) {
                    return "SuperEasy";
                }
                else if (clickY >= 150 + buttonHeight + 20 && clickY <= 150 + 2 * buttonHeight + 20) {
                    return "Easy";
                }
                else if (clickY >= 150 + 2 * buttonHeight + 40 && clickY <= 150 + 3 * buttonHeight + 40) {
                    return "Normal";
                }
                else if (clickY >= 150 + 3 * buttonHeight + 60 && clickY <= 150 + 4 * buttonHeight + 60) {
                    return "Hard";
                }
                else if (clickY >= 150 + 4 * buttonHeight + 80 && clickY <= 150 + 5 * buttonHeight + 80) {
                    return "Insane";
                }
            }
        }
    }
}

// 生成随机蛇，确保刷新位置不与现有蛇重合
vector<Snake> generateRandomSnakes(const string& difficulty, const Snake& playerSnake, int initSize = 3) {
    vector<Snake> snakes;
    int numSnakes = 0;
    if (difficulty == "SuperEasy") numSnakes = 1;
    else if (difficulty == "Easy") numSnakes = 3;
    else if (difficulty == "Normal") numSnakes = 5;
    else if (difficulty == "Hard") numSnakes = 10;
    else if (difficulty == "Insane") numSnakes = 15;
    srand(static_cast<unsigned int>(time(NULL))); // 明确类型转换
    for (int i = 0; i < numSnakes; ++i) {
        bool validPosition = false;
        int x, y;
        while (!validPosition) {
            x = rand() % (WIDTH / CELL_SIZE) * CELL_SIZE;
            y = rand() % (HEIGHT / CELL_SIZE) * CELL_SIZE;
            validPosition = true;
            // 检查是否与玩家蛇重合
            for (const auto& node : playerSnake.body) {
                if (x == node.x && y == node.y) {
                    validPosition = false;
                    break;
                }
            }
            // 检查是否与已生成的敌方蛇重合
            for (const auto& snake : snakes) {
                for (const auto& node : snake.body) {
                    if (x == node.x && y == node.y) {
                        validPosition = false;
                        break;
                    }
                }
                if (!validPosition) break;
            }
        }
        snakes.push_back(Snake(x, y, initSize));
    }
    return snakes;
}

// 生成随机泡泡，确保不与现有泡泡重合
Bubble generateRandomBubble(const vector<Bubble>& existingBubbles) {
    int x, y;
    bool validPosition = false;
    while (!validPosition) {
        x = (rand() % (WIDTH / CELL_SIZE)) * CELL_SIZE;
        y = (rand() % (HEIGHT / CELL_SIZE)) * CELL_SIZE;
        validPosition = true;
        for (const auto& bubble : existingBubbles) {
            if (x == bubble.x && y == bubble.y) {
                validPosition = false;
                break;
            }
        }
    }
    return Bubble(x, y, CELL_SIZE / 2);
}

// 绘制蛇
void drawSnake(const Snake& snake, bool isPlayer) {
    if (isPlayer) {
        setfillcolor(RED);
    }
    else {
        setfillcolor(GREEN);
    }
    auto it = snake.body.begin();
    // 先绘制蛇身
    ++it;
    for (; it != snake.body.end(); ++it) {
        fillroundrect(it->x, it->y, it->x + CELL_SIZE, it->y + CELL_SIZE, 10, 10);
    }
    // 再绘制蛇头
    SnakeNode head = snake.body.front();
    int pts[6];
    switch (head.direction) {
    case 0: // 上
        pts[0] = head.x + CELL_SIZE / 2; pts[1] = head.y;
        pts[2] = head.x; pts[3] = head.y + CELL_SIZE;
        pts[4] = head.x + CELL_SIZE; pts[5] = head.y + CELL_SIZE;
        break;
    case 1: // 右
        pts[0] = head.x + CELL_SIZE; pts[1] = head.y + CELL_SIZE / 2;
        pts[2] = head.x; pts[3] = head.y;
        pts[4] = head.x; pts[5] = head.y + CELL_SIZE;
        break;
    case 2: // 下
        pts[0] = head.x + CELL_SIZE / 2; pts[1] = head.y + CELL_SIZE;
        pts[2] = head.x; pts[3] = head.y;
        pts[4] = head.x + CELL_SIZE; pts[5] = head.y;
        break;
    case 3: // 左
        pts[0] = head.x; pts[1] = head.y + CELL_SIZE / 2;
        pts[2] = head.x + CELL_SIZE; pts[3] = head.y;
        pts[4] = head.x + CELL_SIZE; pts[5] = head.y + CELL_SIZE;
        break;
    }
    fillpoly(3, pts);
}

// 绘制泡泡
void drawBubbles(const vector<Bubble>& bubbles) {
    setfillcolor(MAGENTA);
    for (const auto& bubble : bubbles) {
        fillcircle(bubble.x, bubble.y, bubble.size);
    }
}

// 判断蛇是否吃到泡泡，增大吃的范围
bool isEatingBubble(const Snake& snake, const Bubble& bubble) {
    SnakeNode head = snake.body.front();
    int bubbleRange = CELL_SIZE * 2;
    return abs(head.x - bubble.x) < bubbleRange && abs(head.y - bubble.y) < bubbleRange;
}

// 判断蛇头是否碰撞
bool isHeadColliding(const Snake& snake1, const Snake& snake2) {
    SnakeNode head1 = snake1.body.front();
    SnakeNode head2 = snake2.body.front();
    return head1.x == head2.x && head1.y == head2.y;
}

// 结算界面
int gameOverScreen(const string& username, int score, const string& difficulty, int timeElapsed, int length) {
    cleardevice();
    putimage(0, 0, &img_res[2]);
    settextcolor(BLACK);
    setbkmode(TRANSPARENT); // 设置字体背景为透明
    string resultText = "Game Over! Score: " + to_string(score) + ", Time: " + to_string(timeElapsed) + "s, Length: " + to_string(length);
    outtextxy(600, 300, resultText.c_str());

    string replayPrompt = "Press 'Y' to play again, 'N' to quit, 'M' to return to main menu.";
    outtextxy(600, 350, replayPrompt.c_str());

    while (true) {
        if (GetAsyncKeyState('Y') & 0x8000) {
            return 1;
        }
        if (GetAsyncKeyState('N') & 0x8000) {
            return 0;
        }
        if (GetAsyncKeyState('M') & 0x8000) {
            return 2;
        }
    }
}

// 主游戏循环
int gameLoop(const string& username, const string& difficulty) {
    initgraph(WIDTH, HEIGHT);
    setbkcolor(WHITE);
    BeginBatchDraw(); // 开启双缓冲，解决屏幕闪烁问题
    Snake playerSnake(WIDTH / 2, HEIGHT / 2);
    vector<int> enemySnakeSizes;
    vector<Snake> enemySnakes = generateRandomSnakes(difficulty, playerSnake);
    for (const auto& snake : enemySnakes) {
        enemySnakeSizes.push_back(snake.size);
    }
    vector<Bubble> bubbles;
    vector<Bubble> deadSnakeBubbles;
    for (int i = 0; i < BUBBLE_COUNT; ++i) {
        bubbles.push_back(generateRandomBubble(bubbles));
    }
    clock_t startTime = clock();
    int score = 0;
    int timeElapsed = 0;
    bool isPaused = false;

    while (true) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            Sleep(200); // 避免重复触发
            isPaused = !isPaused;
            if (isPaused) {
                settextcolor(BLACK);
                setbkmode(TRANSPARENT); // 设置字体背景为透明
                outtextxy(600, 400, "Game Paused. Press ESC to continue.");
                FlushBatchDraw();
            }
        }

        if (isPaused) {
            continue;
        }

        cleardevice(); // 清屏
        timeElapsed = (clock() - startTime) / CLOCKS_PER_SEC;

        if (GetAsyncKeyState('W') & 0x8000) {
            playerSnake.changeDirection(0);
        }
        else if (GetAsyncKeyState('D') & 0x8000) {
            playerSnake.changeDirection(1);
        }
        else if (GetAsyncKeyState('S') & 0x8000) {
            playerSnake.changeDirection(2);
        }
        else if (GetAsyncKeyState('A') & 0x8000) {
            playerSnake.changeDirection(3);
        }

        if (GetAsyncKeyState('W') & 0x8000) {
            if (playerSnake.longPressTime == 0) {
                playerSnake.longPressTime = clock();
            }
            if (playerSnake.size > 3 && clock() - playerSnake.longPressTime >= 0.5 * CLOCKS_PER_SEC) {
                playerSnake.isAccelerating = true;
                if (playerSnake.accelerateCountdown == 0) {
                    playerSnake.accelerateCountdown = clock();
                }
            }
        }
        else if (GetAsyncKeyState('D') & 0x8000) {
            if (playerSnake.longPressTime == 0) {
                playerSnake.longPressTime = clock();
            }
            if (playerSnake.size > 3 && clock() - playerSnake.longPressTime >= 0.5 * CLOCKS_PER_SEC) {
                playerSnake.isAccelerating = true;
                if (playerSnake.accelerateCountdown == 0) {
                    playerSnake.accelerateCountdown = clock();
                }
            }
        }
        else if (GetAsyncKeyState('S') & 0x8000) {
            if (playerSnake.longPressTime == 0) {
                playerSnake.longPressTime = clock();
            }
            if (playerSnake.size > 3 && clock() - playerSnake.longPressTime >= 0.5 * CLOCKS_PER_SEC) {
                playerSnake.isAccelerating = true;
                if (playerSnake.accelerateCountdown == 0) {
                    playerSnake.accelerateCountdown = clock();
                }
            }
        }
        else if (GetAsyncKeyState('A') & 0x8000) {
            if (playerSnake.longPressTime == 0) {
                playerSnake.longPressTime = clock();
            }
            if (playerSnake.size > 3 && clock() - playerSnake.longPressTime >= 0.5 * CLOCKS_PER_SEC) {
                playerSnake.isAccelerating = true;
                if (playerSnake.accelerateCountdown == 0) {
                    playerSnake.accelerateCountdown = clock();
                }
            }
        }
        else {
            playerSnake.longPressTime = 0;
            playerSnake.isAccelerating = false;
            playerSnake.speed = CELL_SIZE; // 恢复正常速度
        }

        if (playerSnake.isAccelerating) {
            playerSnake.speed = CELL_SIZE * 2; // 加速时速度翻倍
            if (clock() - playerSnake.accelerateCountdown >= 3 * CLOCKS_PER_SEC) {
                if (playerSnake.size > 3) {
                    playerSnake.size--;
                    playerSnake.body.pop_back();
                    playerSnake.accelerateCountdown = clock();
                }
            }
        }

        // 移动玩家蛇
        playerSnake.move();

        // 移动敌人蛇
        auto it = enemySnakes.begin();
        auto sizeIt = enemySnakeSizes.begin();
        while (it != enemySnakes.end()) {
            if (it->isCollidingWithWall() || it->isCollidingWithSnake(playerSnake) || it->isCollidingWithSelf()) {
                // 蛇死亡变成泡泡
                for (const auto& node : it->body) {
                    deadSnakeBubbles.push_back(Bubble(node.x + CELL_SIZE / 2, node.y + CELL_SIZE / 2, node.size));
                }
                // 增加蛇的长度
                *sizeIt += 1;
                // 重新生成蛇，确保不与现有蛇重合
                bool validPosition = false;
                int x, y;
                while (!validPosition) {
                    x = rand() % (WIDTH / CELL_SIZE) * CELL_SIZE;
                    y = rand() % (HEIGHT / CELL_SIZE) * CELL_SIZE;
                    validPosition = true;
                    // 检查是否与玩家蛇重合
                    for (const auto& node : playerSnake.body) {
                        if (x == node.x && y == node.y) {
                            validPosition = false;
                            break;
                        }
                    }
                    // 检查是否与已生成的敌方蛇重合
                    for (auto otherIt = enemySnakes.begin(); otherIt != enemySnakes.end(); ++otherIt) {
                        if (otherIt == it) continue;
                        for (const auto& node : otherIt->body) {
                            if (x == node.x && y == node.y) {
                                validPosition = false;
                                break;
                            }
                        }
                        if (!validPosition) break;
                    }
                }
                *it = Snake(x, y, *sizeIt);
            }
            if (rand() % 100 < 10) { // 10% 的概率改变方向
                it->changeDirection(rand() % 4);
            }
            it->move();
            ++it;
            ++sizeIt;
        }

        // 检查玩家蛇是否吃到泡泡
        for (auto it = bubbles.begin(); it != bubbles.end(); ) {
            if (isEatingBubble(playerSnake, *it)) {
                playerSnake.grow();
                it = bubbles.erase(it);
                bubbles.push_back(generateRandomBubble(bubbles));
                score++;
                break; // 确保只吃一个泡泡
            }
            else {
                ++it;
            }
        }

        // 检查玩家蛇是否吃到死亡蛇变成的泡泡
        for (auto it = deadSnakeBubbles.begin(); it != deadSnakeBubbles.end(); ) {
            if (isEatingBubble(playerSnake, *it)) {
                playerSnake.grow();
                it = deadSnakeBubbles.erase(it);
                score++;
                break; // 确保只吃一个泡泡
            }
            else {
                ++it;
            }
        }

        // 检查蛇头碰撞
        bool isGameOver = false;
        for (const auto& enemy : enemySnakes) {
            if (isHeadColliding(playerSnake, enemy)) {
                // 蛇头撞蛇头，双方都死
                for (const auto& node : playerSnake.body) {
                    deadSnakeBubbles.push_back(Bubble(node.x + CELL_SIZE / 2, node.y + CELL_SIZE / 2, node.size));
                }
                for (const auto& node : enemy.body) {
                    deadSnakeBubbles.push_back(Bubble(node.x + CELL_SIZE / 2, node.y + CELL_SIZE / 2, node.size));
                }
                isGameOver = true;
                break;
            }
            else if (playerSnake.isCollidingWithSnake(enemy) && !isHeadColliding(playerSnake, enemy)) {
                // 玩家蛇头撞敌人蛇身，玩家死
                for (const auto& node : playerSnake.body) {
                    deadSnakeBubbles.push_back(Bubble(node.x + CELL_SIZE / 2, node.y + CELL_SIZE / 2, node.size));
                }
                isGameOver = true;
                break;
            }
        }

        if (isGameOver) {
            break;
        }

        // 检查玩家蛇是否撞到自己或墙壁
        if (playerSnake.isCollidingWithSelf() || playerSnake.isCollidingWithWall()) {
            for (const auto& node : playerSnake.body) {
                deadSnakeBubbles.push_back(Bubble(node.x + CELL_SIZE / 2, node.y + CELL_SIZE / 2, node.size));
            }
            break;
        }

        // 绘制蛇和泡泡
        drawSnake(playerSnake, true);
        for (const auto& enemy : enemySnakes) {
            drawSnake(enemy, false);
        }
        drawBubbles(bubbles);
        drawBubbles(deadSnakeBubbles);

        // 显示得分、时间和长度，更改显示颜色
        settextcolor(BLUE);
        setbkmode(TRANSPARENT); // 设置字体背景为透明
        string scoreText = "Score: " + to_string(score);
        string timeText = "Time: " + to_string(timeElapsed) + "s";
        string lengthText = "Length: " + to_string(playerSnake.size);
        outtextxy(10, 10, scoreText.c_str());
        outtextxy(10, 30, timeText.c_str());
        outtextxy(10, 50, lengthText.c_str());

        FlushBatchDraw(); // 刷新屏幕
        Sleep(playerSnake.isAccelerating ? 50 : 100); // 加速时减少休眠时间
    }

    EndBatchDraw(); // 关闭双缓冲

    // 记录得分
    vector<RankRecord> rankings = readRankings();
    updateRankings(rankings, username, score, difficulty, timeElapsed, playerSnake.size);
    writeRankings(rankings);

    // 显示结算界面
    return gameOverScreen(username, score, difficulty, timeElapsed, playerSnake.size);
}

int main() {
    string username;
    loadImg();
    PlayMusic();
    while (true) {
        username = login();
        string difficulty = selectDifficulty();
        int result = gameLoop(username, difficulty);
        if (result == 0) {
            break;
        }
        else if (result == 2) {
            continue;
        }
    }
    return 0;
}