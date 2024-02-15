# DDNet-PvP HunterN猎人杀
**DDNet-HunterN**是一个基于**DDNet-PvP**的模组 提供**HunterN**(猎人杀)游戏模式

### HunterN的游戏规则：
1. 在该模式中 每回合都会**秘密随机**选择玩家成为**猎人**或**平民** 猎人的目标是**消灭所有平民** 开局时玩家只知道自己身份 
1. 猎人使用高伤武器、瞬杀追踪锤和破片榴弹 而平民没有锤子且只能使用常规武器
1. 死亡后 消息**仅旁观/死人可见** 如果猎人死亡还会通知其他猎人

## 在Ubuntu上使用CMake构建HunterN
1. 使用apt安装依赖库
```
sudo apt install build-essential cmake python3 libsqlite3-dev
```
2. 转到项目目录编译服务端
```
cmake .
make -j16
```
## HunterN的服务端下载&配置
* 你可以下载**正式发布包**于[Github Releases](https://github.com/Hu1night/DDNet-HunterN/releases)
* 或者下载**开发构建**于[Github Actions](https://github.com/Hu1night/DDNet-HunterN/actions/workflows/build.yaml)(下载需Github账号)

**HunterN的一些配置文件**
* 服务端启动时执行/autoexec.cfg
* HunterN房间启动时执行/room_config/modes/huntern.rcfg
* HunterN的Mega_std_collection地图启动时执行/data/maps/huntern_msc.map.cfg