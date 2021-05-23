# BeSunPinyin

采用 libsunpinyin 2.0/3.0 的 Sun 拼音输入模块，适用于 BeOS/HaikuOS 类操作系统，亦适用 EIME 所移植各平台。

## 前注
鉴于 EIME-XIM 需求，而目前尚且无法对其他 XIM 服务端融入作简洁处理，由此打算写一个简洁的拼音输入模块。

多年以前曾协助其 BeOS 环境下相关修改，目前无照搬此前源码的计划，尽量从最简开始写起。

## 依赖项
**1. libsunpinyin**

Ubuntu Linux 系统中可通过 apt-get install libsunpinyin-dev 安装；

HaikuOS 系统中可对其 SConstruct 作如下补丁后编译，可能同时需要手动安装 sqlite3 开发库；BeOS 系统目前几乎无法在大多数机器上运行，因此暂且略过，有需要者自请神通。

```
--- sunpinyin-3.0.0~git20160910/SConstruct	2021-05-14 13:20:16.026214400 +0000
+++ sunpinyin-3.0.0~working/SConstruct	2021-05-20 12:07:11.472383488 +0000
@@ -221,6 +221,9 @@
                       tools=['default', 'textfile'])
     env.Append(BUILDERS={'InstallAsSymlink': libln_builder})
     env['ENDIANNESS'] = "be" if sys.byteorder == "big" else "le"
+    if GetOS() == 'Haiku':
+    	env['CPPPATH']+=['/boot/develop/headers']
+    	env['CPPPATH']+=['/boot/system/develop/headers']
     return env
 
 
@@ -244,6 +247,10 @@
 if GetOption('datadir') is not None:
     env['DATADIR'] = GetOption('datadir')
 
+if GetOS() == 'Haiku':
+	env.Append(LIBPATH="/boot/develop/lib")
+	env['DATADIR'] = '/boot/home/config/lib'
+
 env['ENABLE_PLUGINS'] = GetOption('enable_plugins')
 
 opts.Save('configure.conf', env)
```

**2. Lite BeAPI**

	仅在非 BeOS/HaikuOS 系统中有此依赖。

**3. EIME 或 input_server**

	BeOS/HaikuOS 系统下将在运行 make 进行编译时自动进行链接；非 BeOS/HaikuOS 依赖 EIME/EIME-XIM/EIME-TSF 等。
