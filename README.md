# test_malloc_use_so

#### 介绍
用于测试内存库性能的C++测试软件。

#### 软件架构
软件架构说明：使用C++编写的简单支持多线程的 测试内存库性能的C++测试软件，具有多种功能。


#### 安装教程
只有一个CPP文件，非常简单， 推荐的编译方法如下：
g++ -Wall -fexceptions -march=core2 -O2 -std=c++14 -c ./main.cpp -o ./main.o
g++ -o ./test_malloc_use_so ./main.o  -s  -lpthread

#### 使用说明

./test_malloc_use_so 
./test_malloc_use_so --help

Usage: test_use_so  test_count  loop_count  thread_count
Default:  test_count 1000000
Default:  loop_count 2
Default:  thread_count 1


#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
