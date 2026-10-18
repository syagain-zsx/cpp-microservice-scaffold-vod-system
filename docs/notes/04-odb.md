# ODB：C++ ORM 与数据层（本项目实现）

> 代码：`server/data/data.h`（实体定义）、`server/data/*.cc`（DAO）、`scaffold/JianLiuScaffold/odb.cc`（连接池）、
> `server/CMakeLists.txt`（odb 代码生成）、`deploy/sql/data.sql`（schema）

---

## 1. 原理速览

### 1.1 ODB 是什么

ODB 是一个 C++ ORM + 代码生成器：

- 你在头文件里用 `#pragma db` 描述「类 ↔ 表」的映射；
- 编译前运行 `odb` 命令，生成 `data-odb.hxx/.cxx/.ixx`（映射代码）和 `data.sql`（建表语句）；
- 运行时用 `odb::database` 执行 `persist / update / erase / query`。

### 1.2 常用映射

```cpp
#pragma db object table("tbl_video_meta")
class Video {
    #pragma db id auto
    size_t _id;
    #pragma db unique
    std::string _video_id;
    odb::nullable<std::string> _summary;   // 可空列
};

#pragma db view object(Session) query((?))
struct SessionPtr { std::shared_ptr<Session> session; };   // 视图
```

### 1.3 事务

`odb::transaction txn(db->begin());` 构造即开始，`commit()` 提交，析构未提交则回滚（RAII）。

---

## 2. 本项目怎么做的

### 2.1 实体

`server/data/data.h` 定义 13 张表：`Session`、`File`、`User`、`UserIdentifyRole`、`Follow`、
`Video`、`VideoLike`、`VideoCatgoryTag`、`Barrage`、`OperatorRole` 等；
每个 DAO（`data/xxx.cc`）构造时接收 `odb::transaction&`，复用同一个事务。

### 2.2 代码生成（构建期）

`server/CMakeLists.txt`：

```cmake
add_custom_command(
  OUTPUT ${GEN_DIR}/data-odb.hxx ${GEN_DIR}/data-odb.ixx ${GEN_DIR}/data-odb.cxx
  COMMAND odb -d mysql --std c++17 --generate-query --generate-schema
          --profile boost/date-time -I data -o ${GEN_DIR} data/data.h ...)
```

`data-odb.hxx` 被 `data/*.h` 包含（`#include <data-odb.hxx>`），只有 data.h 变化才会重新生成。

### 2.3 连接池

```cpp
// JianLiuScaffold/odb.cc
std::unique_ptr<odb::mysql::connection_factory> pool(
    new odb::mysql::connection_pool_factory(settings.connection_pool_size));
auto db = std::make_shared<odb::mysql::database>(user, passwd, db, host, port, ...);
```

`connection_pool_factory` 内部维护固定数量的连接，按需借还；事务结束归还。

### 2.4 查询示例

```cpp
// 单条
Video::ptr v(db.query_one<Video>(odb::query<Video>::video_id == vid));

// 列表
auto res = db.query<Video>(odb::query<Video>::video_status == 2);
for (auto it = res.begin(); it != res.end(); ++it) { ... }

// 删除
db.erase_query<Video>(odb::query<Video>::video_id == vid);
```

### 2.5 事务使用（服务层）

```cpp
odb::transaction mtx(_mysql->begin());
vp_data::UserData ud(mtx.database(), redis, sync);
ud.insert(user);                 // 多个 DAO 共享同一事务
mtx.commit();                    // 全部成功才提交
```

跨服务不能用一个事务，用「本地事务 + MQ 最终一致」保证。

### 2.6 与缓存/搜索的配合

DAO 里既写 MySQL 也写 Redis（`insertToCache`）、ES（`VideoSearchData::insert`），
写库成功后再更新缓存/索引；缓存失效通过延迟队列（见笔记 01）。

---

## 3. 面试问答

**Q：为什么选 ODB，不选手写 SQL 或其它 ORM？**
类型安全（字段编译期检查）、schema 自动生成、支持 MySQL/SQLite/PostgreSQL 切换；
代价是构建多一步代码生成、复杂 SQL 要用 native query。

**Q：ORM 的 N+1 问题？**
遍历对象再逐个查关联表会放大查询次数。ODB 支持 `view` 和 `query` 联表，把关联一次性查出来；
本项目用视图 `SessionPtr`、`DetailUser` 等减少查询次数。

**Q：事务隔离级别？**
MySQL 默认 REPEATABLE READ；本项目的事务主要保证「多表写入的原子性」，
缓存一致性靠延迟队列，不依赖数据库隔离级别。

**Q：连接池开多大？**
经验值：`CPU 核数 × 2 + 磁盘数`，且不超过 MySQL `max_connections` 的 1/实例数。
本项目默认 5，压测后再调。

**Q：schema 变更怎么管理？**
当前直接用 odb 生成的 `data.sql`（会 DROP/CREATE）；生产应改成迁移脚本（Flyway/Liquibase 思路）。

**Q：大字段/二进制怎么存？**
文件内容不进 MySQL，只存 `file_id` + 元信息，实际数据在 FastDFS（`tbl_file_meta.path`）。

---

## 4. 动手实验

1. 改 `data.h` 给 Video 加一个字段，重新 `cmake --build build`，看 `build/server/generated/data-odb.hxx` 与 `data.sql` 的变化；
2. 用 `deploy/sql/data.sql` 重建库，再用 `mysql` 客户端对比 `SHOW CREATE TABLE`；
3. 在 `video.cc` 里把 `query_one` 改成 `query` 并打印条数，验证唯一索引；
4. 故意插入重复 `video_id`，观察 ODB 抛出的异常与错误码转换。
