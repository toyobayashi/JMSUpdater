用远古 VC 写的日服冒险岛更新工具

# 完整客户端下载

[官网](https://maplestory.nexon.co.jp/gameguide/gamestart/download/)

直连下载主安装程序和分包到同一目录下，LE 启动 MSSetup.exe 安装：

```
https://webdown2.nexon.co.jp/maple/MSSetup.exe
https://webdown2.nexon.co.jp/maple/MSSetup.z00
https://webdown2.nexon.co.jp/maple/MSSetup.z01
https://webdown2.nexon.co.jp/maple/MSSetup.z02
...
https://webdown2.nexon.co.jp/maple/MSSetup.z16
...
```

# 手动更新程序

Patch 文件直连下载地址：

例：从 v393 更新到 v394

```
https://webdown2.nexon.co.jp/maple/patch/patchdir/00394/00393to00394.patch
```

下载好丢进 `data` 目录，输入对应的版本号点击开始按钮即可制作手动更新程序。

更新程序格式：

```cpp
struct pre_patcher {
  // [Binary] Patcher.exe 可执行文件
  uint8_t*   base_patcher_exe;
  // [Binary] 自己手动下载好的 Patch 文件
  uint8_t*   patch_file;
  // [String] 通知字符串，不以 \0 结束，当更新页面访问失败时显示
  char*      notice;
  // [UINT32] Patch 文件大小
  uint32_t   patch_file_size;
  // [UINT32] 通知字符串长度
  uint32_t   notice_length;
  // [固定 4 个字节 F3 FB F7 F2]
  uint8_t    magic[4];
};
```
