# DDNet-HunterN猎人杀 [![](https://github.com/Hu1night/DDNet-HunterN/workflows/Build/badge.svg)](https://github.com/Hu1night/DDNet-HunterN/actions/workflows/build.yaml?query=branch%3Amaster++)
[**DDNet-HunterN**](https://github.com/Hu1night/DDNet-HunterN)是一个基于[**DDNet-PvP**](https://github.com/TeeworldsCN/ddnet-pvp)(传统竞技)(基于DDNet 15.3.2)的[**DDNet**](https://github.com/DDNet/DDNet)模组项目 提供名为***HunterN***(猎人杀)的PvP模式

### HunterN的游戏规则：
1. 每局开始时会逐个**秘密随机**选择玩家成为**猎人**或**平民** 开局只提示玩家自己身份 猎人的目标是**消灭所有平民**
2. 猎人使用高伤武器、瞬杀追踪锤和破片榴弹 **而平民没有锤子且只能使用常规武器**
3. 当玩家死时如为猎人死亡则通知其他猎人 玩家死亡原因和死后聊天**仅旁观/死人可见**

### HunterN的职业细节
- 猎人锤子20伤,长按追踪
- 猎人手枪&霰弹每颗2伤
- 猎人破片榴弹爆炸6伤(对自己无伤)和16*1伤破片
- 猎人激光9伤

## 在Ubuntu上使用CMake构建DDNet-HunterN
1. 使用apt安装***依*****赖***库*
```
sudo apt install build-essential cmake python3 libsqlite3-dev libcurl4-openssl-dev zlib1g-dev
```
2. 转到项目目录编译服务端
```
cmake .
make -j$(nproc)
```
Pass the number of threads for compilation to `make -j`. `$(nproc)` in this case returns the number of processing units.

DDNet requires additional libraries, that are bundled for the most common platforms (Windows, Mac, Linux, all x86 and x86\_64). The bundled libraries are now in the ddnet-libs submodule.

The following is a non-exhaustive list of build arguments that can be passed to the `cmake` command-line tool in order to enable or disable options in build time:

* **-DCMAKE_BUILD_TYPE=[Release|Debug|RelWithDebInfo|MinSizeRel]** <br>
An optional CMake variable for setting the build type. If not set, defaults to "Release" if `-DDEV=ON` is **not** used, and "Debug" if `-DDEV=ON` is used. See `CMAKE_BUILD_TYPE` in CMake Documentation for more information.

* **-DPREFER_BUNDLED_LIBS=[ON|OFF]** <br>
Whether to prefer bundled libraries over system libraries. Setting to ON will make DDNet use third party libraries available in the `ddnet-libs` folder, which is the git-submodule target of the [ddnet-libs](https://github.com/ddnet/ddnet-libs) repository mentioned above -- Useful if you do not have those libraries installed and want to avoid building them. If set to OFF, will only use bundled libraries when system libraries are not found. Default value is OFF.

* **-DWEBSOCKETS=[ON|OFF]** <br>
Whether to enable WebSocket support for server. Setting to ON requires the `libwebsockets-dev` library installed. Default value is OFF.

* **-DMYSQL=[ON|OFF]** <br>
Whether to enable MySQL/MariaDB support for server. Requires at least MySQL 8.0 or MariaDB 10.2. Setting to ON requires the `libmariadbclient-dev`, `libmysqlcppconn-dev` and `libboost-dev` libraries installed, which are also provided as bundled libraries for the common platforms. Default value is OFF.

   Note that the bundled MySQL libraries might not work properly on your system. If you run into connection problems with the MySQL server, for example that it connects as root while you chose another user, make sure to install your system libraries for the MySQL client and C++ connector. Make sure that the CMake configuration summary says that it found MySQL libs that were not bundled (no "using bundled libs").

* **-DDOWNLOAD_GTEST=[ON|OFF]** <br>
Whether to download and compile GTest. Useful if GTest is not installed and, for Linux users, there is no suitable package providing it. Default value is OFF.

* **-DDEV=[ON|OFF]** <br>
Whether to optimize for development, speeding up the compilation process a little. If enabled, don't generate stuff necessary for packaging. Setting to ON will set CMAKE\_BUILD\_TYPE to Debug by default. Default value is OFF.

* **-DUPNP=[ON|OFF]** <br>
Whether to enable UPnP support for the server.
You need to install `libminiupnpc-dev` on Debian, `miniupnpc` on Arch Linux.

* **-GNinja** <br>
Use the Ninja build system instead of Make. This automatically parallizes the build and is generally faster. Compile with `ninja` instead of `make`. Install Ninja with `sudo apt install ninja-build` on Debian, `sudo pacman -S --needed ninja` on Arch Linux.

## DDNet-HunterN的服务端下载&配置
- **关于DDNet-HunterN的一些下载源**
  - 你可以下载**正式发布包**于[Github Releases](https://github.com/Hu1night/DDNet-HunterN/releases)
  - 或者下载**开发构建**于[Github Actions](https://github.com/Hu1night/DDNet-HunterN/actions/workflows/build.yaml?query=branch%3Amaster++)（[几乎汉化分支构建](https://github.com/Hu1night/DDNet-HunterN/actions/workflows/build.yaml?query=branch%3Ahuntern-zh_cn++)）(下载需Github账号)

- **关于DDNet-HunterN的一些配置指令**
  - ```sv_room_commands```于控制台启用时才能使玩家创建/加入房间
  - ```room_setting 0```于控制台输入可查看0号房间可用的模式指令 使用实例：```room_setting 0 timelimit 3``` (设置0号房间每局限时3分钟)
  - ```/setting```于(管理员)聊天栏输入可查看所在房间可用的模式指令 使用实例：```/setting map dm2``` (设置所在房间的子地图为dm2)

- **关于DDNet-HunterN的一些配置文件**
  - 服务端启动时执行/autoexec.cfg
  - 默认HunterN房间启动时执行/room_config/modes/huntern.rcfg
  - huntern_msc.map地图启动时执行/data/maps/huntern_msc.map.cfg
  - 服务器资产定位使用/storage.cfg

- **关于DDNet-PvP的Mega map生成**
  - 使用[DDNet-PvP附加工具](https://github.com/Hu1night/DDNet-HunterN/releases/download/0.3a1/DDNet-PvP.Extra.tools.zip)(一般编译会自动生成)中的map_merge程序进行Mega map的生成
  - 使用用法：```map_merge 输出地图名 输入地图1 输入地图2 输入地图3 ... 输入地图n```
  - 使用实例：```map_merge huntern.map hunter1.map hunter2.map hunter3.map hunter4.map hunter6.map```
  - 程序原理：在**Mega地图**的**SpeedUp层**里的**n号子地图**放置**最大速度值为n**、类型ID为255的Tile
  - 实现片段：
    - ```TILE_MEGAMAP_INDEX = 255```/src/game/mapitems.h:211
    - ```bool IsMegaMapIndex = SpeedupTile.m_Type == TILE_MEGAMAP_INDEX;```/src/game/gamecontext.cpp:3036 <- ***2024.3.3***
    - ```pSpeedupTiles[TargetIndex].m_MaxSpeed = MapIndex + 1;```/src/tools/map_merge.cpp:648