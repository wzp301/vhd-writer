## 介绍

vhd-writer是一个用于修改虚拟磁盘([vhd](https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/dd323654(v=vs.85)))文件的命令行小工具。

## 功能

1.  目前只支持固定尺寸的vhd文件。

2.  支持LBA模式修改vhd文件，CHS模式待开发。

3.  支持显示vhd文件的基本信息。

## 编译

### Windows

```shell
PS C:\Users\vhd-writer> .\build_win.bat
```

### Linux

```shell
~$ sh buildall_linux.sh 
```

##  使用示例

1.显示帮助信息

```shell
.\vhd-writer.exe -h
Usage: vhd-writer <option(s)> vhdPath binPath
Options:
        -h,--help       DESTINATION     Show this help message
        -i,--info       DESTINATION     Show information about vhd file
        -vp,--vhd_path  DESTINATION     Specify the path of vhd file
        -bp,--bin_path  DESTINATION     Specify the path of bin file
        -m,--mode       DESTINATION     Specify the writing mode(LBA default)
        -s,--sector     DESTINATION     Specify the number of logic sector(Only in LBA mode)
```

2.打印vhd文件信息

```shell
.\vhd-writer.exe -vp ./test.vhd -i
        cookies : conectix
        creater app : vbox
        cylinder : 240
        head : 4
        sector : 17
        logic sector : 16319
        type : 2 (2->fixed, 3->dynamic)
```

3.读二进制文件以LBA模式写入vhd文件指定扇区

```shell
.\vhd-writer.exe -vp ./test.vhd -bp ./test.bin -s 0  #读test.bin中的512字节数据写到vhd的第0个逻辑扇区
info : Writing 512 bytes to sector 0 of vhd.
```