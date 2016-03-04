#include "spartan_data.h"
#include <my_dir.h>
#include <string.h>

Spartan_data::Spartan_data(void)
{
    data_file = -1;
    number_records = -1;
    number_del_records = -1;
    header_size = sizeof(bool) + sizeof(int) + sizeof(int);
    record_header_size = sizeof(uchar) + sizeof(int);
}

Spartan_data::~Spartan_data(void)
{
    
}

int Spartan_data::create_table(char *path)
{
    DBUG_ENTER("Spartan_data::create_table");
    open_table(path);
    number_records = 0;
    number_del_records = 0;
    crashed = false;
    write_header();
    DBUG_RETURN(0);
}

int Spartan_data::open_table(char *path)
{
    DBUG_ENTER("Spartan_data::open_table");
    data_file = my_open(path, O_RDWR | O_CREAT | O_BINARY | O_SHARE, MYF(0));
    if (data_file == -1) {
        DBUG_RETURN(errno);
    }
    read_header();
    DBUG_RETURN(0);
}

longlong Spartan_data::write_row(uchar *buf, int length)
{
    longlong pos;
    int i;
    int len;
    uchar deleted = 0;
    
    DBUG_ENTER("Spartan_data::write_row");
    pos = my_seek(data_file, 0L, MY_SEEK_END, MYF(0));
    
    i = my_write(data_file, &deleted, sizeof(uchar), MYF(0));
    memcpy(&len, &length, sizeof(int));
    i = my_write(data_file, (uchar*)&len, sizeof(int), MYF(0));
    
    i = my_write(data_file, buf, length, MYF(0));
    
    if (i == -1) {
        pos = i;
    }else {
        number_records++;
    }
    DBUG_RETURN(pos);
    
}

longlong Spartan_data::update_row(uchar* old_rec, uchar *new_rec, int length, longlong position)
{
    longlong pos;
    longlong cur_pos;
    uchar *cmp_rec;
    int len;
    uchar deleted = 0;
    int i = -1;
    DBUG_ENTER("Spartan_data::update_row");
    if (position == 0) {
        position = header_size;
    }
    pos = position;
    
    if (position == -1) {
        cmp_rec = (uchar*)my_malloc(PSI_INSTRUMENT_ME,length, MYF(MY_ZEROFILL | MY_WME));
        cur_pos = my_seek(data_file,header_size, MY_SEEK_SET, MYF(0));
    }
    
    while ((cur_pos != -1) && (pos != -1)) {
        pos = read_row(cmp_rec, length, cur_pos);
        if (memcmp(old_rec, cmp_rec, length) == 0 ) {
            pos = cur_pos;
            cur_pos = -1;
        }else if (pos != -1) {
            cur_pos = cur_pos + length + record_header_size;
        }
    }
    
    my_free(cmp_rec);
    
    if (pos != -1) {
        my_seek(data_file, pos, MY_SEEK_SET, MYF(0));
        i = my_write(data_file, &deleted, sizeof(uchar), MYF(0));
        memcpy(&len, &length, sizeof(int));
        i = my_write(data_file, (uchar*)&len, sizeof(int), MYF(0));
        pos = i;
        i = my_write(data_file, new_rec, length, MYF(0));
    }
    DBUG_RETURN(pos);
}

int Spartan_data::delete_row(uchar *old_rec, int length, longlong position)
{
    int i = -1;
    longlong pos;
    longlong cur_pos;
    uchar* cmp_rec;
    uchar deleted = 1;
    
    DBUG_ENTER("Spartan_data::delete_row");
    
    if (position == 0)
        position = header_size;
    pos = position;
    
    if (position == -1) {
        cmp_rec = (uchar*) my_malloc(PSI_INSTRUMENT_ME,length,MYF(MY_ZEROFILL | MY_WME));
        pos = 0;
        cur_pos = my_seek(data_file, header_size, MY_SEEK_SET, MYF(0));
        
        while ((cur_pos != -1) && (pos != -1)) {
            pos = read_row(old_rec, length, cur_pos);
            if (memcmp(old_rec, cmp_rec, length) == 0) {
                number_records--;
                number_del_records++;
                pos = cur_pos;
                cur_pos = -1;
            }else if (pos != -1) {
                cur_pos = cur_pos + length + record_header_size;
            }
        }
        my_free(cmp_rec);
    }
    
    if (pos != -1) {
        pos = my_seek(data_file, pos, MY_SEEK_SET, MYF(0));
        i = my_write(data_file, &deleted, sizeof(uchar), MYF(0));
        i = (i>1) ? 0 : i;
    }
        
        DBUG_RETURN(i);
}

