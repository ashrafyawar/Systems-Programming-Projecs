char* replace_strings(int case_insensetive_flag,int normal_string_flag,int found_rabbit_mul_flag,int found_dollar_flag,int found_star_flag,int found_brace_flag,char* sentence,char* str1,char* str2);

int parse_strings(int str_pair_counter,char *str1, char *str2, int case_insensetive_flag,int fd,const char *path);

int do_read_write(int fd,int str_pair_counter, char *temp_str,int bytesread, int byteswrite, int limit, char *buf, char *bp,int case_insensetive_flag,int normal_string_flag,int found_rabbit_mul_flag,int found_dollar_flag, int found_star_flag,int found_brace_flag,char* str1,char* str2, int temp_file_fd,char* result,char* template);
void print_error_1();

char *to_lower_string(char *str);

void fcntl_syscall_error_print();

void open_syscall_error_print();

void close_syscall_error_print();

void lseek_syscall_error_print();

void mkstemp_syscall_error_print();

void unlink_syscall_error_print();

void read_syscall_error_print();

void write_syscall_error_print();

int to_lower(int chr);