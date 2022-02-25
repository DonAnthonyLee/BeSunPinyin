# BeOS/HaikuOS 系统

终端下切换当前目录至 Makefile 文件所在处，而后运行 make 命令


# 用 Visual Studio 编译

+ 在 Makefile 文件所在文件夹下新建立子文件夹 libs
+ 将 sqlite 编译后的文件夹（需命名为 sqlite-build）移动到 libs 文件夹中
+ 将 etkxx-devel-binary 相关文件解压到 libs 文件夹中（该文件夹名为 etkxx-win32）
+ 将 sunpinyin 编译后的文件夹（需命名为 sunpinyin）移动到 libs 文件夹中
+ 运行 utils/prepare_to_build.bat 自动生成文件
+ 打开 build/Win32/BeSunPinyin.sln 项目即可进行编译

