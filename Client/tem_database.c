/********************************************************************************
 *      Copyright:  (C) 2024 LingYun<iot25@lingyun>
 *                  All rights reserved.
 *
 *       Filename:  tem_database.c
 *    Description:  This file is a sql file.
 *
 *        Version:  1.0.0(24/12/23)
 *         Author:  Qinyuan <qyts1989@163.com>
 *      ChangeLog:  1, Release initial version on "24/12/23 15:31:52"
 *                 
 ********************************************************************************/
#include "ds18b20.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h> // 编译时：gcc tem_database.c -o tem_database -lsqlite3 -I/home/iot25/sqlite3/include -L/home/iot25/sqlite3/lib

sqlite3         *db;   // 用于引用打开的数据库链接
sqlite3_stmt    *stmt; // sqlite3_stmt 指针的地址，用来存储编译的 sql 语句对象
char            *zErrMsg = 0; // 用于输出错误信息


/**
 * @brief 打开数据库，并创建表
 * @return 成功返回0，失败返回负值
*/
int open_db()
{
    /* 打开数据库 */
    if (sqlite3_open("offline_data.db", &db))
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    else
    {
        fprintf(stdout, "Open database successfully\n");
    }

    /* 创建表格 */
    char *create_table_sql = "CREATE TABLE IF NOT EXISTS offline_data(" \
        "timestamp      INTEGER     NOT NULL," \
        "serial_no        TEXT        NOT NULL," \
        "temperature    REAL        NOT NULL);";

    if (sqlite3_exec(db, create_table_sql, 0, 0, &zErrMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}


/**
 * @brief 向表中插入数据 
 * @param timestamp 时间戳
 * @param serial_no 序列号
 * @param temperature 树莓派温度 
 * @return 成功返回0，失败返回-1
 */ 
int insert_data(time_t timestamp, char *serial_no, float temperature)
{
    char *insert_sql = "INSERT INTO offline_data (timestamp, serial_no, temperature)" \
        "VALUES (?, ?, ?);"; // '?' 表示将在执行时提供实际的值

    /*
        sqlite3_prepare_v2() 这个函数通过将 SQL 语句编译成 sqlite3_stmt 对象，使得 SQL 语句可以在多次执行时复用，而不需要每次都重新解析和编译 SQL。
        如果 SQL 语句包含参数（如 ?），sqlite3_prepare_v2() 会为后续的参数绑定做准备，执行时可以为这些占位符提供具体的值。
    */
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0) != SQLITE_OK) // -1 表示自动计算 sql 语句的长度
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, timestamp); // 将一个 64 位整数值（sqlite3_int64 类型）绑定到 SQL 语句中的第 1 个占位符（?）
    sqlite3_bind_text(stmt, 2, serial_no, -1, SQLITE_STATIC); // 将 serial_no 绑定到第 2 个占位符(?)
    sqlite3_bind_double(stmt, 3, temperature); // 将 temperature 绑定到第 3 个占位符

    if (sqlite3_step(stmt) != SQLITE_DONE) // sqlite3_step() 用于执行 sqlite3_prepare_v2() 已经准备好的 sql 语句
    {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_finalize(stmt); // 专门用来清理和释放 预编译 sql 语句(sqlite3_stmt)相关资源
    return 0;
}


/**
 * @brief 获取断线时存储的发送失败的数据
 * @return 成功返回实际数据行数，失败返回-1
 */
int get_offline_data(data_t *data, int max_size)
{
    char *select_sql = "SELECT * FROM offline_data;";  // 从数据库查询所有的离线数据

    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    int count = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW)  // 如果查询到数据行
    {
        if (count >= max_size) {
            break;  // 防止数组越界
        }

        // 填充 data_t 结构体
        data[count].timestamp= sqlite3_column_int64(stmt, 0);
        strncpy(data[count].serial_no, (char *)sqlite3_column_text(stmt, 1), sizeof(data[count].serial_no) - 1);
        data[count].temperature = sqlite3_column_double(stmt, 2);

        count++;
    }

    sqlite3_finalize(stmt);
    return count;  // 返回实际查询到的离线数据个数
}


/**
 * @brief 删除数据库中的离线数据
 * @param timestamp 时间戳
 * @return 成功返回0，失败返回-1
 */
int delete_offline_data(time_t timestamp)
{
    char *delete_sql = "DELETE FROM offline_data WHERE timestamp = ?;";

    if (sqlite3_prepare_v2(db, delete_sql, -1, &stmt, 0) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int64(stmt, 1, timestamp);  // 将时间戳绑定到 SQL 语句

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "Failed to delete data: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}


void close_db()
{
    sqlite3_close(db);
}