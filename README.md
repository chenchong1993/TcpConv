# TcpConv
 数据流转发服务端

 程序说明：
 程序利用HHTP的GET请求从数据库获取基准站信息，并调用goproxy接口师兄数据流转发
 目前仅支持入口为client出口为server

 部署环境：ubuntu18.04
 部署方法：
 	1、整个文件夹程序放在/mnt/hgfs/fileShare/下；
 	2、到/mnt/hgfs/fileShare/TcpConv/bin/Debug目录下，给TcpConv赋予权限： chmod 751 TcpConv
 	3、程序入口main.cpp
