# BeSunPinyin

采用 libsunpinyin 2.0/3.0 的 Sun 拼音输入模块，适用于 BeOS/HaikuOS 类操作系统，亦适用 EIME 所移植各平台。

## 前注
鉴于 EIME-XIM 需求，而目前尚且无法对其他 XIM 服务端融入作简洁处理，由此打算写一个简洁的拼音输入模块。

多年以前曾协助其 BeOS 环境下相关修改，目前无照搬此前源码的计划，尽量从最简开始写起。

## 依赖项
**1. libsunpinyin**

+ Ubuntu Linux 系统可通过 apt-get install libsunpinyin-dev 安装
+ HaikuOS 系统可通过 HaikuDepot 安装 libsunpinyin（尚未确定）
+ Windows 系统可关注 [此仓库](https://github.com/DonAnthonyLee/sunpinyin/) （目前未发布，可自行编译）

**2. Lite BeAPI**

在 BeOS/HaikuOS 系统中编译无需此项。

其它系统可关注 [此仓库](https://github.com/DonAnthonyLee/etkxx-devel-binary) 。

*注：目前发布版本仅为 Windows 平台版本（头文件除了 ETKBuild.h 之外所有平台通用），其改进正处于 WIP 。*

**3. EIME 或 input_server**

在 BeOS/HaikuOS 系统中将在运行 make 进行编译时自动链接系统自身的 input_server。

其它系统中将依赖 EIME/EIME-XIM/EIME-WL/EIME-TSF 等等，具体可关注 [老李的杂货铺](https://donanthonylee.github.io) 。

*注：目前 EIME 各平台相应开发与 ETK++ 改进正在同步进行中，暂未发布开发库。*

## 编译后如何启用

+ BeOS/HaikuOS

将编译后的 SunPinyin.so 复制至 input_server 的 add-ons 目录 (建议采用 /boot/home/config/add-ons/input_server/methods )，然后重新启动 input_server（或重新启动系统）。

*注：HaikuOS 采用手动方式（即未制作 hpkg），建议复制至 /boot/home/config/non-packaged/add-ons/input_server/methods*

若遇启动异常，可于启动时按动空格键并禁用 user add-ons 即可。

+ 其它平台

将编译后的 SunPinyin.so 或 SunPinyin.dll 复制至 EIME 的 addons 目录下，然后重新启动相关实现。

*注：Windows 平台可将 libsunpinyin.dll 及 libsqlite3.dll 等依赖运行时文件复制到 eime.dll 所在目录。*


