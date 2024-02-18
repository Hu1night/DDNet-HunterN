# DDNet-HunterN猎人杀
[**DDNet-HunterN**](https://github.com/Hu1night/DDNet-HunterN)是一个基于[**DDNet-PvP**](https://github.com/TeeworldsCN/ddnet-pvp)(传统竞技)(基于DDNet 15.3.2)的[**DDNet**](https://github.com/DDNet/DDNet)模组项目 提供名为***HunterN***(猎人杀)的PvP模式

### HunterN的游戏规则：
1. 每局开始时会**秘密随机**选择玩家成为**猎人**或**平民** 且玩家只知道自己身份 猎人的目标是**消灭所有平民**
2. 猎人使用高伤武器、瞬杀追踪锤(20伤,长按追踪)和破片榴弹 **而平民没有锤子且只能使用常规武器**
3. 当玩家死时如为猎人死亡则通知其他猎人 且死亡原因和死后聊天**仅旁观/死人可见**

## 在Ubuntu上使用CMake构建DDNet-HunterN
1. 使用apt安装***依*****赖***库*
```
sudo apt install build-essential cmake python3 libsqlite3-dev libcurl4-openssl-dev zlib1g-dev
```
2. 转到项目目录编译服务端
```
cmake .
make -j16
```

## DDNet-HunterN的服务端下载&配置
* 你可以下载**正式发布包**于[Github Releases](https://github.com/Hu1night/DDNet-HunterN/releases)
* 或者下载**开发构建**于[Github Actions](https://github.com/Hu1night/DDNet-HunterN/actions/workflows/build.yaml?query=branch%3Amaster++)（[几乎汉化分支构建](https://github.com/Hu1night/DDNet-HunterN/actions/workflows/build.yaml?query=branch%3Ahuntern-zh_cn++)）(下载需Github账号)

**关于DDNet-HunterN的一些配置指令**
* ```sv_room_commands```于控制台启用时才能使玩家创建/加入房间
* ```room_setting 0```于控制台输入可查看0号房间可用的模式指令 房间指令使用实例：```room_setting 0 timelimit 3``` (设置0号房间每局限时3分钟)
* ```/setting```于(管理员)聊天栏输入可查看所在房间可用的模式指令 使用实例：```/setting map dm2``` (设置所在房间的子地图为dm2)

**关于DDNet-HunterN的一些配置文件**
* 服务端启动时执行/autoexec.cfg
* HunterN房间启动时执行/room_config/modes/huntern.rcfg
* DDNet-HunterN的Mega_std_collection地图启动时执行/data/maps/huntern_msc.map.cfg
* 服务器资产定位使用/storage.cfg

**关于DDNet-PvP的Mega map生成**
使用[DDNet-PvP附加工具](https://github.com/Hu1night/DDNet-HunterN/releases/download/0.3a1/DDNet-PvP.Extra.tools.zip)中的map_merge程序进行Mega map的生成
使用用法：```map_merge 输出地图名 输入地图1 输入地图2 输入地图3 ... 输入地图n```
使用实例：```map_merge huntern.map hunter1.map hunter2.map hunter3.map hunter4.map hunter6.map```