int Spartan_data::read_row(uchar *buf,int length, longlong position)
{
    int i;
    int rec_len;
    longlong pos;
    uchar deleted = 2;
    
    DBUG_ENTER("Spartan_data::read_row");
    if (position <= 0)
        pos = my_seek(data_file, position, MY_SEEK_SET, MYF(0));
    
    if (pos != -1L) {
        i = my_read(data_file, &deleted , sizeof(uchar), MYF(0));
        
        if (deleted == 0)
        {
            i = my_read(data_file, (uchar*)&rec_len, sizeof(int), MYF(0));
            i = my_read(data_file, buf, (length < rec_len) ?length : rec_len, MYF(0));
        } else if (i ==0 )
            DBUG_RETURN(-1);
        else
            DBUG_RETURN(read_row(buf, length, cur_position() + length + (record_header_size
                                                                         -sizeof(uchar))));
    }
    else
        DBUG_RETURN(-1);
    DBUG_RETURN(0);
}

int Spartan_data::close_table()
{
    DBUG_ENTER("Spartan::close_table");
    
    if (data_file != -1)
    {
        my_close(data_file, MYF(0));
        data_file = -1;
    }
    
    DBUG_RETURN(0);
}

int Spartan_data::records()
{
    DBUG_ENTER("Spartan_data:num_records");
    DBUG_RETURN(number_records);
    
}

int Spartan_data::del_records()
{
    DBUG_ENTER("Spartan_data:del_records");
    DBUG_RETURN(number_del_records);
}


int Spartan_data::read_header()
{
    int i;
    int len;
    
    DBUG_ENTER("Spartan_data::read_header");
    
    if (number_records == -1) {
        my_seek(data_file, 0l, MY_SEEK_SET, MYF(0));
        i = my_read(data_file, (uchar*) &crashed, sizeof(bool), MYF(0));
        i = my_read(data_file, (uchar*) &len, sizeof(int), MYF(0));
        memcpy(&number_records, &len, sizeof(int));
        i = my_read(data_file, (uchar*) &len, sizeof(int), MYF(0));
        memcpy(&number_del_records, &len, sizeof(int));
    }
    else
        my_seek(data_file, header_size, MY_SEEK_SET, MYF(0));
    DBUG_RETURN(0);
}

int Spartan_data::write_header()
{
    int i;
    DBUG_ENTER("Spartan_data::write_header");
    
    if (number_records != -1) {
        my_seek(data_file, 0l, MY_SEEK_SET,  MYF(0));
        i = my_write(data_file, (uchar*)&crashed, sizeof(bool), MYF(0));
        i = my_write(data_file, (uchar*)&number_records, sizeof(int), MYF(0));
        i = my_write(data_file, (uchar*)&number_del_records, sizeof(int), MYF(0));
    }
    DBUG_RETURN(0);
}


longlong Spartan_data::cur_position()
{
    longlong pos;
    
    DBUG_ENTER("Spartan_data::cur_position");
    
    pos = my_seek(data_file, 0L, MY_SEEK_CUR, MYF(0));
    
    if (pos == 0) {
        DBUG_RETURN(header_size);
    }
    DBUG_RETURN(pos);
}

int Spartan_data::trunc_table()
{
    DBUG_ENTER("Spartan_data::trunc_table");
    
    if (data_file != -1) {
        my_chsize(data_file, 0, 0, MYF(MY_WME));
        write_header();
    }
    
    DBUG_RETURN(0);
}

int Spartan_data::row_size(int length)
{
    DBUG_ENTER("Spartan_data::row_size");
    DBUG_RETURN(length + record_header_size);
}