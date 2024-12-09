#include <LXMysql.h>
#include <iostream>
#include <format>
#include <chrono>

int main(int argc, char *argv[])
{
    LXMysql my;
    /// 1 mysql 初始化
    my.init();
    my.setConnectTimeout(3); /// 连接超时秒
    my.setReconnect(true);   /// 自动重连

    /// 2 连接mysql 如果没有调用init 内部会自动调用
    if (!my.connect("192.168.1.89", "root", "Handabao123@", "laoxiaketang"))
    {
        std::cerr << "my.Connect failed!" << std::endl;
        return -1;
    }
    std::cout << "my.Connect success！" << std::endl;

    /// 3 执行sql语句创建表
    std::string       sql        = "";
    const std::string table_name = "t_video2";
    const std::string col_id     = "id";
    const std::string col_name   = "name";
    const std::string col_data   = "data";
    const std::string col_size   = "size";

    sql = std::format("CREATE TABLE IF NOT EXISTS `{0}` ("
                      "`{1}` INT AUTO_INCREMENT, "
                      "`{2}` VARCHAR(1024),"
                      "`{3}` BLOB,"
                      "`{4}` INT,PRIMARY KEY(`{1}`))",
                      table_name, col_id, col_name, col_data, col_size);

    std::cout << "create table: " << my.query(sql.c_str()) << std::endl;


    // /// 测试自动重连
    // for (;;)
    // {
    //     std::cout << my.query(sql.c_str()) << std::flush;
    // }

    // /// 插入一条记录
    // sql = std::format("INSERT INTO `{0}` (`{1}`,`{2}`) VALUES ('{3}',{4})", table_name, col_name, col_size, "test",
    //                   100);
    // std::cout << "insert one job:" << my.query(sql.c_str()) << std::endl;

    my.startTransaction();
    XDATA kv;
    kv[col_name] = "transaction001";
    kv[col_size] = "200";
    my.insert(kv, table_name);

    kv[col_name] = "transaction002";
    kv[col_size] = "300";
    my.insert(kv, table_name);
    my.rollback();

    kv[col_name] = "transaction003";
    kv[col_size] = "300";
    my.insert(kv, table_name);
    kv[col_name] = "transaction004";
    kv[col_size] = "300";
    my.insert(kv, table_name);

    my.commit();
    my.stopTransaction();

    kv[col_name] = "transaction005";
    kv[col_size] = "200";
    my.insert(kv, table_name);

    kv[col_name] = "transaction006";
    kv[col_size] = "300";
    my.insert(kv, table_name);
    my.rollback();

    // /// 二进制数据插入
    // const std::string &fileName = "mysql.jpg";
    // LXData             file1;
    // file1.loadFile("mysql.jpg");
    // kv[col_name] = fileName.c_str();
    // kv[col_data] = file1;
    // kv[col_size] = &file1.size;
    // my.insertBin(kv, table_name);
    // file1.drop();

    // /// 修改数据
    // XDATA updateKV;
    // updateKV[col_name] = "updtename001";
    // updateKV[col_size] = "99999";
    // std::cout << "my.Update = " << my.update(updateKV, table_name, "`id`=1") << std::endl;
    //
    // /// 修改数据
    // XDATA              updateKV2;
    // const std::string &file2Name = "mysql2.jpg";
    // LXData             file2;
    // file2.loadFile(file2Name.c_str());
    // updateKV2[col_name] = file2Name.c_str();
    // updateKV2[col_data] = file2;
    // updateKV2[col_size] = &file2.size;
    // std::cout << "my.UpdateBin = " << my.updateBin(updateKV2, table_name, "`id`=79") << std::endl;
    // file2.drop();

    /// 获取结果集
    sql = std::format("SELECT * FROM `{0}`", table_name);
    std::cout << "select * result:" << my.query(sql.c_str()) << std::endl;
    my.storeResult(); /// 结果集本地全部存储

    int i = 0;
    for (;;)
    {
        auto row = my.fetchRow();
        if (row.empty())
            break;

        for (auto data : row)
        {
            if (data.data)
            {
                if (data.type == LXData::LXD_TYPE_BLOB)
                {
                    // row[2].saveFile(row[1].data);
                    data.saveFile(std::format("mysql_{0}.jpg", i).c_str());
                    i++;
                }
                std::cout << data.data << " ";
            }
            else
            {
                std::cout << "NULL ";
            }
        }
        std::cout << std::endl;
    }
    my.freeResult();


    std::cout << "select * result:" << my.query(sql.c_str()) << std::endl;

    my.useResult(); /// 开始接收结果集
    my.freeResult();


    my.close();
    std::cin.get();
    return 0;
}
