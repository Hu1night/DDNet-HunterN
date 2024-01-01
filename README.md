DDNet-PvP HunterN猎人杀
===
模式规则：

1.每回合都会秘密随机选择猎人。猎人必须消灭所有平民。

2.猎人造成双倍伤害，有一把瞬杀锤和破片榴弹，而平民没有锤子，只能使用常规武器。

3.活着的玩家看不到死去玩家的信息。

4.如果猎人死亡，将通知其他猎人。

5.在游戏开始时，玩家只知道自己的身份。

Rules:

1.Each round will secretly randomly select Hunter(s). Hunter(s) must eliminate all the Civilians.

2.The Hunter deals double damage and has an instant-kill hammer and fragmentation grenades, while Civilians have no hammer and can only use regular weapons.

3.The living players cannot see messages from dead players.

4.If the Hunter dies, the other Hunters will be notified.

5.At the beginning of the game, players only know their own identity.

在Ubuntu上使用CMake构建
---
1.安装依赖库
```
    sudo apt install build-essential cmake python3 libsqlite3-dev
```
2.编译服务端
```
    cmake ..
    make -j16
```