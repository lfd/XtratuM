
#define ESTR(s) STR(s)
#define STR(s) #s
#define eprintf(...) { printf("["__FILE__":"ESTR(__LINE__)"] " __VA_ARGS__); XM_halt_partition(XM_PARTITION_SELF); }

