
typedef struct XmManagerDevice_t XmManagerDevice_t;
struct XmManagerDevice_t {
#define DEVICE_FLAG_COOKED (1<<0)
    int flags;

    int (*init) (void);
    int (*read) (char *str, int len);
    int (*write) (char *str, int len);
};

int XmManager(XmManagerDevice_t *dev);
