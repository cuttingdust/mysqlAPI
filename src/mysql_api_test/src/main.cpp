#include <LXMysql.h>
#include <iostream>
#include <format>
#include <chrono>
#include <thread>

int main(int argc, char *argv[])
{
    LXMysql my;
    /// 1 mysql ��ʼ��
    my.init();
    my.setConnectTimeout(3); /// ���ӳ�ʱ��
    my.setReconnect(true);   /// �Զ�����

    /// 2 ����mysql ���û�е���init �ڲ����Զ�����
    if (!my.connect("localhost", "root", "System123@", "laoxiaketang")
    {
        std::cerr << "my.Connect failed!" << std::endl;
        return -1;
    }
    std::cout << "my.Connect success��" << std::endl;

    /// 3 ִ��sql��䴴����
    std::string       sql        = "";
    std::string       table_name = "t_video2";
    const std::string col_id     = "id";
    const std::string col_name   = "name";
    const std::string col_data   = "data";
    const std::string col_size   = "size";

    // sql = std::format("CREATE TABLE IF NOT EXISTS `{0}` ("
    //                   "`{1}` INT AUTO_INCREMENT, "
    //                   "`{2}` VARCHAR(1024),"
    //                   "`{3}` BLOB,"
    //                   "`{4}` INT,PRIMARY KEY(`{1}`))",
    //                   table_name, col_id, col_name, col_data, col_size);
    //
    // std::cout << "create table: " << my.query(sql.c_str()) << std::endl;


    // /// �����Զ�����
    // for (;;)
    // {
    //     std::cout << my.query(sql.c_str()) << std::flush;
    // }

    // /// ����һ����¼
    // sql = std::format("INSERT INTO `{0}` (`{1}`,`{2}`) VALUES ('{3}',{4})", table_name, col_name, col_size, "test",
    //                   100);
    // /// ���Ա���
    // /// LOCK TABLES t_video2 WRITE;
    // /// UNLOCK TABLES;
    // std::cout << "insert one job:" << my.query(sql.c_str()) << std::endl;
    // std::cout << "=================����=========================" << std::endl;

    // /// ��������
    // my.startTransaction();
    // XDATA kv;
    // kv[col_name] = "transaction001";
    // kv[col_size] = "200";
    // my.insert(kv, table_name);
    //
    // kv[col_name] = "transaction002";
    // kv[col_size] = "300";
    // my.insert(kv, table_name);
    // my.rollback();
    //
    // kv[col_name] = "transaction003";
    // kv[col_size] = "300";
    // my.insert(kv, table_name);
    // kv[col_name] = "transaction004";
    // kv[col_size] = "300";
    // my.insert(kv, table_name);
    //
    // my.commit();
    // my.stopTransaction();
    //
    // kv[col_name] = "transaction005";
    // kv[col_size] = "200";
    // my.insert(kv, table_name);
    //
    // kv[col_name] = "transaction006";
    // kv[col_size] = "300";
    // my.insert(kv, table_name);
    // my.rollback();

    // /// ���������ݲ���
    // const std::string &fileName = "mysql.jpg";
    // LXData             file1;
    // file1.loadFile("mysql.jpg");
    // kv[col_name] = fileName.c_str();
    // kv[col_data] = file1;
    // kv[col_size] = &file1.size;
    // my.insertBin(kv, table_name);
    // file1.drop();

    // /// �޸�����
    // XDATA updateKV;
    // updateKV[col_name] = "updtename001";
    // updateKV[col_size] = "99999";
    // std::cout << "my.Update = " << my.update(updateKV, table_name, "`id`=1") << std::endl;
    //
    // /// �޸�����
    // XDATA              updateKV2;
    // const std::string &file2Name = "mysql2.jpg";
    // LXData             file2;
    // file2.loadFile(file2Name.c_str());
    // updateKV2[col_name] = file2Name.c_str();
    // updateKV2[col_data] = file2;
    // updateKV2[col_size] = &file2.size;
    // std::cout << "my.UpdateBin = " << my.updateBin(updateKV2, table_name, "`id`=79") << std::endl;
    // file2.drop();

    // /// ��ȡ�����
    // sql = std::format("SELECT * FROM `{0}`", table_name);
    // std::cout << "select * result:" << my.query(sql.c_str()) << std::endl;
    // my.storeResult(); /// ���������ȫ���洢
    //
    // int i = 0;
    // for (;;)
    // {
    //     auto row = my.fetchRow();
    //     if (row.empty())
    //         break;
    //
    //     for (auto data : row)
    //     {
    //         if (data.data)
    //         {
    //             if (data.type == LXData::LXD_TYPE_BLOB)
    //             {
    //                 // row[2].saveFile(row[1].data);
    //                 data.saveFile(std::format("mysql_{0}.jpg", i).c_str());
    //                 i++;
    //             }
    //             std::cout << data.data << " ";
    //         }
    //         else
    //         {
    //             std::cout << "NULL ";
    //         }
    //     }
    //     std::cout << std::endl;
    // }
    // my.freeResult();


    // std::cout << "select * result:" << my.query(sql.c_str()) << std::endl;
    //
    // my.useResult(); /// ��ʼ���ս����
    // my.freeResult();

    // {
    //     /// ��ʼ�����ַ��� ���⣬ ���룬��ȡ GBK utf-8
    //     std::cout << "��ʼ�����ַ���" << std::endl;
    //     std::string       coding         = "utf8";
    //     const std::string table_name_gbk = std::format("t_{}", coding);
    //
    //     /// ����utf8 ָ���ֶ�name�� utf �ַ���
    //     sql = std::format("CREATE TABLE IF NOT EXISTS `{0}` ("
    //                       "`{1}` INT AUTO_INCREMENT,"
    //                       "`{2}` VARCHAR(1024) CHARACTER SET "
    //                       "{3} COLLATE {3}_bin,PRIMARY KEY(`{1}`))",
    //                       table_name_gbk, col_id, col_name, coding);
    //
    //     my.query(sql.c_str());
    //     /// �������
    //     my.query(std::format("TRUNCATE {};", table_name_gbk).c_str());
    //     /// ָ����mysql������ַ���
    //     my.query(std::format("SET NAMES {};", coding).c_str());
    //     {
    //         XDATA data;
    //         data["name"] =
    //                 (char *)u8"UTF8���ģ� ����"; /// ������Ҫ�ͱ�ı����ʽ��һ�������ݶ������������ ��������϶ž�
    //         my.insert(data, table_name_gbk);
    //     }
    //
    //     XROWS rows = my.getResult(std::format("select * from {}", table_name_gbk).c_str());
    //     for (int i = 0; i < rows.size(); i++)
    //     {
    //         auto row = rows[i];
    //         for (int i = 0; i < row.size(); i++)
    //         {
    //             if (!row[i].data)
    //             {
    //                 std::cout << "[NULL],";
    //                 continue;
    //             }
    //             switch (row[i].type)
    //             {
    //                 case LXData::LXD_TYPE_BLOB:
    //                     std::cout << "[BLOB]";
    //                     break;
    //                 case LXData::LXD_TYPE_LONG:
    //                 case LXData::LXD_TYPE_STRING:
    //                 default:
    //                     std::cout << row[i].utf8ToGbk();
    //                     break;
    //             }
    //
    //             std::cout << "|";
    //         }
    //
    //         std::cout << std::endl;
    //     }
    // }


    {
        /// ��Ʊģ��(����) t_tickets(id int,sold int)
        table_name                 = "t_tickets";
        const std::string col_sold = "sold";
        sql                        = std::format("CREATE TABLE IF NOT EXISTS `{0}` ("
                                                                        "`{1}` INT AUTO_INCREMENT,"
                                                                        "`{2}` INT,"
                                                                        "PRIMARY KEY(`{1}`))",
                                                 table_name, col_id, col_sold);
        my.query(sql.c_str());


        XDATA data;
        data["sold"] = "0";
        my.insert(data, "t_tickets"); //id=1
        my.startTransaction();

        bool        bIsUpdate  = true;
        std::string str_update = "";
        bIsUpdate ? str_update = "for update" : str_update = ""; /// ����
        //����
        sql = std::format("select * from {0} where {1} {2} {3};", table_name, std::format("{}=0", col_sold),
                          std::format("order by {}", col_id), str_update);
        // std::cout << sql << std::endl;
        XROWS       rows = my.getResult(sql.c_str());
        std::string id   = rows[0][0].data;
        std::cout << "Buy ticket id is " << id << std::endl;

        /// ģ���ͻ
        std::this_thread::sleep_for(std::chrono::seconds(10));
        data["sold"] = "1";
        my.update(data, table_name, std::format("{}={}", col_id, id));

        std::cout << "Buy ticket id  " << id << " success!" << std::endl;
        sql = std::format("select * from {} where {} {};", table_name, std::format("{}={}", col_sold, 0), str_update);
        // my.getResult(sql.c_str());
        my.commit();
        my.stopTransaction();
    }


    // my.freeResult();
    my.close();
    std::cin.get();
    return 0;
}
