# 贪吃蛇

### 要求：

1.用户首次使用游戏，要求注册游戏用户名和密码;

2.用户每次开始游戏，要求验证用户名和密码，验证通过后进入游戏;在游戏主界面添加显示“张三正在游戏中”

3.在游戏主界面添加显示“按F5显示游戏用户日志”，并完善相应功能;

4.游戏用户表用以记录用户名和密码，游戏用户日志用以记录用户的ID和用户名、每次游戏的开始时间、持续时长和得分;

### 更新：

1.重构游戏流程，以便未来要支持更多“界面”切换，将主循环改为状态机:

(1) **去掉全局 `int endgamestatus`**，所有碰撞/退出原因都由 `snakemove()` 返回：

- `0`：正常移动
- `1`：撞墙
- `2`：咬到自己

(2) **`snakemove()`** 改为 `int`，将墙体检测、自咬检测都放在此函数内，并只负责更新蛇身。

(3) **新增 `gamecircle()` 返回 `GameState`**，将循环内的退出/碰撞逻辑集中处理：

- `ESC` 直接调用 `endgame(3, score)`，返回 `MENU`
- 收到 `snakemove()` 的非零返回值，也调用 `endgame(reason, score)`

(4) **将 `loginMenu()` 重构为状态机方法**，使用 `GameState` 枚举来表示用户选择：

- **`PLAY`** 表示登录成功，进入游戏循环；
- **`EXIT_PROGRAM`** 表示用户选择退出，程序应终止；
- **`loginMenu()`** 第三项“退出”不再直接 `exit(0)`，而是返回 `EXIT`，由 `main()` 退出主循环。

(5) **`main()`** 用 `GameState state` 统一驱动

这样整个流程都由 **`enum GameState { MENU, PLAY, EXIT }`** 来控制，不再依赖额外的全局状态变量。

2.修改了`endgame()`，结束游戏后，由玩家选择以当前账号再来一局或是重新登录。

3.修改了 `writeUserLog()`，用 `localtime()` 替换了 `gmtime()`，确保时间是本地时间；同时格式化并记录“开始时间”、“结束时间”以及“时长（秒）”。

4.修改了`registerUser()`。先检查`userlog.txt`和`user.dat`这两个文件是否存在，如果不存在就先创建它们，再做后续的用户名检测和写入。

5.修改了`registerUser()` 函数，注册时会先扫描 `users.dat` 中已有的用户名，如果发现重复就提示“用户名已被注册”，并让用户重新输入。

6.修改了`snakemove()`函数中的绘制蛇身的逻辑，不用重新绘制整条蛇身，而是只重绘头和尾。这样优化后，每次移动只会更新蛇头和蛇尾两个位置，刷新效率大幅提升。

7.仅在程序启动时调用 `srand()`，将 `srand((unsigned)time(NULL));` 移到 `main()` 函数中。

8.使用独立线程监听按键

(1) **新增 `keyListener`**：独立线程循环里用 `GetAsyncKeyState` 非阻塞地读取方向、暂停（SPACE）、退出（ESC）、加减速（F1/F2），并写入原子变量 `direction`、`pauseFlag`、`exitFlag`。

(2) **`gamestart()` 启动线程**：`std::thread t(keyListener); t.detach();`。

(3) **`gamecircle()`**：

- 每轮检查 `exitFlag` 强制退出；
- 检查 `pauseFlag` 暂停循环；
- 去掉原先的方向判断，把移动调用 `snakemove()` 改用 `direction.load()`。

(4) **`snakemove()`**：把 `if (GetAsyncKeyState…)` 块删掉，改为 `switch(direction.load())`。



userlog表 userlog.txt

| 字段名       | 类型         | 含义                                                 |
| ------------ | ------------ | ---------------------------------------------------- |
| `id`         | `INT`        | 用户的唯一标识（`currentUser.id`）                   |
| `username`   | `VARCHAR(…)` | 用户名（`currentUser.username`）                     |
| `start_time` | `DATETIME`   | 本局游戏开始时间（格式`YYYY-MM-DD HH:MM:SS`）        |
| `end_time`   | `DATETIME`   | 本局游戏结束时间（格式`YYYY-MM-DD HH:MM:SS`）        |
| `duration_s` | `INT`        | 本局时长，单位秒（`difftime(end_time, start_time)`） |
| `score`      | `INT`        | 本局得分                                             |

users表 users.dat

| 字段名     | 类型          | 说明                                                     |
| ---------- | ------------- | -------------------------------------------------------- |
| `id`       | `INT`         | 用户唯一标识，自动递增（第一个用户为 1，后续按记录数+1） |
| `username` | `VARCHAR(32)` | 用户名，长度上限 31 字符，必须唯一                       |
| `password` | `VARCHAR(32)` | 密码，长度上限 31 字符                                   |
