# KV_Engine
KV存储引擎
==
本项目就是基于跳表实现的轻量级键值型存储引擎，使用C++实现。

接口
--
* insertElement 插入元素
* deleteElement 删除元素
* searchElement 查找元素
* changeElement 修改元素
* displayList 输出所有键值对
* dumpFile 持久化保存
* loadFile 读取保存
* size 输出容量

项目运行方式
--
```cpp
make     
./bin/main
```
具体运行结果
--
<a href="https://imgtu.com/i/XwBHcd"><img src="https://s1.ax1x.com/2022/06/06/XwBHcd.png" alt="XwBHcd.png" border="0" /></a>
<a href="https://imgtu.com/i/XwBz4S"><img src="https://s1.ax1x.com/2022/06/06/XwBz4S.png" alt="XwBz4S.png" border="0" /></a>
