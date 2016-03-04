#include "my_global.h"
#include "my_sys.h"

const long METADATA_SIZE = sizeof(int) + sizeof(bool);

struct SDE_INDEX
{
    uchar key[128];
    longlong pos;
    int length;
};

struct SDE_NDX_NODE
{
    SDE_INDEX key_ndx;
    SDE_NDX_NODE *next;
    SDE_NDX_NODE *prev;
};

class Spartan_index
{
public:
    Spartan_index(int keylen);
    Spartan_index();
    ~Spartan_index(void);
    int open_index(char *path);
    int create_index(char *path, int key_len);
    int insert_key(SDE_INDEX *ndx, bool allow_dupes);
    int delete_key(uchar* buf, longlong pos ,int key_len);
    int update_key(uchar* buf, longlong pos, int key_len);
    longlong get_index_pos(uchar* buf, int key_len);
    longlong get_first_pos();
    uchar *get_first_key();
    uchar *get_last_key();
    uchar *get_next_key();
    uchar *get_prev_key();
    int close_index();
    int load_index();
    int destroy_index();
    SDE_INDEX *seek_index(uchar *key, int key_len);
    SDE_NDX_NODE *seek_index_pos(uchar *key, int key_len);
    int save_index();
    int truc_index();
    
private:
    File index_file;
    int max_key_len;
    SDE_NDX_NODE *root;
    SDE_NDX_NODE *range_ptr;
    int block_size;
    bool crashed;
    int read_header();
    int write_header();
    longlong write_row(SDE_INDEX *ndx);
    SDE_INDEX *read_row(longlong position);
    longlong curfpos();
    
};