ics_lab2
========
这是一个将 lab2的炸弹 改为离线的小工具
你需要安装以下软件：
gcc objdump

将disconnect.c复制到bomb同一文件夹下

生成可执行文件disconnect
> gcc -o "disconnect" "disconnect.c" 

产生离线的bomb(得到文件bomb_dis)
> ./disconnect bomb bomb_dis

如果断开成功 会显示
>find 0 init_bomb

>find 0 send_msg

>find 1 send_msg

>find 202.120

使bomb_dis变成可执行文件
> chmod 777 bomb_dis

接下来尽情的引爆bomb_dis吧
