|文件|来源|细节|得分|
|---|---|---|---|
|`mm.c_copyfrombook`|书 & ppt实现`mm_realloc()`|46/100|
|`mm.c_first`|修改`mm.c_copyfrombook`|提高`m_realloc()`效率|52/100|
|`mm.c_second`|简单的分离储存|提高`mm_malloc()`效率 无合 并空间利用率降低|一个点超过可分配上限|
|`mm.c_third`|分离适配实现合并 提高空间利用率86/100


|效率(从高到低)|从高到低空间利用率(从高到低)|从高到低得分(从高到低)|
|---|---|---|
|`mm.c_second`|`mm.c_first`|`mm.c_third`|
|`mm.c_third`|`mm.c_third`|`mm.c_first`|
|`mm.c_first`|`mm.c_copyfrombook`|`mm.c_copyfrombook`|
|`mm.c_copyfrombook`|`mm.c_second`|`mm.c_second`|

如果在简单的分离适配中加上一个对边界的判断控制 使那一个点通过 或许会比`mm.c_third`得分高


