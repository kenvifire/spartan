#include "my_global.h"
#include "my_sys.h"

class Spartan_data
{
public:
    Spartan_data(void);
    ~Spartan_data(void);
    int create_table(char *path);
    int open_table(char *path);
    longlong write_row(uchar *buf, int length);
    longlong update_row(uchar *old_rec, uchar *new_rec,
                        int length, longlong position);
    int read_row(uchar* buf, int length, longlong position);
    int delete_row(uchar* old_rec, int length, longlong position);
    int close_table();
    longlong cur_position();
    int records();
    int del_records();
    int trunc_table();
    int row_size(int length);
private:
    File data_file;
    int header_size;
    int record_header_size;
    bool crashed;
    int number_records;
    int number_del_records;
    int read_header();
    int write_header();
};