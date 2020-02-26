# 日志计数器

## 用途

- 统计日志文件，每一个周期内，符合指定规则的日志数量（比如，用于QPS的统计）

## 形式

- 增量统计

## 实现原理

### 数据结构

- 双向队列存储周期内的所有日志数据
- 哈希表存储周期内每条日志的标识与其出现次数的关系（如果，每条日志天然不同，此处可以优化成一个整型。）

### 实现描述

开启两个线程，一个用于增量读取日志数据，将日志数据按一定规则处理，追加到双向队列与哈希表中；另一个用于将周期外的超时数据从双向队列的另一端移除，并维护哈希表。

## 使用

### 配置

见文件注释

### 执行
```
g++ -std=c++11 -pthread main.cpp counter
./counter
```